/*
 * Sunol Systems Disk and Streaming Tape Driver
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * The Sunol disk is a Corvus work-alike, so it uses the routines
 * in cv.c for reading and writing.  This file contains the
 * routines only needed by the tape unit.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "saio.h"
#include "sys/pport.h"
#include "sys/d_profile.h"
#include "sys/cv.h"
#include "sys/sunol.h"

char sunolbuf[PBSIZE];
int tapevd;		/* tape virtual drive is one beyond disk virtual drives */

/*
 * ssztape - find size of Sunol tape
 */
ssztape(fp)
register fp;
{
	register struct iob *ip = &iob[fp-3];
	register punit = ip->i_unit;
	register struct sdparam *sp;
	register tblks;

	if (sop(punit, 2, N_PARAM, TAPEVD) < 0) {
		printf("sop failure getting drive parameters for tape\n");
		return(-1);
	}
	if (cvr(punit, sunolbuf, sizeof(struct sdparam)) < 0) {
		printf("cvr failure getting drive parameters for tape\n");
		return(-1);
	}
	sp = (struct sdparam *)sunolbuf;
	tblks = ((sp->nblk_msb<<16)&0xFF0000)
		+ ((sp->nblk_nsb<<8)&0xFF00) + (sp->nblk_lsb&0xFF);
	return(tblks);
}

/*
 * sszdisk - find number of blocks on Sunol disk from
 *	block offset specified in open to physical end
 */
sszdisk(fp)
int fp;
{
	register struct iob *ip = &iob[fp-3];
	register punit = ip->i_unit;
	register vd;
	register currblks;
	register dblks;
	register struct sdparam *sp;

	dblks = currblks = 0;
	for (vd = 1; vd <= 8; vd++) {
		if (sop(punit, 2, N_PARAM, vd) < 0)
			break;
		if (cvr(punit, sunolbuf, sizeof(struct sdparam)) < 0) {
			printf("cvr failure getting drive parameters for vd %d\n", vd);
			return(-1);
		}
		sp = (struct sdparam *)sunolbuf;
		dblks += currblks;
		currblks = ((sp->nblk_msb<<16)&0xFF0000)
			+ ((sp->nblk_nsb<<8)&0xFF00) + (sp->nblk_lsb&0xFF);
	}
	if ((tapevd = vd-2) < 0)
		return(-1);
	return(dblks - ip->i_boff);
}

/*
 * stotape - copy nblks from sunol device starting at block dbn
 */
stotape(dfp, dbn, nblks)
int dfp;	/* disk file desc. */
int dbn;	/* logical disk bn within disk device */
int nblks;	/* blocks to copy - at most the number of blocks on a tape */
{
	register struct iob *ip = &iob[dfp-3];
	register tstart;	/* starting sector on tape */
	register dstart;	/* starting sector on disk */
	register dpbn;		/* disk physical bn */
	register count;

	tstart = 0;
	dpbn = ip->i_boff + dbn;
	while (nblks > 0) {
		dstart = dpbn & 0xFFFF;
		count = 0x10000 - dstart;
		if (nblks < count)	/* to end of disk v.d. or end of request */
			count = nblks;
		/*if (scopy(dfp, cvbtovd(dpbn), dstart, TAPEVD, tstart, count) < 0)*/
		if (scopy(dfp, cvbtovd(dpbn), dstart, 2, tstart, count) < 0)
			return(-1);
		tstart += count;
		dpbn += count;
		nblks -= count;
	}
}

/*
 * sfrtape - copy nblks to sunol device starting at block dbn
 */
sfrtape(dfp, dbn, nblks)
int dfp;	/* disk file desc. */
int dbn;	/* logical disk bn within disk device */
int nblks;	/* blocks to copy - at most the number of blocks on a tape */
{
	register struct iob *ip = &iob[dfp-3];
	register tstart;	/* starting sector on tape */
	register dstart;	/* starting sector on disk */
	register dpbn;		/* disk physical bn */
	register count;

	tstart = 0;
	dpbn = ip->i_boff + dbn;
	while (nblks > 0) {
		dstart = dpbn & 0xFFFF;
		count = 0x10000 - dstart;
		if (nblks < count)	/* to end of disk v.d. or end of request */
			count = nblks;
		if (scopy(dfp, TAPEVD, tstart, cvbtovd(dpbn), dstart, count) < 0)
			return(-1);
		tstart += count;
		dpbn += count;
		nblks -= count;
	}
}

struct copyparam scopyparam;		/* parameter block for copy command */

scopy(dfp, srcvd, srcstart, destvd, deststart, count)
register dfp, count;
{
	int punit = iob[dfp-3].i_unit;
	register struct device_d *addr = pro_da[punit];
	register char *ira = &(addr->d_ira);
	register struct copyparam *pp = &scopyparam;
	register char *cp;
	register i;
	register char status;
	int zero = 0;

	i = pp->command = N_COPY;
	i += (pp->srcvd = srcvd);
	i += (pp->lsrcstart = srcstart & 0xFF);
	i += (pp->msrcstart = srcstart >> 8 & 0xFF);
	pp->fill1 = 0;
	i += (pp->destvd = destvd);
	i += (pp->ldeststart = deststart & 0xFF);
	i += (pp->mdeststart = deststart >> 8 & 0xFF);
	pp->fill2 = 0;
	i += (pp->lcount = count & 0xFF);
	i += (pp->mcount = count >> 8 & 0xFF);
	i = ((i&0xFF) ^ 0xFF) + 1;
	pp->chksum = i;

#ifdef notdef
cp = (char *)pp;
for (i = sizeof(struct copyparam); i > 0; i--)
printf("0x%x ", (*cp++)&0xFF);
printf("\n");
#endif notdef
	addr->d_ddra = 0xff;	/* port A to output */
	addr->d_irb &= ~0x08;	/* bidirectional driver to output */

	cp = (char *)pp;
	for (i = sizeof(struct copyparam); i > 0; i--) {
		while (((status = addr->d_irb) & ST_BUSY) == 0);
		if ((status & ST_HTOC) == 0)
			break;
		*ira = *cp++;
	}
	for (;;) {
		if (cvwait(addr) < 0) {
			printf("leaving scopy before fill, %d bytes left\n", i);
			return(-1);
		}
		if ((addr->d_irb & ST_HTOC) == 0)
			break;
		*ira = zero;
	}
	if (i > 0) {
		printf("scopy: %d bytes short\n", i);
		return(-1);
	}
	addr->d_ddra = 0x00;	/* data direction port A bits to input */
	addr->d_irb |= 0x08;	/* bidirectional driver to input */
	for (i=1000; i>0; i--);	/* pause so controller goes busy */
	while (((status = addr->d_irb) & ST_BUSY) == 0);	/* cvwait without timeout */
	if (status & ST_HTOC) {	/* controller expects something */
		printf("scopy: no status available after command done\n");
		return(-1);
	}
	if ((status = *ira) == 0) {
printf("scopy: good status after copy\n");
		return(0);
	}
printf("scopy: bad status 0x%x (0x%x)\n", status&0xFF, status&0x1F);
	/* non-zero status is followed by 10 bytes */
	if (cvr(punit, &(pp->srcvd), (sizeof(struct copyparam)-2)) < 0) {
		printf("can't read status block after copy\n");
		{
		/* sunolbuf still has parameters from call to ssztape */
		register struct sdparam *sp = (struct sdparam *)sunolbuf;
		printf("%s: revision 0x%x, ROM version 0x%x\n",
			sp->fwmsg, sp->revision, sp->romv);
		}
		return(-1);
	}
	/* for backups (destvd 9), up to 14 blocks may be missed by
		the copy command and must be handled in start/stop mode */
	count = pp->lcount + (pp->mcount<<8);
	srcstart = pp->lsrcstart + (pp->msrcstart<<8) + cvvdtob(srcvd);
	deststart = pp->ldeststart + (pp->mdeststart<<8) + cvvdtob(destvd);
printf("scopy: %d blocks, src 0x%x, dest 0x%x\n", count, srcstart, deststart);
	while (count > 0) {
		lseek(dfp, (off_t)(srcstart*PBSIZE), 0);
		if ((read(dfp,sunolbuf,PBSIZE)) != PBSIZE) {
			printf("read error bn=%d\n", srcstart);
			return(-1);
		}
		lseek(dfp, (off_t)(deststart*PBSIZE), 0);
		if (write(dfp,sunolbuf,PBSIZE) != PBSIZE) {
			printf("write error bn=%d\n", deststart);
			return(-1);
		}
		count--;
		srcstart++;
		deststart++;
	}
	return(0);
}

/*
 * sop - issue command to sunol
 */
/* VARARGS3 */
sop(unit, na, a)
{
	register int *ap;
	register struct device_d *addr = pro_da[unit];

	addr->d_ddra = 0xff;	/* port A to output */
	addr->d_irb &= ~0x08;	/* bidirectional driver to output */

	ap = &a;
	for (; na > 0; na--, ap++) {
		if (cvwait(addr) < 0)
			return(-1);
		addr->d_ira = *ap;
	}
	return(srstat(addr));		/* read status */
}

#ifdef notdef
	if (sop512(punit, 2, N_DIAGN, 1) < 0) { /* ??? may not be needed */
		printf("sop512 failure setting diag mode\n");
		return(-1);
	}
/*
 * sop512 - issue command to sunol followed by 512 bytes of filler
 */
/* VARARGS3 */
sop512(unit, na, a)
{
	register int *ap;
	register struct device_d *addr = pro_da[unit];
	register zero = 0;
	register i;

	addr->d_ddra = 0xff;	/* port A to output */
	addr->d_irb &= ~0x08;	/* bidirectional driver to output */

	ap = &a;
	for (; na > 0; na--, ap++) {
		if (cvwait(addr) < 0) {
			printf(" timeout in sop512 ");
			return(-1);
		}
		addr->d_ira = *ap;
	}
	for (i = 0; i < 512; i++) {
		if (cvwait(addr) < 0) {
			printf(" timeout in sop512 ");
			return(-1);
		}
		addr->d_ira = zero;
	}
	return(srstat(addr));		/* read status */
}
#endif notdef

srstat(addr)
register struct device_d *addr;
{
	register char status;

	addr->d_ddra = 0x00;	/* data direction port A bits to input */
	addr->d_irb |= 0x08;	/* bidirectional driver to input */
	if (cvwait(addr) < 0)
		return(-1);
/*	cvqwait(addr);		/* quick wait (macro without timeout) */
	if (status = addr->d_ira) {
	/*printf("bad status 0x%x (0x%x)\n", status&0xFF, status&0x1F);*/
		return(-1);
	}
	return 0;
}
