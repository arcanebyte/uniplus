/* #define HOWFAR */
/* #define WBBLK	/* allows writing block 0 */
/* #define UNISOFT	/* allows access to restricted blocks on boot disk */
#define KLUDGE	/* kludge for format to work */
/*
 * (C) Copyright 1983 UniSoft Systems of Berkeley CA
 *
 * Sony driver
 * and eject driver
 *
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
#include "sys/uioctl.h"
#include "sys/al_ioctl.h"
#include "sys/diskformat.h"
#include "setjmp.h"
#include "sys/pport.h"
#include "sys/sony.h"
#include "sys/cops.h"

#define NRETRY	5

#define physical(d) ((minor(d)>>4)&0xF)	/* physical unit number 0-15 */
#define logical(d) (minor(d)&0x7)	/* logical unit number, 0-7 */
#define splsn	spl1

#define GETBUF(bp)	splsn(); \
	while (bp->b_flags & B_BUSY) { \
		bp->b_flags |= B_WANTED; \
		(void)sleep((caddr_t)bp, PRIBIO+1); \
	} \
	bp->b_flags |= B_BUSY; \
	spl0()

#define FREEBUF(bp)	splsn(); \
	if (bp->b_flags & B_WANTED) \
		wakeup((caddr_t)bp); \
	bp->b_flags = 0; \
	spl0()

struct	buf	snhbuf;		/* header buffer (for reading block 0) */
struct	buf	sncbuf;		/* command buffer */
struct	buf	snrbuf;
struct iostat snstat[NSN];
struct iobuf sntab = tabinit(SN1,snstat);	/* active buffer header */

#ifdef WBBLK
char sn_bblk[512];
#endif WBBLK
char sn_bcount[NSN*2];			/* block opens */
char sn_ccount[NSN*2];			/* character opens */
#define	count(d)	(d<<1)		/* first char is how many opens */
#define	boot(d)		((d<<1)+1)	/* 2nd char (sn_bcount only) is whether a boot disk */
char snbf;		/* index into sn_fns array */
#ifdef KLUDGE
char noerror;		/* read before format is acceptable error */
#endif KLUDGE

struct sn_sizes {
	daddr_t sn_offset, sn_size;
} sn_sizes[] = {
	0,		800,		/* a = filesystem */
	201,		599,		/* b = filesystem on a boot Sony */
	0,		201,		/* c = 1-100 for lisa (serialization),
					       101-200 for boot program */
	0,		0,		/* d = unused */
	0,		0,		/* e = unused */
	0,		0,		/* f = unused */
	0,		0,		/* g = unused */
	0,		800		/* h = entire disk */
};

/*
 *	called from oem7init at (at least) level 1, so won't be interrupted
 *	by parallel port 0 or sony
 */
sninit()
{
	char c = 0x00;			/* to avoid clear byte intstructions */

	if (SNIOB->type) {
		printf("Microdiskette with %d head%s\n", SNIOB->type,
			(SNIOB->type==1)?"":"s");
	} else {
		printf("Unix sninit: not a sony drive\n");
		return;
	}
	SNIOB->drive=0x80;		/* always lower drive */
	SNIOB->side = c;		/* always first side */
	SNIOB->mask= 0xff;		/* clear ints and enable */
	if (snwaitrdy()) {
		printf("Unix sninit: command to clear status failed\n");
		return;
	}
	if (SNIOB->drv_connect != 0xff) {
		printf("Unix sninit: no drive connected\n");
		return;
	}
	SNIOB->gobyte=SN_CLRST;
	SNIOB->mask =0x80;		/* enable interrupts */
	if (snwaitrdy()) {
		printf("Unix sninit: command to clear status failed\n");
	}
	SNIOB->gobyte=SN_STMASK;
}

/*
 *	check block 0 to see whether this is a boot disk
 */
sncz(dev)
register dev_t dev;
{
	static char hp[BSIZE];
	register struct buf *bp = &snhbuf;
	register rc = 0;

	GETBUF(bp);
	sn_bcount[boot(physical(dev))] = 0;	/* for now set to non-boot */
	bp->b_bcount = BSIZE;
	bp->b_dev = dev | 7;			/* set logical unit 7 */
	bp->b_blkno = 0;
	bp->b_un.b_addr = hp;
	bp->b_flags |= B_READ;
	snbf = 1;
	snstrategy(bp);
	iowait(bp);
	if (bp->b_flags & B_ERROR) {
		rc++;
		snbf = 0;
	}
	FREEBUF(bp);
	return(rc);
}

/*
 *	don't really check whether it's a boot disk
 */
/* ARGSUSED */
snconf(dev)
dev_t dev;
{
	snbf = 1;
	return(0);
}

struct sn_fns {
        int	(*fnc)();
} sn_fns[] = {
	sncz,		/* the real routine to check block 0 */
	snconf		/* fake routine to confuse */
};

snopen(dev)
register dev_t dev;
{
	dev = physical(dev);
	if (dev >= NSN) {
		u.u_error = ENXIO;
		return(-1);
	}
	return(0);
}

snbopen(dev)
register dev_t dev;
{
	if (snopen(dev) == 0)
		sn_bcount[count(physical(dev))]++;
}

sncopen(dev)
register dev_t dev;
{
	if (snopen(dev) == 0)
		sn_ccount[count(physical(dev))]++;
}

snclose(dev)
register dev_t dev;
{
	if (physical(dev) >= NSN) {
		u.u_error = ENXIO;
		return(-1);
	}
	snbf = 0;
	return(0);
}

snbclose(dev)
register dev_t dev;
{
	if (snclose(dev) == 0)
		sn_bcount[count(physical(dev))] = 0;
}

sncclose(dev)
register dev_t dev;
{
	if (snclose(dev) == 0)
		sn_ccount[count(physical(dev))] = 0;
}

snstrategy(bp)
register struct buf *bp;
{
	register punit;

	punit = physical(bp->b_dev);
	if (bp == &sncbuf) {				/* if command */
		snstat[punit].io_misc++; /* errlog: */
		splsn();
		if (sntab.b_actf == (struct buf *)NULL) /* set up links */
			sntab.b_actf = bp;
		else
			sntab.b_actl->av_forw = bp;
		sntab.b_actl = bp;
		bp->av_forw = (struct buf *)NULL;
	} else {
		if ((*sn_fns[snbf].fnc)(bp->b_dev)) {
			bp->b_flags |= B_ERROR;
			iodone(bp);		/* resets flags */
			return;
		}
		snstat[punit].io_ops++; /* errlog: */
			/* resid for disksort */
		bp->b_resid = bp->b_blkno + sn_sizes[logical(bp->b_dev)].sn_offset;
		splsn();
		disksort(&sntab, bp);
	}
	if (sntab.b_active == 0)
		snstart();
	spl0();
}

snstart()
{
	register struct buf *bp;
	register punit, lunit;
	register daddr_t bn;
	caddr_t addr;

loop:
	if ((bp = sntab.b_actf) == (struct buf *)NULL)
		return;
	if (sntab.b_active == 0) {
		sntab.b_active = 1;
		if (bp != &sncbuf)
			bp->b_resid = bp->b_bcount;
	}
	punit = physical(bp->b_dev);
	lunit = logical(bp->b_dev);
	blkacty |= (1<<SN1);
	if (bp == &sncbuf) {
#ifdef WBBLK
		if (bp->b_resid == UIOCWBBLK)
			snw0(punit);
		else
#endif WBBLK
			sncmd(bp->b_resid);	/* b_resid holds the command */
		return;
	}
	bn = bp->b_blkno + ((bp->b_bcount - bp->b_resid) >> 9);
	if (bp->b_resid < 512 ||
	    bn >= sn_sizes[lunit].sn_size ||
	    ((bn += sn_sizes[lunit].sn_offset) >= SN_MAXBN) ||
	    (bn <= 200 && sn_bcount[boot(punit)])) {
#ifdef HOWFAR
		if (bp->b_resid != 0)
			printf("Unix snstart: blkno=%d resid=%d bn=%d\n",
				 bp->b_blkno, bp->b_resid, bn);
#endif HOWFAR
		blkacty &= ~(1<<SN1);
		if (sntab.b_errcnt)
			logberr(&sntab, 0); /* errlog non-fatal errors */
		sntab.b_active = 0;
		sntab.b_errcnt = 0;
		sntab.b_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	addr = bp->b_un.b_addr + bp->b_bcount - bp->b_resid;
	snrw(punit, bn, bp->b_flags&B_READ, addr);
}

snread(dev)
dev_t dev;
{
	physio(snstrategy, &snrbuf, dev, B_READ);
}

snwrite(dev)
dev_t dev;
{
	physio(snstrategy, &snrbuf, dev, B_WRITE);
}

struct sn_blockmap {
	int maxblock, sectors;
} sn_blockmap[] = {
		192, 12,
		176, 11,
		160, 10,
		144, 9,
		128, 8 
};

/* ARGSUSED */
snrw(unit, bn, rw, addr)	/* always called at priority sn (see splsn above) */
int unit, rw;
register daddr_t bn;
register caddr_t addr;
{
	register char *pm = (char *)SN_DATABUF;
	register struct sn_blockmap *p = sn_blockmap;
	register int i;
	int track, sector;
	char c = 0x00;

	track = 0;
	while( bn >= p->maxblock ) {
		bn -= p->maxblock;
		track += 16;
		p++;
	}
	sector = bn % p->sectors;
	track += bn / p->sectors;
	if (snwaitrdy())
		return;
	SNIOB->side=c;
	SNIOB->drive=0x80;
	SNIOB->track = track; 
	SNIOB->sector= sector;
	if (rw) {
		SNIOB->cmd=SN_READ;
	} else {
		SNIOB->cmd=SN_WRITE;
		i = 511;
		do {
			*pm++;			/* cmos ram, every other byte */
			*pm++ = *addr++;
		} while (--i != -1);
	}
	if (snwaitrdy())
		return;
	SNIOB->gobyte = SN_CMD;
}

/* ARGSUSED */
snioctl(dev, cmd, adr, flag)
dev_t dev;
caddr_t adr;
{
	int unit;
	register struct buf *bp;

	if ((unit = physical(dev)) >= NSN) {
		u.u_error = EFAULT;
		return;
	}
	bp = &sncbuf;
	switch (cmd) {
		case AL_EJECT:
			if ((sn_bcount[count(unit)] > 0)
				|| (sn_ccount[count(unit)] > 1)) {
				u.u_error = EINVAL;
				return;
			}
			break;
		case UIOCFORMAT:
			if (!suser()) {
				u.u_error = EPERM;
				return;
			}
#ifdef KLUDGE
			noerror=1;
			(void)(sncz(0));
			noerror=0;
#endif KLUDGE
			break;
#ifdef WBBLK
		case UIOCWBBLK:
			if (!suser()) {
				u.u_error = EPERM;
				return;
			}
			if (copyin(adr, (caddr_t)sn_bblk, sizeof(sn_bblk))) {
				u.u_error = EFAULT;
				return;
			}
			break;
#endif WBBLK
		default:
			u.u_error = ENOTTY;
			return;
	}
	GETBUF(bp);
	bp->b_dev = dev;
	bp->b_resid = cmd;		/* stash the command in resid */
	u.u_error = 0;			/* any error on sncz is OK */
	snstrategy(bp);
	iowait(bp);
	if (bp->b_flags & B_ERROR)
		u.u_error = EIO;
	FREEBUF(bp);
}

sncmd(cmd)	/* always called at priority sn (see splsn above) */
unsigned int cmd;
{
	if(snwaitrdy())
		return;
	switch (cmd) {
		case UIOCFORMAT:
			SNIOB->confirm=0xff;
			SNIOB->cmd=SN_FORMAT;
			break;
		case AL_EJECT:
			SNIOB->cmd=SN_EJECT;
			break;
		default:
			return;
	}
	SNIOB->gobyte = SN_CMD;
}

#ifdef WBBLK
snw0(punit)
int punit;
{
	register char *pm;
	struct sn_hdr sn_hdr;
	register char *addr;
	register i;
	char c = 0x00;

	if(snwaitrdy())
		return;
	SNIOB->side=c;
	SNIOB->drive=0x80;
	SNIOB->track = 0; 
	SNIOB->sector= 0;
	SNIOB->cmd=SN_WRITE;
	sn_hdr.version = 0;
	sn_hdr.volume = 0;
	sn_hdr.fileid = FILEID;
	sn_hdr.relpg = 0;
	sn_hdr.dum1 = 0;
	sn_hdr.dum2 = 0;
	i = sizeof(sn_hdr) - 1;
	pm = (char *)SN_HDRBUF;		/* write special 12-byte hdr */
	addr = (char *)&sn_hdr;
	do {
		*pm++;			/* cmos ram, every other byte */
		*pm++ = (unsigned)*addr++;
	} while (--i != -1);
	i = 511;
	pm = (char *)SN_DATABUF;	/* write regular 512-byte buffer */
	addr = (char *)sn_bblk;
	do {
		*pm++;			/* cmos ram, every other byte */
		*pm++ = *addr++;
	} while (--i != -1);
	if(snwaitrdy())
		return;
	SNIOB->gobyte = SN_CMD;
}
#endif WBBLK

snintr()	/* called at pl 1 */
{
	struct buf *bp;
	register short i;
	register char *addr;
	register char *pm = (char *)SN_DATABUF;
	int cnt;
	struct deverreg snreg[1]; /* errlog: */

	i = SNIOB->status;
#ifdef HOWFAR
	if (i) 
		printf("Unix snintr: SNIOB->status = 0x%x\n",i);
#endif HOWFAR
	if (sntab.b_active == 0) {
#ifdef HOWFAR
		printf("Unix snintr: b_active==0\n");
#endif HOWFAR
		snbf = 0;	/* force real check for boot disk */
		SNIOB->mask = SN_CLEARMSK;
		SNIOB->gobyte=SN_CLRST;
		(void) snwaitrdy();
		return;
	}
	if ((bp = sntab.b_actf) == (struct buf *)NULL) {
#ifdef HOWFAR
		printf("Unix snintr: bp==0\n");
#endif HOWFAR
		snbf = 0;	/* force real check for boot disk */
		SNIOB->mask = SN_CLEARMSK;
		SNIOB->gobyte=SN_CLRST;
		(void) snwaitrdy();
		return;
	}
	cnt = 512;
	if (i) {
		cnt = 0;
		/* errlog: */
		sntab.io_stp = &snstat[physical(bp->b_dev)];
		snreg[0].drvalue = i;
		snreg[0].drname = "status";
		snreg[0].drbits = "status register";
		fmtberr(&sntab, (unsigned)physical(bp->b_dev),
			(unsigned)(SNIOB->track),	/* trk */
			(unsigned)(SNIOB->side),	/* head */
			(unsigned)(SNIOB->sector),	/* sector */
			(long)(sizeof(snreg)/sizeof(snreg[0])), /* regcnt */
			&snreg[0]);
		if (++sntab.b_errcnt > NRETRY || bp == &sncbuf) {
			bp->b_flags |= B_ERROR;
			logberr(&sntab, 1); /* errlog: */
		}
	}

	/*
	 * because a single buffer can take several io operations, 
	 * we leave it to snstart() to figure out when it's done
	 */
	if (bp->b_flags&B_ERROR || bp == &sncbuf) {
#ifdef KLUDGE
		if (!noerror)
#endif KLUDGE
		if (bp->b_flags & B_ERROR) {
			printf("Unix: HARD I/O ERROR on /dev/s%d%c ",
				physical(bp->b_dev), logical(bp->b_dev)+'a');
			if (bp != &sncbuf)
				printf("bn %d\n",
				bp->b_blkno + ((bp->b_bcount-bp->b_resid) >> 9)
				+ sn_sizes[logical(bp->b_dev)].sn_offset);
			else printf("\n");
		}
		blkacty &= ~(1<<SN1);
		sntab.b_active = 0;
		sntab.b_errcnt = 0;
		sntab.b_actf = bp->av_forw;
		iodone(bp);
	} else if (cnt && bp->b_flags & B_READ) {
		addr = bp->b_un.b_addr + bp->b_bcount - bp->b_resid;
		i = 511;
		do {
			*pm++;			/* cmos ram, every other byte */
			*addr++ = *pm++;
		} while (--i != -1);
		if (!(bp->b_blkno + ((bp->b_bcount-bp->b_resid) >> 9)
			+ sn_sizes[logical(bp->b_dev)].sn_offset)) { /* bn=0 */
			register caddr_t pm;
			register unsigned short fileid;

			pm = (char *)SN_HDRBUF;
			fileid = (*(pm+9))<<8;
			fileid |= *(pm+11);
			if (fileid == FILEID) {
				sn_bcount[boot(physical(bp->b_dev))] = 1;
#ifdef UNISOFT
			sn_bcount[boot(physical(bp->b_dev))] = 0;
#endif UNISOFT
			}
		}
	}
	bp->b_resid -= cnt;
	SNIOB->mask = SN_CLEARMSK;
	SNIOB->gobyte=SN_CLRST;
	snstart();
}

snwaitrdy()
{
	register int i, j, k;

	k = 1024;
	do {
		i = 1000;
		while (((PPADDR)->d_irb & DSKDIAG) == 0) {	/* floppy is busy */
			for(j=0;j<1024;j++);			/* don't access flpy */
			if (--i < 0) {
				printf("Unix snwaitrdy: DSKDIAG not ready\n");
				return(1);
			}
		}
		if (SNIOB->gobyte == 0)
			return(0);
	} while (--k);
	printf("Unix snwaitrdy: drive not ready\n");
	return (1);
}

snprint(dev, str)
char *str;
{
	printf("%s on sn drive %d, slice %d\n", str, (dev>>4)&0xF, dev&0x7);
}

/* This is the eject driver 
 * which uses a different major device so
 * it can tell whether the sony is being used or not
 */

/* ARGSUSED */
ejioctl(dev, cmd, adr, flag)
dev_t dev;
caddr_t adr;
{
	int unit;
	register struct buf *bp;

	if ((unit = physical(dev)) >= NSN) {
		u.u_error = EFAULT;
		return;
	}
	bp = &sncbuf;
	if ( cmd != AL_EJECT ) {
		u.u_error = ENOTTY;
		return;
	}
	if ((sn_bcount[count(unit)] != 0) || (sn_ccount[count(unit)] != 0)) {
		u.u_error = EINVAL;
		return;
	}
	GETBUF(bp);
	bp->b_dev = dev;
	bp->b_resid = cmd;		/* stash the command in resid */
	snstrategy(bp);
	iowait((struct buf *)bp);
	if (bp->b_flags & B_ERROR)
		u.u_error = EIO;
	FREEBUF(bp);
	return;
}
