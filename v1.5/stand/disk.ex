/*
 * WD1010 Disk Controller
 *
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "saio.h"
#include "sys/altblk.h"

/* WD1000 device structure */
struct device {
	char dummy0;	char data;
	char dummy1;	char precomp;
#define d_err precomp
	char dummy2;	char scount;
	char dummy3;	char sector;
	char dummy4;	char cyl_l;
	char dummy5;	char cyl_h;
	char dummy6;	char sdh;
	char dummy7;	char cmd;
#define stat cmd
};
#define WDADDR	((struct device *)0xFEE000)

	/* stat bits */
#define BUSY	0x80
#define READY	0x40
#define WFAULT	0x20
#define SCOMP	0x10
#define DATRQ	0x08
#define ERROR	0x01

	/* controller commands */
#define CRESTORE	0x12	/* restore to track 0 */
#define CSRESTORE	0x13	/* at 2.0 ms stepper rate */
#define CSEEK		0x70	/* seek at 10us */
#define CWRITE		0x30	/* write */
#define CREAD		0x20	/* read */
#define CFORMAT		0x50	/* format */

	/* sdh codes */
#define SEC512		0x20	/* 512 bytes per sector */

/*
 * disk parameters as on disk header block
 */
struct wd_par {
	int	reserved;	/* reserved for a bra for boot code */
	int	init;		/* this block has been initialized */
	short	ncyl;		/* number of cylinders */
	short	precomp;	/* starting write precompensation cyl */
	char	nhead;		/* number of heads */
	char	steprate;	/* stepping rate */
		/* stuff below here is calculated */
	int	maxbn;		/* maximum block number */
	short	recal;		/* recal on this drive requested */
	char	fill[BSIZE-20];	/* fill to BSIZE */
};

#define WD_INIT		1		/* initialization constant */
#define BRABLOCK1	0x600001FE	/* branch to block 1 put in reserved */

#define NWD		4	/* number of physical drives */
#define NSPT		17	/* number of sectors per track */
#define BBSHIFT		10	/* shift for bad block rescaling */

#define DFINTERLEAVE	1	/* default interleave */
#define DFPRECOMP	0x40	/* default precomp value */
#define DFSTEPRATE	0x00	/* default step rate */
#define DFNCYL		512	/* default number of cylinders */
#define DFNHEAD		4	/* default number of heads */

struct dpb dpb[NWD];			/* disk parameter blocks */
struct command command;			/* command block */
struct status status;			/* status block */
struct altblk wd_altblk[NWD];		/* alternate block mapping header */

wdopen(ip)
struct iob *ip;
{
	register unit = ip->i_unit;
	register struct dpb *dp = &dpb[unit];

	if (unit >= NWD) {
		printf("Invalid device 0x%x\n", unit);
		return(-1);
	}
	if (dp->initflg == 0)
		wdrddpb(unit);
}

wdstrategy(ip, func)
register struct iob *ip;
{
	register unit = ip->i_unit;
	register struct dpb *dp = &dpb[unit];
	register daddr_t bn;
	register resid;
	int offset;
	caddr_t addr;
	register struct altblk *hp = &wd_altblk[unit];
	register struct a_map *ap;
	int e;

	if (unit >= NWD)
		return(-1);
	if (hp->a_magic != ALTMAGIC)
		if (wdinit(unit))
			return(-1);
	if (ip->i_bn < 0) {
		printf("wdstrategy: bn < 0\n");
		return(-1);
	}
	resid = ip->i_cc;
loop:
	if (resid < BSIZE)
		goto out;
	offset = ip->i_cc - resid;
	addr = ip->i_ma + offset;
	bn = ip->i_bn + (offset >> BSHIFT);
	if (bn >= dp->maxbn)
		goto out;
	for (ap = hp->a_map, e = hp->a_count; e > 0; --e, --ap)
		if (bn == ap->a_altbk) {
			bn = dp->maxbn + ap->a_index;
			break;
		}
	if (wdrw(unit, func, bn, addr) != 0)
		return(-1);
	resid -= BSIZE;
	goto loop;

out:
	if (resid != 0)
		printf("wdstrategy: bn=%d resid=%d\n", ip->i_bn, resid);
	return(ip->i_cc - resid);
}

wdrw(unit, func, bn, addr)
daddr_t	bn;
caddr_t	addr;
{
	register struct command *cp = &command;
	register struct status *sp = &status;
	register struct dpb *dp = &dpb[unit];
	int sector;

	cp->bits = DATABIT;	/* interrupts disabled */
	cp->cmd = CMDDR(unit) | ((func==READ) ? CMD(READSEC) : CMD(WRITSEC));
	cp->intlvl = 1;		/* 0 not allowed */
	cp->intvec = 2;		/* 0 or 1 not allowed */
	cp->stat = sp;
	cp->cyl = bn / dp->bpc;
	cp->sec = (bn % dp->bpc) * SPB;
	cp->seccnt = 0;
	cp->succ = 0;
	for (sector=0; sector<SPB; sector++) {
		cp->sec += sector;
		cp->data = addr + sector*SECSIZE;
		if ((cp->cyl == 0) && (cp->sec == 0))	/* ignore dpb reads */
			continue;
		while (WDPSTAT & PS_BUSY)	;
		WDIOREG = cp;			/* start command */
		while (WDPSTAT & PS_BUSY)	;
		if (ERRPAT(sp)) {
			printf("%s error on device %d: bn %d, status 0x%x 0x%x\n",
				((func==READ) ? "read" : "write"),
				unit, bn, sp->errpat, sp->errword);
			return(-1);
		}
	}
	return(0);
}

/*
 * read the disk parameter block
 */
wdrddpb(unit)
register int unit;
{
	register struct dpb *dp = &dpb[unit];
	register struct command *cp = &command;
	register struct status *sp = &status;

	cp->bits = DATABIT;	/* interrupts disabled */
	cp->cmd = CMDDR(unit) | CMD(READDPB);
	cp->intlvl = 1;		/* 0 not allowed */
	cp->intvec = 2;		/* 0 or 1 not allowed */
	cp->stat = sp;
	cp->data = (char *)dp;
	while (WDPSTAT & PS_BUSY)	;
	WDIOREG = cp;			/* start command */
	while (WDPSTAT & PS_BUSY)	;
	dp->initflg++;
	if (ERRPAT(sp)) {
		printf("parameter block error on device %d: status 0x%x 0x%x\n",
			unit, sp->errpat, sp->errword);
		dp->nheads = 3;
		dp->step = 305 >> 8;		/* set max cyl no. to 306 */
		dp->ncyls = 305 & 0xFF;
		dp->nsects = 127;
	}
	dp->bpc = MAXSEC(dp) / SPB;		/* UNIX blks per cyl */
	dp->maxbn = dp->bpc * MAXCYL(dp);
	dp->maxbn -= NICALT;			/* remove bad blocks */
}

wdinit(unit)
register int unit;
{
	register struct altblk *hp = &wd_altblk[unit];

	hp->a_magic = ALTMAGIC;
	hp->a_count = 0;
	hp->a_nicbad = 0;
	if (wdrw(unit, READ, (daddr_t)0, (caddr_t)hp)) {
		hp->a_magic = 0;
		hp->a_count = 0;
		hp->a_nicbad = 0;
		return(-1);
	}
	if (hp->a_magic != ALTMAGIC) {
		hp->a_magic = ALTMAGIC;
		hp->a_count = 0;
		hp->a_nicbad = 0;
	}
	return(0);
}
