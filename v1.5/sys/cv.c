/*#define HOWFAR*/
#define INTSON		/* defined for an interrupting disk */

/*
 * Corvus Disk System
 */
#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/utsname.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/altblk.h"
#include "sys/diskformat.h"
#include "setjmp.h"
#include "sys/cops.h"
#include "sys/pport.h"
#include "sys/d_profile.h"
#include "sys/cv.h"
#include "sys/swapsz.h"

#ifdef notdef		/* defined in d_profile.h */
#define logical(x)	(minor(x) & 7)		/* eight logicals per phys */
#define interleave(x)	(minor(x) & 0x8)	/* interleave bit for swapping */
#define physical(x)	((minor(x) & 0xF0) >> 4)/* 10 physical devs */
#endif
#define cv_addr(d)	(prodata[d].pd_da)
#define cvwait(a)	while(((a)->d_irb & ST_BUSY) == 0)

/*
 * the total space on the corvus h series is:
 * 306 cylinders are there but corvus reserves 2
 * 304 cylinders * 20 sectors per track * 6 heads =
 * 	36480
 *
 * The first 100 blocks are reserved for the boot program and
 * are inaccessible via unix.
 */
#define	MAXBOOT	100
struct cv_sizes {
	daddr_t	sz_offset;
	daddr_t	sz_size;
}	cv_sizes[] = {
	CVNSWAP+101,	32420,	/* a: root filesystem */
	101,	CVNSWAP,	/* b: swap area (3959 blocks) */
	0,	0,		/* c: unused */
	0,	0,		/* d: unused */
	0,	0,		/* e: unused */
	0,	0,		/* f: unused */
	0,	0,		/* g: unused */
	101,	1000000		/* h: filesystem using entire disk */
};

struct iostat cvstat[NPPDEVS];
struct iobuf cvtab = tabinit(CV2,cvstat);	/* active buffer header */
struct buf cvrbuf;

/*
 * cvopen - check for existence of controller
 */
cvopen(dev)
register dev;
{
	register punit;
	register struct device_d *devp;
	int cvint();
	extern char slot[];

	punit = physical(dev);
	if (punit) {		/* for expansion slot check slot number and type */
		if (!PPOK(punit) || (slot[PPSLOT(punit)] != PR0)) {
			u.u_error = ENXIO;
			return 1;
		}
	}
	devp = pro_da[punit];
	u.u_error = 0;

	if (iocheck(&devp->d_ifr)) {	/* board there ? */
		if (cv_addr(punit) != devp) {	/* not already setup */
			if (setppint((cv_addr(punit) = devp),cvint))
				goto fail;
			if (cvinit(&prodata[punit])) {
				freeppin(devp);
				goto fail;
			}
		}
	} else {
	fail:
		u.u_error = ENXIO;
		cv_addr(punit) = (struct device_d *)0;
		return 1;
	}
	return 0;
}

/*
 * cvinit - initialize drive first time
 */
cvinit(p)
register struct prodata *p;
{
	register struct device_d *devp = p->pd_da;
	register char irb;
	register char zero = 0;
	int pl;

	pl = spl6();
	if (devp == PPADDR) {
		devp->d_ddrb &= 0x5C;	/* port B bits: 0,1,5,7 to in, 2,3,4,6 to out */
		devp->d_pcr = 0x6B;	/* set controller CA2 pulse mode strobe */
		devp->d_ddra = zero;	/* set port A bits to input **/
		devp->d_irb |= CMD|DRW;	/* set command = false  set direction = in */
		devp->d_ddrb |= 0x7C;
		devp->d_irb &= ~DEN;	/* set enable = true */
	} else {
		devp->d_pcr = 0x6B;	/* set controller CA2 pulse mode strobe */
		devp->d_ddra = zero;	/* set port A bits to input **/
		devp->d_irb = CMD|DRW;	/* set command = false  set direction = in */
		devp->d_ddrb = 0x7C;
	}
#ifdef	INTSON
	devp->d_ier = FIRQ|FCA1;
	irb = devp->d_irb;
#ifdef lint
	pl = irb;
#endif lint
	p->pd_state = SCMD;
#endif	INTSON
	splx(pl);
	return 0;
}

cvstrategy(bp)
register struct buf *bp;
{
	register punit, lunit, bn;

	punit = physical(bp->b_dev);
	lunit = logical(bp->b_dev);
	bn = bp->b_blkno + cv_sizes[lunit].sz_offset;
	if (bp->b_blkno < 0 || bn <= MAXBOOT) {
#ifdef HOWFAR
		prdev("cvstrategy: illegal blkno", bp->b_dev);
		printf("blkno=%d bcount=%d\n", bp->b_blkno, bp->b_bcount);
#endif HOWFAR
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	cvstat[punit].io_ops++;
#ifdef INTSON
	bp->b_resid = bn;	/* resid for disksort */
	SPL6();
	disksort(&cvtab, bp);
#else INTSON
	bp->av_forw = (struct buf *)NULL; /* last of all bufs */
	if (cvtab.b_actf == NULL)
		cvtab.b_actf = bp;	/* empty - put on front */
	else
		cvtab.b_actl->av_forw = bp; /* else put at end */
	cvtab.b_actl = bp;
#endif INTSON
	if (cvtab.b_active == 0)
		cvstart();
#ifdef INTSON
	SPL0();
#else INTSON
	while (cvtab.b_active)
		cvint();
#endif INTSON
	return;
}

cvstart()
{
	register struct buf *bp;
	register lunit, offset, bn;
	register struct device_d *addr;

loop:
	if ((bp = cvtab.b_actf) == (struct buf *)NULL)
		return;
	if (cvtab.b_active == 0) {
		bp->b_resid = bp->b_bcount;
		cvtab.b_active = 1;
	}
	lunit = logical(bp->b_dev);
	blkacty |= (1<<CV2);
	offset = bp->b_bcount - bp->b_resid;
	bn = bp->b_blkno + btod(offset);	/* logical block number */
	if (bp->b_resid < BSIZE || bn >= cv_sizes[lunit].sz_size) {
	next:
#ifdef HOWFAR
		if (bp->b_resid != 0)
			printf("Unix cvstart: blkno=%d resid=%d bn=%d\n",
				bp->b_blkno, bp->b_resid, bn);
#endif HOWFAR
		blkacty &= ~(1<<CV2);
		cvtab.b_active = 0;
		if (cvtab.b_errcnt) {
			logberr(&cvtab, 0); /* errlog non-fatal errors */
			cvtab.b_errcnt = 0;
		}
		addr = cv_addr(physical(bp->b_dev));
		addr->d_ifr = addr->d_ifr;		/* reset intr */
		cvtab.b_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	if (cvrw(minor(bp->b_dev), bn + cv_sizes[lunit].sz_offset, BSIZE,
			bp->b_flags&B_READ, bp->b_un.b_addr + offset) < 0) {
		bp->b_flags |= B_ERROR;
		logberr(&cvtab, 1);	/* log fatal error */
		goto next;
	}
	return;
}

caddr_t cv_buf;
int cv_count;

cvrw(dev, blkno, n, flag, buff)
register dev;
register daddr_t blkno;
register n, flag;
register caddr_t buff;
{
	register punit;
	register struct device_d *addr;

	punit=physical(dev);
	cv_buf = buff;
	cv_count = n;
	if (cvop(punit, 4, flag==B_READ ? N_R512 : N_W512, (blkno>>16 & 0xf)+1,
		blkno & 0xff, blkno>>8 & 0xff) < 0)
		goto bad;

	if (flag == B_WRITE && cvw(punit, buff, n) < 0)
		goto bad;
	addr = cv_addr(punit);
	addr->d_ddra = 0x00;	/* data direction port A bits to input */
	addr->d_irb |= 0x08;	/* bidirectional driver to input */
	return 0;
bad:
#ifdef HOWFAR
	printf("cvrw: %s error unit=%d blkno=%d n=%d buf=0x%x\n",
		flag==B_READ?"read":"write", punit, blkno, n, buff);
#endif HOWFAR
	return -1;
}

/* VARARGS3 */
cvop(unit, na, a)
{
#ifdef INTSON
	register s;
#endif INTSON
	register int *ap;
	register struct device_d *addr = cv_addr(unit);

	addr->d_ddra = 0xff;	/* port A to output */
	addr->d_irb &= ~0x08;	/* bidirectional driver to output */

	ap = &a;
	if (na-- > 0) {
#ifdef INTSON
		s = spl7();
#endif INTSON
		cvwait(addr);
		addr->d_ira = *ap++;
#ifdef INTSON
		splx(s);
#endif INTSON
	}
	for (; na > 0; na--, ap++) {
		cvwait(addr);
		addr->d_ira = *ap;
	}
	return 0;
}

cvint(punit)
int punit;
{
	register struct device_d *addr;
	register struct buf *bp;
	register char status;

	(void) spl6();
	addr = cv_addr(punit);
	if (cvtab.b_active == 0) {
#ifdef HOWFAR
		printf("cvint: b_active == 0\n");
#endif HOWFAR
		if (addr == PPADDR)
			addr->d_ifr = addr->d_ifr;		/* reset intr */
		return;
	}
	if ((bp = cvtab.b_actf) == (struct buf *)NULL) {
#ifdef HOWFAR
		printf("cvint: b_actf == NULL\n");
#endif HOWFAR
		if (addr == PPADDR)
			addr->d_ifr = addr->d_ifr;		/* reset intr */
		return;
	}
	cvwait(addr);
	if (addr == PPADDR)
		addr->d_ifr = addr->d_ifr;		/* reset intr */
	if (status = addr->d_ira) {
	err:
		printf("%s error: /dev/c%d%c blkno=%d\n",
			bp->b_flags&B_READ?"read":"write", punit,
			'a'+logical(bp->b_dev), bp->b_blkno);
		cvlog(status);
		cv_count = 0;
		if (++cvtab.b_errcnt > NRETRY) {
			bp->b_flags |= B_ERROR;
			logberr(&cvtab, 1);	/* log fatal error */
			blkacty &= ~(1<<CV2);
			cvtab.b_errcnt = 0;
			cvtab.b_actf = bp->av_forw;
			cvtab.b_active = 0;
			iodone(bp);
		}
	} else if (bp->b_flags&B_READ)
		if (cvr(punit, (char *)cv_buf, cv_count) < 0)
			goto err;
	/*
	 * because a single buffer can take several io operations, 
	 * we leave it to cvstart() to figure out when it's done
	 */
	bp->b_resid -= cv_count;
	cvstart();
}

cvlog(status)
register status;
{
	register struct buf *bp;
	register struct device_d *addr;
	register bn, punit, lunit;
	struct deverreg cvreg[2];

	bp = cvtab.b_actf;
	punit = physical(bp->b_dev);
	lunit = logical(bp->b_dev);
	cvtab.io_stp = &cvstat[lunit];
	addr = cv_addr(punit);
	cvreg[0].draddr = (long)&(addr->d_ira);
	cvreg[0].drvalue = status;
	cvreg[0].drname = "cv status";
	cvreg[0].drbits = "Corvus disk status code";
	cvreg[1].draddr = (long)0;
	cvreg[1].drvalue = cv_count;
	cvreg[1].drname = "count";
	cvreg[1].drbits = "byte count of transfer";
	bn = bp->b_blkno + btod(bp->b_bcount - bp->b_resid) + cv_sizes[lunit].sz_offset;
	fmtberr(&cvtab,
		(unsigned)punit,
		(unsigned)0,		/* cylinder */
		(unsigned)0,		/* track */
		(unsigned)bn,		/* sector */
		(long)(sizeof(cvreg)/sizeof(cvreg[0])), /* regcnt */
		&cvreg[0],&cvreg[1]);
}

cvw(unit, buff, n)
register char *buff;
register n;
{
	register char *ira;
	register struct device_d *addr = cv_addr(unit);

	ira = &(addr->d_ira);
	cvwait(addr);
	for ( ; n > 0; n--) {
		if ((addr->d_irb & ST_HTOC) == 0)
			break;
		*ira = *buff++;
	}
	for (;;) {
		cvwait(addr);
		if ((addr->d_irb & ST_HTOC) == 0)
			break;
		*ira = 0;
	}
	cv_count -= n;
	if (n > 0) {
#ifdef HOWFAR
		printf("cvw: %d bytes short\n", n);
#endif HOWFAR
		cvlog(0);
		return -1;
	}
	return 0;
}

cvr(unit, buff, n)
register char *buff;
register n;
{
	register char *ira;
	register struct device_d *addr = cv_addr(unit);

	cvwait(addr);
	ira = &(addr->d_ira);
	for ( ; n > 0; n--) {
		if (addr->d_irb & ST_HTOC)
			break;
		*buff++ = *ira;
	}
	cv_count -= n;
	if (n > 0) {
#ifdef HOWFAR
		printf("cvr: %d bytes short\n", n);
#endif HOWFAR
		return -1;
	}
	for (;;) {
		cvwait(addr);
		if (addr->d_irb & ST_HTOC)
			break;
		n = *ira;
	}
	return 0;
}

/********
#ifdef	INTSON
#else	INTSON
/* wait for controller to host direction or timeout */ /*
cvctoh(a)
register struct device_d *a;
{
	register i;

	for (i = 20; i-- > 0;);
	i = 100000;
	do
		while (--i > 0 && ((a->d_irb&ST_BUSY) == 0));
	while (i > 0 && (a->d_irb & ST_HTOC));
	if (i <= 0) {
		printf("cvctoh: timeout\n");
		return -1;
	}
	return 0;
}
#endif	INTSON
********/

cvread(dev)
dev_t dev;
{
	physio(cvstrategy, &cvrbuf, dev, B_READ);
}

cvwrite(dev)
dev_t dev;
{
	physio(cvstrategy, &cvrbuf, dev, B_WRITE);
}

cvprint(dev, str)
char *str;
{
	printf("%s on cv drive %d, slice %d\n", str, (dev>>4)&0xF, dev&7);
}
