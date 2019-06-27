/* #define HOWFAR */
/* #define UNISOFT		/* allow access to boot partition */
/*
 * Priam Datatower
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
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
#include "sys/reg.h"
#include "sys/altblk.h"
#include "sys/cops.h"
#include "sys/pport.h"
#include "sys/priam.h"
#include "sys/swapsz.h"

#define logical(x)	(minor(x) & 7)		/* eight logicals per phys */
#define physical(x)	((minor(x) & 0xF0) >> 4)/* 10 physical devs */
#define splpm	spl5

/*
 * When the disk is first opened its size is determined and pm_sizes is
 * initialized accordingly (in pmdinit).
 *
 * The first 100 blocks are reserved for the boot program and
 * are inaccessible via unix.
 */
#define	MAXBOOT	100
struct pm_sizes {
	daddr_t	sz_offset;
	daddr_t	sz_size;
} pm_sizes[NPM][8];

#define GETBUF(bp)	splpm(); \
	while (bp->b_flags & B_BUSY) { \
		bp->b_flags |= B_WANTED; \
		(void)sleep((caddr_t)bp, PRIBIO+1); \
	} \
	bp->b_flags |= B_BUSY; \
	spl0()

#define FREEBUF(bp)	splpm(); \
	if (bp->b_flags & B_WANTED) \
		wakeup((caddr_t)bp); \
	bp->b_flags = 0; \
	spl0()

struct	iostat	pmstat[NPM];
struct	iobuf	pmtab = tabinit(PM3,pmstat);	/* active buffer header */
struct	buf	pmcbuf;		/* command buffer */
struct	buf	pmrbuf;
char	pmiflag[NPM];
char	pmresults[NPMRES];	/* result regs. for last command comp ack */
int	pmnblks[NPM];		/* number of blocks on disk */
int	pmcblkcnt;		/* current block count */
struct pmfmtparms pmfmt;	/* format parameters for packet-based format cmd */
struct pmfmtstat pmfstat;	/* format status from packet-based format cmd */
/* variables needed to share info for interrupt routines */
char	pmstate;		/* idling or waiting for interrupt */
#define	IDLING		0
#define	INITING		1
#define	READING1	2	/* 1st intr while reading */
#define	READING2	3	/* 2nd intr while reading */
#define	WRITING1	4	/* 1st intr while writing */
#define	WRITING2	5	/* 2nd intr while writing */
#define	FMTING1		6	/* 1st intr while formatting */
#define	FMTING2		7	/* 2nd intr while formatting */
#define	FMTING3		8	/* 3rd intr while formatting */
#define	FMTING4		9	/* 4th intr while formatting */
caddr_t	pmma;			/* current memory address */
char	pmierror;		/* error value from intr */

pmopen(dev)
register dev;
{
	register punit;
	extern char slot[];

	punit = physical(dev);
	if (punit >= NPM) {
		u.u_error = ENXIO;
		return(-1);
	}
	if (slot[punit] != PM3) {
		printf("Unix pmopen: no Priam card in slot %d\n", punit);
		u.u_error = ENODEV;
		return(-1);
	}
	if (pmiflag[punit] == 0) {
		if (pmdinit(dev)) {
			u.u_error = EIO;
			return(-1);
		}
		pmiflag[punit]++;
	}
	return(0);
}

/*
 * pmdinit - initialize device (sequence up)
 */
pmdinit(dev)
register dev_t dev;
{
	register struct buf *bp = &pmcbuf;
	register punit, i;
	register struct pm_sizes *sp;
	register daddr_t offset, size;

	punit = physical(dev);
	GETBUF(bp);
	bp->b_dev = dev;
	bp->b_resid = PMRDEVPMS;
	pmnblks[punit] = 0;
	pmstrategy(bp);
	iowait(bp);
	i = bp->b_flags & B_ERROR;
	FREEBUF(bp);
	if (i)
		return(-1);
	sp = pm_sizes[punit];
	offset = MAXBOOT + 1;			/* avoid boot area */
	size = pmnblks[punit] - offset;
	sp[7].sz_offset = offset;		/* h = entire */
	sp[7].sz_size = size;
	sp[1].sz_offset = offset;		/* b = swap */
	sp[1].sz_size = PMNSWAP;
	offset += PMNSWAP;
	size -= PMNSWAP;
	if (size < 0) {
		printf("Unix pmdinit: disk size = %d (insufficient)\n", pmnblks[punit]);
		return(-1);
	}
	sp[0].sz_offset = offset;		/* a = root */
	sp[0].sz_size = size;
	for (i = 2; i < 7; i++) {
		sp[i].sz_offset = (daddr_t)(0);	/* c-g =spare */
		sp[i].sz_size = (daddr_t)0;
	}
	return(0);
}

pmstrategy(bp)
register struct buf *bp;
{
	register punit, lunit, bn;

	punit = physical(bp->b_dev);
	if (bp == &pmcbuf) {			/* if command */
		pmstat[punit].io_misc++; /* errlog: */
		splpm();
		if (pmtab.b_actf == (struct buf *)NULL) /* set up links */
			pmtab.b_actf = bp;
		else
			pmtab.b_actl->av_forw = bp;
		pmtab.b_actl = bp;
		bp->av_forw = (struct buf *)NULL;
	} else {
		lunit = logical(bp->b_dev);
		bn = bp->b_blkno + pm_sizes[punit][lunit].sz_offset;
#ifdef UNISOFT
		if (bp->b_blkno < 0 ) {
#else UNISOFT
		if (bp->b_blkno < 0 || bn <= MAXBOOT) {
#endif UNISOFT
#ifdef HOWFAR
			prdev("pmstrategy: illegal blkno", bp->b_dev);
			printf("blkno=%d bcount=%d\n", bp->b_blkno, bp->b_bcount);
#endif HOWFAR
			bp->b_flags |= B_ERROR;
			iodone(bp);
			return;
		}
		pmstat[punit].io_ops++; /* errlog: */
		bp->b_resid = bn;	/* resid for disksort */
		splpm();
		disksort(&pmtab, bp);
	}
	if (pmtab.b_active == 0)
		pmstart();
	SPL0();
}

pmstart()
{
	register struct buf *bp;
	register offset, bn, lunit, punit;

loop:
	if ((bp = pmtab.b_actf) == (struct buf *)NULL)
		return;
	if (pmtab.b_active == 0) {
		pmtab.b_active = 1;
		if (bp != &pmcbuf)
			bp->b_resid = bp->b_bcount;
	}
	blkacty |= (1<<PM3);
	if (bp == &pmcbuf) {
		if (pmcmd(bp) != 0) {	/* b_resid holds the command */
			bp->b_flags |= B_ERROR;
			goto next;
		}
		return;
	}
	lunit = logical(bp->b_dev);
	punit = physical(bp->b_dev);
	offset = bp->b_bcount - bp->b_resid;
	bn = bp->b_blkno + btod(offset);	/* logical block number */
	if (bp->b_resid < BSIZE || bn >= pm_sizes[punit][lunit].sz_size) {
	next:
#ifdef HOWFAR
		if (bp->b_resid != 0)
			printf("Unix pmstart: blkno=%d resid=%d bn=%d\n",
				bp->b_blkno, bp->b_resid, bn);
#endif HOWFAR
		pmtab.b_active = 0;
		pmtab.b_errcnt = 0;
		blkacty &= ~(1<<PM3);
		pmtab.b_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	bn += pm_sizes[punit][lunit].sz_offset;	/* physical block number */
	if (pmrw(punit, bn, bp->b_flags&B_READ, bp->b_un.b_addr+offset) != 0) {
		bp->b_flags |= B_ERROR;
		goto next;
	}
}

/* ARGSUSED */
pmioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register punit;
	register struct buf *bp;

	punit = physical(dev);
	if (punit >= NPM) {
		u.u_error = EINVAL;
		return;
	}
	switch (cmd) {
	case UIOCSIZE:		/* get size of Priam disk */
		if (copyout((caddr_t)&pmnblks[punit], (caddr_t)addr, 4))
			u.u_error = EFAULT;
		break;
	case UIOCFORMAT:
		if (!suser()) {
			u.u_error = EPERM;
			return;
		}
		bp = &pmcbuf;
		GETBUF(bp);
		bp->b_dev = dev;
		bp->b_resid = PMPKTXFER;	/* stash the command in resid */
		pmstrategy(bp);
		iowait(bp);
		if (bp->b_flags & B_ERROR)
			u.u_error = EIO;
		FREEBUF(bp);
		break;
	default:
		u.u_error = ENOTTY;
		break;
	}
}

pmcmd(bp)
register struct buf *bp;
{
	register struct pm_base *pmhwbase;
	register struct pm_base *addr;
	register punit;
	register struct pmfmtparms *fmtp = &pmfmt;

	punit = physical(bp->b_dev);
	pmierror = 0;
	switch (bp->b_resid) {
	case PMRDEVPMS:		/* read device parameters (spin up if necessary) */
		pmhwbase = pmaddr(punit);	/* base address for this card */
		addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
		if (pmwaitrf(addr)) {
			printf("Unix pmcmd: timeout before read dev parms\n");
			return(-1);
		}
		addr->p0 = 0;			/* device always 0 */
		addr = pmBIaddr(pmhwbase);	/* byte mode - intr enabled */
		pmstate = INITING;
		addr->cmdreg = PMRDEVPMS;
		return(0);
	case PMPKTXFER:		/* format disk (done with packet-based command) */
		pmhwbase = pmaddr(punit);	/* base address for this card */
		addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
		if (pmwaitrf(addr)) {
			printf("Unix pmcmd: timeout before format\n");
			return(-1);
		}
		fmtp->pm_opcode = PMFMT;/* set up format parms packet */
		fmtp->pm_devsel = 0;	/* device select = 0 (no daisy chaining) */
		fmtp->pm_scntl = PMFBD;	/* fill byte disabled, media type = 0 */
		fmtp->pm_fill = 0;	/* fill byte */
		fmtp->pm_ssize = 536;	/* sector size */
		fmtp->pm_dcntl = 0;	/* defect mapping enabled, 0 spares/track */
		fmtp->pm_ncyl = 10;	/* # cyls for alt tracks and alt sectors */
		fmtp->pm_cif = 0;	/* cyl interleave factor */
		fmtp->pm_hif = 1;	/* head interleave factor */
		fmtp->pm_sif = 1;	/* sector interleave factor */
		fmtp->pm_sitl = 0;	/* sector interleave table length */
		addr->p0 = 0;			/* device always 0 */
		addr->p2 = 0;			/* MSB of packet length */
		addr->p3 = sizeof(struct pmfmtparms);	/* LSB of packet length */
		addr = pmBIaddr(pmhwbase);	/* byte mode - intr enabled */
		pmstate = FMTING1;
		addr->cmdreg = PMPKTXFER;
		return(0);
	default:
		return(-1);
	}
}

pmrw(unit, bn, rflag, addr)
register unit;
register bn;
register rflag;
register caddr_t addr;
{
	register struct pm_base *pmhwbase;
	register struct pm_base *hwaddr;
	register tmp;

	pmhwbase = pmaddr(unit);	/* base address for this card */
	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if (pmwaitrf(hwaddr)) {
		printf("pmrw: timeout before setting %s parameters ",
			rflag?"read":"write");
		printf("card %d, bn %d, addr 0x%x\n", unit, bn, addr);
		return(-1);
	}
	hwaddr->p0 = 0;			/* device always 0 */
	tmp = bn;
	hwaddr->p3 = tmp & 0xFF;	/* most sig. byte */
	tmp = tmp >> 8;
	hwaddr->p2 = tmp & 0xFF;	/* middle byte */
	tmp = tmp >> 8;
	hwaddr->p1 = tmp & 0xFF;	/* least sig. byte */
	hwaddr->p4 = 1;			/* operation count = 1 sector */
	pmcblkcnt = 1;
	hwaddr = pmBIaddr(pmhwbase);	/* byte mode - intr enabled */
	pmma = addr;
	pmierror = 0;
	pmstate = rflag ? READING1 : WRITING1;
	hwaddr->cmdreg = rflag ? PMREAD : PMWRITE;	/* start r/w */
	return(0);
}

pmintr(ap)
struct args *ap;
{
	register struct pm_base *pmhwbase;
	register struct pm_base *addr;
	register struct buf *bp;
	register char status;
	dev_t punit;

	(void) splpm();
	punit = ap->a_dev;
	pmhwbase = pmaddr(punit);	/* base address for this card */
	addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if (pmtab.b_active == 0) {
#ifdef HOWFAR
		printf("Unix pmintr: b_active == 0\n");
#endif HOWFAR
spurious:
		addr->cmdreg = 0;	/* clear spurious intr (clrb) */
		return;
	}
	if ((bp = pmtab.b_actf) == (struct buf *)NULL) {
#ifdef HOWFAR
		printf("Unix pmintr: b_actf == NULL\n");
#endif HOWFAR
		goto spurious;
	}
	if (bp == &pmcbuf) {		/* if command */
		switch (bp->b_resid) {
		case PMRDEVPMS:	/* read device parameters (spin up if necessary) */
			if (pmstate != INITING)
				goto spurious;
			if (pmierror = pmackcc(addr)) {
				printf("error on read dev parms: /dev/pm%d%c ",
					punit, 'a'+logical(bp->b_dev));
				printf("status 0x%x\n", pmierror);
				bp->b_flags |= B_ERROR;
			} else {
				register nheads, ncyls, nsecs;
				nheads = PMNH(pmresults[1]);
				ncyls = PMNC(pmresults[1], pmresults[2]);
				nsecs = PMNS(pmresults[3]);
				pmnblks[punit] = nheads * ncyls * nsecs;
			}
			break;
		case PMPKTXFER:	/* format disk (done with packet-based command) */
			switch (pmstate) {
			case FMTING1:		/* intr to transfer packet */
				if ((addr->status & DTREQ) == 0)
					goto ackcc;	/* if not waiting for data */
				{		/* send packet */
					register char *cp = (char *)&pmfmt;
					register i = sizeof(struct pmfmtparms);
					do {
						addr->pdata = *cp++;
					} while ( --i );
				}
				(void)pmackdt(addr);	/* Apple's code doesn't check */
		wait:		pmstate++;	/* not done yet - wait for next intr */
				return;
			case FMTING2:		/* intr after packet complete */
				(void)pmackcc(addr);
				(void)pmwaitrf(addr);
				addr->p0 = 0;	/* packet ID = 0 */
				addr = pmBIaddr(pmhwbase);
				addr->cmdreg = PMPKTRST;/* read packet status */
				goto wait;
			case FMTING3:		/* intr when status can be read */
				if ((addr->status & DTREQ) == 0)
					goto ackcc;	/* if not waiting for data */
				{		/* read packet status */
					register char *cp = (char *)&pmfstat;
					register i = sizeof(struct pmfmtstat);
					do {
						*cp++ = addr->pdata;
					} while ( --i );
				}
				(void)pmackdt(addr);	/* Apple's code doesn't check */
				goto wait;
			case FMTING4:		/* intr after read packet status */
		ackcc:		(void)pmackcc(addr);
				if ((pmfstat.pm_pstate != PMPKTCOMP)
					|| pmfstat.pm_pristat) {
					pmierror = pmfstat.pm_pristat;
					bp->b_flags |= B_ERROR;
				}
			}
			break;
		}
		goto out;
	}
	if ((addr->status & CMD_DONE) == 0) {	/* block transfer interrupt */
		if (!pmcblkcnt) {	/* 2nd interrupt */
			if ((pmstate==READING2) || (pmstate==WRITING2))
				goto done;
			printf("Unix pmintr: state=%d, expecting 2nd\n", pmstate);
			bp->b_flags |= B_ERROR;
			goto out;
		}
		/* 1st interrupt */
		if ((pmstate!=READING1) && (pmstate!=WRITING1)) {
			printf("Unix pmintr: state=%d, expecting 1st\n", pmstate);
			bp->b_flags |= B_ERROR;
			goto out;
		}
		pmcblkcnt--;
		if (pmstate == READING1)
			pmrsect(punit);	/* sets pmierror for parity */
		else
			pmwsect(punit);
		pmstate++;	/* set to READING2 or WRITING2 */
		addr->cmdreg = PMCBTI;	/* clear block transfer intr */
		addr = pmBWIaddr(pmhwbase);	/* setup for next intr */
		status = addr->status;
		return;
	} else {		/* cmd completed (error) */
		if ((pmstate!=READING2) && (pmstate!=WRITING2))
			printf("Unix pmintr: command complete, state=%d\n", pmstate);
done:
		if (status = pmackcc(addr)) {	/* ack intr */
			if ((pmstate==READING2) && (pmresults[0] == PMECCERR))
				/* ECC err requiring scavenger write */
				status = PMECCERR;
			else if (pmresults[0] == PMDTIMOUT)
				/* dev timeout - they reissue read/write */
				status = PMDTIMOUT;
		}
		if (pmierror |= status)
			bp->b_flags |= B_ERROR;
	}
out:
	/*
	 * because a single buffer can take several io operations, 
	 * we leave it to pmstart() to figure out when it's done
	 */
	if (bp->b_flags&B_ERROR || bp == &pmcbuf) {
		if (bp->b_flags & B_ERROR) {
			{
			register bn = 0;
			struct deverreg pmreg[2];

			printf("Unix: HARD I/O ERROR on /dev/pm%d%c ",
				punit, logical(bp->b_dev)+'a');
			if (pmierror) {
				if (pmierror & PMPERROR)
					printf("parity error ");
				if (status = (pmierror & ~PMPERROR))
					printf("error code 0x%x ", status);
			}
			if (bp != &pmcbuf) {
				bn = bp->b_blkno + btod(bp->b_bcount - bp->b_resid)
					+ pm_sizes[punit][logical(bp->b_dev)].sz_offset;
				printf("bn %d\n", bn);
			} else printf("\n");
			/* error logging */
			pmtab.io_stp = &pmstat[punit];
			pmreg[0].draddr = (long)0;
			pmreg[0].drvalue = pmierror;
			pmreg[0].drname = "pmierror";
			pmreg[0].drbits = "Priam disk status code";
			pmreg[1].draddr = (long)0;
			pmreg[1].drvalue = pmstate;
			pmreg[1].drname = "pmstate";
			pmreg[1].drbits = "Priam driver state";
			fmtberr(&pmtab,
				(unsigned)punit,
				(unsigned)0,		/* cylinder */
				(unsigned)0,		/* track */
				(unsigned)bn,		/* sector */
				(long)(sizeof(pmreg)/sizeof(pmreg[0])), /* regcnt */
				&pmreg[0],&pmreg[1]);
			}
			logberr(&pmtab, 1);	/* log hard (unrecovered) error */
		}
		blkacty &= ~(1<<PM3);
		pmtab.b_active = 0;
		pmtab.b_errcnt = 0;
		pmtab.b_actf = bp->av_forw;
		iodone(bp);
	} else
		bp->b_resid -= 512;
	pmstate = IDLING;
	pmstart();
}

/*
 * read block to memory at pmma
 * from PRIAMASM.TEXT - READ_SECT
 */
pmrsect(punit)
register punit;
{
	register struct pm_base *pmhwbase;
	register struct pm_base *hwaddr;
	register short *waddr;
	register i;

	pmhwbase = pmaddr(punit);	/* base address for this card */
	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmPaddr(pmhwbase);	/* parity checking enabled */
	waddr = (short *)pmma;
	*waddr = hwaddr->data;		/* start read of first word */
	i = 12;
	do {				/* read and throw away header */
		*waddr = hwaddr->data;
	} while ( --i );		/* read 24-byte header */
	waddr = (short *)pmma;
	i = 255;
	do {
		*waddr++ = hwaddr->data;
	} while ( --i );		/* read 510 bytes */
	hwaddr = pmNaddr(hwaddr);	/* read last 2 avoiding device cycle */
	*waddr = hwaddr->data;
	hwaddr = pmpaddr(punit);	/* parity in lo select space */
	pmierror |= (hwaddr->parity & PMPERROR);
}

/*
 * write block from memory at pmma
 * from PRIAMASM.TEXT - WRITE_SECT
 */
pmwsect(punit)
register punit;
{
	register struct pm_base *pmhwbase;
	register struct pm_base *hwaddr;
	register short *waddr;
	register i;
	register short tmp = 0;

	pmhwbase = pmaddr(punit);	/* base address for this card */
	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmhwbase;
	i = 12;
	do {				/* write dummy header */
		hwaddr->data = tmp;
	} while ( --i );		/* write 24 bytes */
	waddr = (short *)pmma;
	i = 256;
	do {
		hwaddr->data = *waddr++;
	} while ( --i );		/* write 512 bytes */
}

/*
 * wait for register file not busy
 * from PRIAMASM.TEXT and PRIAMCARDASM.TEXT - WAITRF
 */
pmwaitrf(addr)
register struct pm_base *addr;
{
	register i;

	i = TIMELIMIT;
	while ((addr->status & ISR_BUSY) != 0) {
		if (--i)
			continue;
		return(-1);
	}
	return(0);
}

/*
 * acknowledge command completion
 * from PRIAMASM.TEXT and PRIAMCARDASM.TEXT - ACKCC
 */
pmackcc(addr)
register struct pm_base *addr;	/* pmBWaddr */
{
	register i;

	i = TIMELIMIT;
	while ((addr->status & CMD_DONE) == 0) {	/* wait for cmd done */
		if (--i)
			continue;
		printf("pmackcc: timeout waiting for CMD_DONE\n");
		return(-1);
	}
	pmresults[0] = addr->r0;	/* transaction status */
	pmresults[1] = addr->r1;
	pmresults[2] = addr->r2;
	pmresults[3] = addr->r3;
	pmresults[4] = addr->r4;
	pmresults[5] = addr->r5;
	addr->cmdreg = 0;		/* send command acknowledge (clrb) */
	i = TIMELIMIT;
	while ((addr->status & CMD_DONE) != 0) {	/* wait 'til reset */
		if (--i)
			continue;
		printf("pmackcc: timeout waiting for CMD_DONE reset\n");
		return(-1);
	}
	return(pmresults[0] & PMCTYPE);	/* error completion type */
}

/*
 * acknowledge data transfer
 * from PRIAMASM.TEXT - ACKDT
 * Their code does not check the return value, so who knows what'll happen
 * if it times out?
 */
pmackdt(addr)
register struct pm_base *addr;	/* pmBWaddr */
{
	register i;

	addr->cmdreg = PMCBTI;		/* clear block transfer intr */
	i = TIMELIMIT;
	while ((addr->status & BTR_INT) != 0) {
		if (--i)
			continue;
		printf("Unix pmackdt: timeout\n");
		return(-1);
	}
	addr = pmIaddr(addr);		/* pmBWIaddr */
	i = addr->status & BTR_INT;	/* setup for next intr */
	return(0);
}

/*
 * pmcinit - initialize controller (turn on motor)
 * called from oem7init (config.c)
 */
pmcinit(unit)
register unit;
{
	register struct pm_base *addr;
	register i;

	addr = pmaddr(unit);	/* hwbase */
	addr = pmBWaddr(addr);	/* byte mode - waiting */
	if ((addr->status & CMD_DONE) != 0) {	/* cmd complete must be false */
		if (pmwaitrf(addr)) {
			printf("timeout with status CMD_DONE\n");
			goto err;
		}
		addr->cmdreg = 0;	/* command ack power up (clrb) */
		i = TIMELIMIT;
		while ((addr->status & CMD_DONE) == 0) {
			if (--i)
				continue;
			printf("timeout waiting for power up\n");
			goto err;
		}
	}
	if (pmwaitrf(addr)) {
		printf("timeout before software reset\n");
		goto err;
	}
	addr->cmdreg = PMRESET;		/* issue software reset */
	if ((i=pmackcc(addr)) != PMICOMP) {
		printf("software reset error 0x%x\n", i);
		goto err;
	}
	if (pmwaitrf(addr)) {
		printf("timeout before read mode\n");
		goto err;
	}
	addr->p0 = 0;		/* (clrb) */
	addr->cmdreg = PMRMODE;
	if (pmackcc(addr)) {
		printf("read mode cmd ack failed\n");
		goto err;
	}
	if (pmwaitrf(addr)) {
		printf("timeout after read mode\n");
		goto err;
	}
	addr->p0 = 0;		/* (clrb) */
	if (pmresults[3] == 2) {
		printf("Smart E controller not implemented\n");
		goto err;
	}
	addr->p1 = PMLOGSECT;		/* set mode "logical sector mode" */
	addr->p2 = 0;		/* (clrb) */
	addr->cmdreg = PMSMODE;
	if (pmackcc(addr)) {
		printf("set mode cmd ack failed\n");
		goto err;
	}
	if (pmwaitrf(addr)) {
		printf("timeout after set mode\n");
		goto err;
	}
	addr->p0 = 0x40;
	addr->p1 = PMPSEL1;
	addr->p2 = PM1PARMS;
	addr->cmdreg = PMSETP;
	if (pmackcc(addr)) {
		printf("set parms 1 cmd ack failed\n");
		goto err;
	}
	if (pmwaitrf(addr)) {
		printf("timeout after set parms 1\n");
		goto err;
	}
	addr->p0 = 0x40;
	addr->p1 = PMPSEL0;
	addr->p2 = PM0PARMS;
	addr->cmdreg = PMSETP;
	if (pmackcc(addr)) {
		printf("set parms 0 cmd ack failed\n");
		goto err;
	}

	return 0;
err:	printf("Unix pmcinit: can't initialize controller\n");
	return(-1);
}

pmread(dev)
dev_t dev;
{
	physio(pmstrategy, &pmrbuf, dev, B_READ);
}

pmwrite(dev)
dev_t dev;
{
	physio(pmstrategy, &pmrbuf, dev, B_WRITE);
}

pmprint(dev, str)
char *str;
{
	printf("%s on priam drive %d, slice %d\n", str, (dev>>4)&0xF, dev&7);
}
