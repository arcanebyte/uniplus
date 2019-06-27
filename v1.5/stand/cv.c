/*
 * Corvus Disk System
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "saio.h"
#include "sys/cops.h"
#include "sys/pport.h"
#include "sys/d_profile.h"
#include "sys/cv.h"

#define cvqwait(a)	while(((a)->d_irb & ST_BUSY) == 0)

cvwait(addr)
register struct device_d *addr;
{
	register i;

	for (i=10; i>0; i--);	/* pause for Sunol */
	i = 1000000;
	while ((addr->d_irb & ST_BUSY) == 0)
		if (i-- == 0) {
			printf("cvwait:  timeout waiting for controller\n");
			return(-1);
		}
	return(0);
}

char cviflag[NPPDEVS];		/* has it been opened before? */

/*
 * cvopen - check for existence of controller
 */
cvopen(iobp)
register struct iob *iobp;
{
	register punit = iobp->i_unit;

	if (punit) {	/* for expansion slot check slot number */
		if (!PPOK(punit)) {
			printf("Invalid device 0x%x\n", punit);
			return(-1);
		}
	}
	if (cviflag[punit] == 0) {
		if (cvinit(punit))
			return(-1);
		cviflag[punit]++;
	}
	return 0;
}

/*
 * cvinit - initialize drive first time
 */
cvinit(punit)
register punit;
{
	register struct device_d *devp = pro_da[punit];
	register char zero = 0;

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
	return 0;
}

cvstrategy(ip, func)
register struct iob *ip;
{
	register punit = ip->i_unit;
	register daddr_t bn;
	register resid;
	int offset;
	caddr_t addr;

	if (ip->i_bn < 0) {
		printf("cvstrategy: bn < 0\n");
		return(-1);
	}
	resid = ip->i_cc;
loop:
	if (resid < PBSIZE)
		goto out;
	offset = ip->i_cc - resid;
	addr = ip->i_ma + offset;
	bn = ip->i_bn + (offset >> PBSHIFT);
	if (cvrw(punit, bn, func&F_READ, addr) != 0)
		return(-1);
	resid -= PBSIZE;
	goto loop;

out:
	if (resid != 0)
		printf("cvstrategy: bn=%d resid=%d\n", ip->i_bn, resid);
	return(ip->i_cc - resid);
}

cvrw(punit, bn, rflag, buff)
register punit;
daddr_t bn;
register rflag;
register caddr_t buff;
{
	register struct device_d *addr;
	register char status;

	if (cvop(punit, 4, rflag ? N_R512 : N_W512,
		cvbtovd(bn), bn & 0xff, bn>>8 & 0xff) < 0)
		return(-1);

	if (!rflag && cvw(punit, buff) < 0)
		return(-1);
	addr = pro_da[punit];
	addr->d_ddra = 0x00;	/* data direction port A bits to input */
	addr->d_irb |= 0x08;	/* bidirectional driver to input */
	if (cvwait(addr) < 0)
		return(-1);
/*	cvqwait(addr);		/* quick wait (macro without timeout) */
	if (status = addr->d_ira) {
		printf("bad status 0x%x\n", status&0xFF);
		return(-1);
	}
	if (rflag && cvr(punit, buff, PBSIZE) < 0)
		return(-1);
	return(0);
}

/* VARARGS3 */
cvop(unit, na, a)
{
	register int *ap;
	register struct device_d *addr = pro_da[unit];

	addr->d_ddra = 0xff;	/* port A to output */
	addr->d_irb &= ~0x08;	/* bidirectional driver to output */

	ap = &a;
	for (; na > 0; na--, ap++) {
		if (cvwait(addr) < 0) {
			printf("cvop: timeout\n");
			return(-1);
		}
		addr->d_ira = *ap;
	}
	return 0;
}

cvw(unit, buff)
register char *buff;
{
	register char *ira;
	register struct device_d *addr = pro_da[unit];
	register n = 512;
	register zero = 0;
	register char status;

	if (cvwait(addr) < 0) {
		printf("leaving cvw before trying any writes\n");
		return(-1);
	}
	ira = &(addr->d_ira);
	for ( ; n > 0; n--) {
		while (((status = addr->d_irb) & ST_BUSY) == 0);
		if ((status & ST_HTOC) == 0)
			break;
		*ira = *buff++;
	}
	for (;;) {
		if (cvwait(addr) < 0) {
			printf("leaving cvw before fill, %d bytes left\n", n);
			return(-1);
		}
		if ((addr->d_irb & ST_HTOC) == 0)
			break;
		*ira = zero;
	}
	if (n > 0) {
		printf("cvw: %d bytes short\n", n);
		return -1;
	}
	return 0;
}

cvr(unit, buff, n)
register char *buff;
register n;
{
	register char *ira;
	register struct device_d *addr = pro_da[unit];
	register char status;

	if (cvwait(addr) < 0) {
		printf("leaving cvr before trying any reads\n");
		return(-1);
	}
	ira = &(addr->d_ira);
	for ( ; n > 0; n--) {
		while (((status = addr->d_irb) & ST_BUSY) == 0);
		if (status & ST_HTOC)
			break;
		*buff++ = *ira;
	}
	if (n > 0) {
		printf("cvr: %d bytes short\n", n);
		return -1;
	}
	for (;;) {
		if (cvwait(addr) < 0) {
			printf("leaving cvr after reading all bytes\n");
			return(-1);
		}
		if (addr->d_irb & ST_HTOC)
			break;
		n = *ira;
	}
	return 0;
}
