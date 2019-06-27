/*#define NOERROR */
#define MOREASM
/*
 * SY6522 disk driver
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
#include "sys/ttold.h"
#include "setjmp.h"
#include "sys/profile.h"
#include "sys/pport.h"
#include "sys/d_profile.h"
#include "sys/cops.h"
#include "sys/swapsz.h"

#ifdef notdef		/* these are in d_profile.h */
#define logical(x)	(minor(x) & 7)		/* eight logicals per phys */
#define interleave(x)	(minor(x) & 0x8)	/* interleave bit for swaping */
#define physical(x)	((minor(x) & 0xF0) >> 4)/* 10 physical devs */
#endif

char pro_secmap[NSEC] = {
/*11*/	0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1
};

	/* Logical Units */
	/* The first 100 blocks are reserved for the boot program and
	   are inaccessible via unix.
	 */
#define	MAXBOOT	100
struct prlmap {
	daddr_t	pm_beg;		/* Base address in blocks */
	daddr_t	pm_len;		/* Number of blocks in logical device */
} prlmap[] = {
/* a */	{PRNSWAP+101,	16955},	/* root filesystem on 10 Meg. disk */
/* b */	{101,	PRNSWAP},	/* swap area (2400 blocks) */
/* c */	{PRNSWAP+101,	7227},	/* root filesystem on 5 Meg. disk */
/* d */	{9728,	9728},		/* 2nd filesystem on 10 Meg. disk */
/* e */	{0,	0},		/* unused */
/* f */	{0,	7168},		/* old root filesystem (old a) */
/* g */	{7168,	2496},		/* old swap (old b) */
/* h */	{101,	19355},		/* f.s. using entire 10 Meg. disk */
};
/* THESE MAY REPLACE f AND g ABOVE */
/* f 	{4101,	15355},		/* alternate root f.s. on 10 Meg. disk */
/* g 	{101,	4000},		/* alternate swap */

struct iostat prostat[NPPDEVS];
struct iobuf protab = tabinit(PR0,prostat);	/* active buffer header */
struct buf rprobuf;				/* Raw input-output buffer */
struct proheader rphbuf;
struct proheader phbuf;

#ifndef NOERROR
#define ERROR(x) printf("HARD DISK ERROR "); printf x
char *pro_lefmt = "ASSERTION (%s) FAILED IN PROC %s ";
#define ASSERT(e, p, m, x) if (!(e)) {\
printf(pro_lefmt,"e","p");\
printf m;\
printf ("\n");\
x;};
#else
#define ERROR(x)
#define ASSERT(e, p, m, x)
#endif

/* Contrast change ok flag.  Maintained by the disk driver.  When 0 the
 * parallel port is not in use and may be switched to allow console contrast
 * changes.  If the contrast is waiting for the disk then 'l2_rcflag' is one.
 * When convenient and this is set the disk will call l2ramp.
 */
int ppinuse;
extern char l2_rcflag;

/*
 * proopen - check for existence of controller
 */
proopen(dev)
{
	int i;
	register struct device_d *devp;
	int prointr();
	extern char slot[];

	i = physical(dev);
	if (i) {		/* for expansion slot check slot number and type */
		if (!PPOK(i) || (slot[PPSLOT(i)] != PR0)) {
			u.u_error = ENXIO;
			return 1;
		}
	}
	devp = pro_da[i];
	u.u_error = 0;
	if (iocheck(&devp->d_ifr)) {
		{ asm(" nop "); }
		if (prodata[i].pd_da != devp) {	/* not already setup */
			if (setppint((prodata[i].pd_da = devp),prointr))
				goto fail;
			if (proinit(&prodata[i])) {
				freeppin(devp);
				goto fail;
			}
		}
	} else {
	fail:
		u.u_error = ENXIO;
		prodata[i].pd_da = (struct device_d *)0;
		return 1;
	}
	return 0;
}

/*
 * prostrategy - set up to start the transfer
 */
prostrategy(bp)
register struct buf *bp;
{
	int pun = physical(bp->b_dev);
	register struct prodata *p = &prodata[pun];
	register struct buf *up;

	if (!p->pd_da) {
		printf("Attempt to read/write unopened profile device\n");
		printf("bp=%x dev=%x (Unit %d)\n",bp,bp->b_dev,pun);
		p->pd_err = "device not open";
		goto haderr;
	}
	if (bp->b_blkno >= 0 &&
	   (bp->b_blkno < prlmap[logical(bp->b_dev)].pm_len)) {
		bp->av_forw = (struct buf *)NULL; /* last of all bufs and */
		bp->ul_forw = (struct buf *)NULL; /* last of units bufs */

		SPL6();		/* must be highest of all ints for this code*/
		if (protab.b_actf == NULL)
			protab.b_actf = bp;	/* empty - put on front */
		else
			protab.b_actl->av_forw = bp; /* else put at end */
		protab.b_actl = bp;

		if (p->pd_actv == 0) {		/* controller inactive */
			p->pd_actv = bp;	/* start of unit blk list */

	/* Since we might fail before ever getting an interrupt
	 * we must be prepared to do the buffer cleanup here also.
	 */
			while (prostart(p)) {	/* start up the transfer */
				bp->b_resid = bp->b_bcount;
				bp->b_flags |= B_ERROR;
				prorelse(p,bp);
			}
		} else {			/* link onto unit list */
			for (up=p->pd_actv; up->ul_forw; up=up->ul_forw)
				;
			up->ul_forw = bp;
		}
				
		SPL0();
		return;
	}
	p->pd_err = "invalid blkno";
haderr:
	bp->b_resid = bp->b_bcount;
	bp->b_flags |= B_ERROR;
	iodone(bp);
	return;
}

/*
 * Release finished buffer and unlink from list.  Two lists are maintained.
 * The av_forw pointers are used to link all the buffers in use by the driver
 * onto the protab iobuf header.  The av_back pointers (dubbed ul_forw) are
 * used to link together the buffers into unit lists (headed by the prodata
 * entry for that unit).
 */
prorelse (p, bp)
register struct prodata *p;
register struct buf *bp;
{
	register struct buf *up;

	if (protab.b_actf == bp) {	/* first buffer */
		if ((protab.b_actf = bp->av_forw) == (struct buf *)0)
			protab.b_actl == (struct buf *)0;
	} else {			/* middle or last buffer */
		for (up=protab.b_actf; up->av_forw != bp; up=up->av_forw)
			if (!up->av_forw) panic("prorelse: buf list error");
		up->av_forw = bp->av_forw;
		if (!up->av_forw && (protab.b_actl == bp))
			protab.b_actl = up;
	}

	p->pd_actv = bp->ul_forw;	/* next buf for this unit */
	iodone(bp);
}

/*
 * proinit - initialize drive first time or after severe error
 */
proinit(p)
	struct prodata *p;
{
	register char zero = 0;
	register struct device_d *devp = p->pd_da;
	int pl;

	pl = spl6();
	devp->d_acr = zero;
	devp->d_pcr = 0x6B;	/* set controller CA2 pulse mode strobe */
	devp->d_ddra = zero;	/* set port A bits to input **/
	if (devp == PPADDR)
		devp->d_ddrb &= 0x5C;	/* set BSY and OCD to input */
	else
		devp->d_ddrb &= 0xFC;	/* two or four port cards */
	devp->d_ddrb |= 0x1C;	/* set port B bits 2,3,4 to out */
	devp->d_irb &= ~DEN;	/* set enable = true */
	devp->d_irb |= CMD|DRW;	/* set command = false  set direction = in */
	devp->d_t2cl = zero;
	devp->d_t2ch = zero;
	devp->d_ier = FIRQ|FCA1;
	zero = devp->d_irb;
	if (zero & OCD) {
		p->pd_state = SERR;
		splx(pl);
		return 1;
	}
	p->pd_state = SCMD;
	splx(pl);
	return 0;
}

/*
 * proread - process read from disk
 */
proread(dev)
dev_t dev;
{
	physio(prostrategy, &rprobuf, dev, B_READ);
}

/*
 * prowrite - process write to disk
 */
prowrite(dev)
dev_t dev;
{
	physio(prostrategy, &rprobuf, dev, B_WRITE);
}

/*
 * prostart - initiate the next logical io operation
 */
prostart(p)
register struct prodata *p;
{
	register struct buf *bp = p->pd_actv;

	if (!bp)
		return 0;
	ASSERT(bp&&p,prostart,("bp=x%x p=x%x",bp,p),while(1))

	p->pd_limit = prlmap[logical(bp->b_dev)].pm_beg;/* logical offset */
	p->pd_blkno = bp->b_blkno + p->pd_limit;	/* starting blk # */
#ifndef UNISOFT
	if (p->pd_blkno <= MAXBOOT)		/* don't allow access to boot */
		return(-1);
#endif UNISOFT
	p->pd_limit += prlmap[logical(bp->b_dev)].pm_len; /* max blk # + 1*/
	p->pd_bcount = bp->b_bcount;
	p->pd_addr = bp->b_un.b_addr;
	return procmd(bp->b_flags, bp->b_dev, p->pd_blkno, (unsigned)p->pd_bcount);
}

/*
 * Procmd - initiate next physical io operation
 */
procmd(func, dev, bn, ct)
	register daddr_t bn;
	unsigned ct;
{
	register int pun = physical(dev);
	register struct prodata *p = &prodata[pun];
	register struct cmd *pc;

	ASSERT(ct!=0,Procmd,("ct=%d",ct),while(1))

#ifndef UNISOFT
	if (bn <= MAXBOOT)			/* check again to be sure */
		return(-1);
#endif UNISOFT
	if (p->pd_state == SERR) {
		if (proinit(p))			/* initialize drive */
			return -1;
	}
		/* controller should not be busy now */

		/* Build Command (ok to send extra bytes on write cmd) */
	pc = &p->pd_cmdb;
	pc->p_cmd = (func & B_READ) ? PROREAD : PROWRITE;
	pc->p_high = bn >> 16;
	pc->p_mid = bn >> 8;
	if (interleave(dev) == 0)
		pc->p_low = bn;
	else
		pc->p_low = (bn & 0xF0) | pro_secmap[bn&0xF];
	pc->p_retry = 10;
	pc->p_thold = 3;
	p->pd_state = SCMD;
	p->pd_nxtst = SCMD;

	if (prochk(p,SCMD)) {		/* sync controller to cmd state */
		if (!prochk(p,SCMD))	/* if it failed it should work now */
			return 0;
		p->pd_err = "cant force disk into CMD state";
		p->pd_state = SERR;
		return -1;
	}
	return 0;
}

prointr(pun)
int pun;
{	
	register struct device_d *devp;	/* a5 */
	register char *cp;		/* a4 */
	register char csum;		/* d7 */	/*NOTUSED*/
	register char zero = 0;		/* d6 */
	register struct buf *bp;
	register struct prodata *p = &prodata[pun];
	register short i;
	struct proheader *ph;
	devp = p->pd_da;

	(void) spl6();	/* added April 4/84 to prevent panic in prorelse */
		/* changed from spl5 August 30/84 to fix multi-user bug on 2/10 */
	/* ASSERT((stats&BSY)==BSY,prointr,("disk %d busy: state=%d, irb=x%x",pun,p->pd_state,stats),return(devp->d_ifr = devp->d_ifr)) */

#ifdef lint
	csum = 0;
	i = csum;
#endif
	if ((bp = p->pd_actv) == 0) {	/* spurious interrupt */
	 	/* printf("Spurious INT on profile dev %d [at %x]\n",pun,devp); */
		devp->d_ddrb |= 0x80;	/* setup for a reset */
		devp->d_irb |= 0x80;
		devp->d_ddrb &= 0x7F;
		devp->d_ifr = devp->d_ifr;		/* reset interrupt trap */
		/* proinit(p); */
		return;
	}

	/* ASSERT(bp!=0,prointr,("dsk=%d ier=x%x ifr=x%x state=%d",pun,devp->d_ier&255,devp->d_ifr&255,p->pd_state),return devp->d_ifr=0x7F) */
	ASSERT((p->pd_state!=SERR&&p->pd_state!=SSTOP),prointr,("b_flags=0x%x",bp->b_flags),while(1))
	ASSERT(physical(bp->b_dev)==pun,prointr,("dev=x%x unit=%d",bp->b_dev,pun),while(1))

	devp->d_ifr = devp->d_ifr;		/* reset interrupt trap */
		/*
		 * Note that IO operations fail when OCD.
		 * This may result in a `panic'.  Allowing it to
		 * block and be restarted has it problems also.
		 */
	if (devp->d_irb & OCD) {		/* cable disconnected ? */
		p->pd_err = "Open Cable Disconnect";
		goto haderr;
	}

	if (p->pd_state == SCMD) {		/* Send command */
		devp->d_irb &= ~DRW;	/* set dir=out */
		devp->d_ddra = 0xFF;	/* set port A bits to output */
					/* Now send command */
		cp = (char *)(&p->pd_cmdb);
		if (*cp == PROREAD)
			p->pd_nxtst = SRDBLK;
		else
			p->pd_nxtst = SWRTD;

		devp->d_ira = *cp++; devp->d_ira = *cp++; devp->d_ira = *cp++;
		devp->d_ira = *cp++; devp->d_ira = *cp++; devp->d_ira = *cp;

		devp->d_irb |= DRW;
		devp->d_ddra = zero;
		if (prochk(p,p->pd_nxtst)) {
			p->pd_err = "failed to issue cmd to disk";
			goto haderr;
		}
		return;		/* will send data or get status next */
	}

	if (p->pd_state == SRDBLK || p->pd_state == SFINI) {/* Read status word */
		devp->d_irb |= DRW;
		devp->d_ddra = zero;
		p->pd_sbuf = 0;
		cp = (char *)(&p->pd_sbuf);
		*cp++ = devp->d_ira;
		*cp++ = devp->d_ira;
		*cp++ = devp->d_ira;
		*cp = devp->d_ira;
		p->pd_sbuf &= ~STATMSK;	/* mask off redund stat bits */
		if (p->pd_sbuf) {
			ERROR(("dev %d: state=%d status=x%x\n",p-prodata,p->pd_state,p->pd_sbuf));
			p->pd_err = "bad status";
			goto haderr;
		}
		if (p->pd_state == SRDBLK) { /* Read successful so pickup data*/
			ASSERT(p->pd_bcount>0,prointr,(""),goto haderr)

			i = sizeof(rphbuf) - 1;		/* sizeof header */
			cp = (char *)(&rphbuf);
			do *cp++ = devp->d_ira;
			while (--i != -1);

			cp = p->pd_addr;
			i = min(SECSIZE, (unsigned)p->pd_bcount);
#ifdef MOREASM
			if ((i & 3) == 0) {
				i = (i >> 2) - 1;
				do {
					asm (" movb a5@(9),a4@+ ");
					asm (" movb a5@(9),a4@+ ");
					asm (" movb a5@(9),a4@+ ");
					asm (" movb a5@(9),a4@+ ");
					/* asm (" movb a5@(9),d6 "); */
					/* asm (" movb d6,a4@+ "); */
					/* asm (" eorb d6,d7 "); */
					/* asm (" movb a5@(9),d6 "); */
					/* asm (" movb d6,a4@+ ");  */
					/* asm (" eorb d6,d7 "); */
					/* asm (" movb a5@(9),d6 "); */
					/* asm (" movb d6,a4@+ ");  */
					/* asm (" eorb d6,d7 "); */
					/* asm (" movb a5@(9),d6 "); */
					/* asm (" movb d6,a4@+ ");  */
					/* asm (" eorb d6,d7 "); */
				} while (--i != -1);	/* optimizes to DBRA */
			} else {
#endif
				i--;
				do {
					asm (" movb a5@(9),a4@+ ");
					/* asm (" movb a5@(9),d6 "); */
					/* asm (" movb d6,a4@+ "); */
					/* asm (" eorb d6,d7 "); */
				} while (--i != -1);
#ifdef MOREASM
			}
#endif
		}

	/* Determine if IO operation is completed or spans another block.  */
		p->pd_bcount -= min(SECSIZE, (unsigned)p->pd_bcount);
		if (p->pd_bcount <= 0 || ++p->pd_blkno >= p->pd_limit) {
			bp->b_resid = p->pd_bcount;
			prorelse(p,bp);
			if (p-prodata == 0) ppinuse = 0;	/* contrast */

			if (prostart(p)) {	/* startup next buf in chain */
				p->pd_err = "prostart failed on next blk";
				goto haderr;
			}
			return;		/* next op started or no next op */
		}
		p->pd_addr += SECSIZE;	/* setup for next block of io trans */
		if(procmd(bp->b_flags,bp->b_dev,p->pd_blkno,(unsigned)p->pd_bcount)){
			p->pd_err = "Procmd failed to continue operation";
			goto haderr;
		}
		return;			/* end of i/o or beg of next blk */
	}

	if (p->pd_state == SWRTD) {	/* send data */
		p->pd_nxtst = SFINI;
		ASSERT(p->pd_bcount>0,prointr,(""),while(1))
		devp->d_irb &= ~DRW;	/* set dir=out */
		devp->d_ddra = 0xFF;	/* set port A bits to output */

		ph = &phbuf;
		ph->ph_fileid = p->pd_blkno ?0 :0xAAAA;
		i = sizeof(phbuf) - 1;
		cp = (char *)(ph);
		do devp->d_ira = *cp++;
		while (--i != -1);
		cp = (char *)p->pd_addr;	/* place to get data from */

		i = min(SECSIZE, (unsigned)p->pd_bcount);
#ifdef MOREASM
		if ((i & 3) == 0) {
			i = (i >> 2) - 1;
			do {
				asm (" movb a4@+,a5@(9) ");
				asm (" movb a4@+,a5@(9) ");
				asm (" movb a4@+,a5@(9) ");
				asm (" movb a4@+,a5@(9) ");
				/* asm (" movb a4@+,d6 ");	*/
				/* asm (" eorb d6,d7 ");	*/
				/* asm (" movb d6,a5@(9) ");	*/
				/* asm (" movb a4@+,d6 ");	*/
				/* asm (" eorb d6,d7 ");	*/
				/* asm (" movb d6,a5@(9) ");	*/
				/* asm (" movb a4@+,d6 ");	*/
				/* asm (" eorb d6,d7 ");	*/
				/* asm (" movb d6,a5@(9) ");	*/
				/* asm (" movb a4@+,d6 ");	*/
				/* asm (" eorb d6,d7 ");	*/
				/* asm (" movb d6,a5@(9) ");	*/
			} while (--i != -1);		/* optimizes to DBRA */
		} else {
#endif
			i--;
			do {
				asm (" movb a4@+,a5@(9) ");
				/* asm (" movb a4@+,d6 "); */
				/* asm (" eorb d6,d7 "); */
				/* asm (" movb d6,a5@(9) "); */
			} while (--i != -1);
#ifdef MOREASM
		}
#endif
		if (prochk(p,SPERFORM)) {
			p->pd_err = "didn't get to perfrom state";
			goto haderr;
		}
		return;			/* will pick up status next intr */
	}
	p->pd_err = "invalid state";
haderr:
	do {
		bp->b_resid = p->pd_bcount;
		bp->b_flags |= B_ERROR;
		ERROR(("dev %d: %s\n",p-prodata,p->pd_err));
		prorelse(p,bp);
		p->pd_state = SERR;
	} while (prostart(p));
}

/*
 * Get in sync with disk.
 * (subroutine FINDD2 & CHKRSP from 'profrom.text' document)
 * Expects enable==true and Direction==in at start.
 * If disk response is 'state' then returns 0 (success)
 * otherwise fails (returns -1 if timeout and cur state if bad state).
 */
prochk(p, state)
	register struct prodata *p;
{
	register struct device_d *devp = p->pd_da;
	register zero = 0;
	register i;
	int resp;
/* while((devp->d_irb&BSY)==0); */
	ASSERT((devp->d_irb&BSY)==BSY,prochk,("state=%d [waiting]",state),goto err)
	/* while ((devp->d_irb&BSY)==0); */

	if (state == SCMD && (p-prodata == 0)) {/* PP inuse */
		if (l2_rcflag)		/* ramp in progress */
			l2ramp(2);	/* so help it along */
		ppinuse = 1;
	}
	devp->d_irb |= DRW;		/* set input mode */
	devp->d_ddra = zero;		/* set port A bits to input */
	devp->d_irb &= ~CMD;		/* set cmd and enable bufs */

	i = RSPTIME;			/* about 1ms */
	while (devp->d_irb&BSY && i--);	/* wait sig that resp byte is ready*/

	resp = PIDL;			/* reply to use if resp byte wrong */
	if (i > 0) {			/* didn't timeout */
		i = devp->d_ira;	/* get response from disk */
		if (i == state)		/* got correct state */
			resp = PGO;	/* reply to use if resp byte correct*/
		devp->d_irb &= ~(DRW|CMD);	/* set dir=out cmd=true */
		devp->d_ddra = 0xFF;		/* set port A bits to output */
		devp->d_ira = resp;		/* send reply (GO or RESET) */
		devp->d_ier = FIRQ|FCA1;	/* enable interrupts */
		devp->d_irb |= CMD;		/* sig disk to read resp */
		p->pd_state = p->pd_nxtst;		/* setup next state */
		return (i == state) ?0 :i;
	}
err:
	if (p-prodata == 0) ppinuse = 0;	/* reset ppinuse flag */
	p->pd_state = SERR;
	p->pd_err = "EXCESSIVE DISK DELAY -- (is the drive plugged in??)";
	ERROR(("dev %d: %s\n",p-prodata,p->pd_err));
	devp->d_ddra = zero;		/* set port A bits to input */
	devp->d_irb |= CMD|DRW;		/* set dir=in, disable buffers */
	devp->d_ier = ~FIRQ;		/* disable all interrupts */
	return (-1);
}

/* ARGSUSED */
proioctl(dev, cmd, addr, flag)
dev_t dev;
int cmd;
caddr_t addr;
int flag;	/*NOTUSED*/
{
	struct prodata *p = &prodata[physical(dev)];

	switch (cmd) {

	case TIOCGETP:
		if (copyout((caddr_t)&p->pd_flags, addr, sizeof(p->pd_flags)))
			u.u_error = EFAULT;
		break;
	case TIOCSETP:
		if (copyin(addr, (caddr_t)&p->pd_flags, sizeof(p->pd_flags)))
			u.u_error = EFAULT;
		break;
	default:
		u.u_error = ENOTTY;
	}
}
proprint(dev, str)
char *str;
{
	printf("%s on pro drive %d, slice %d\n", str, (dev>>4)&0xF, dev&7);
}
