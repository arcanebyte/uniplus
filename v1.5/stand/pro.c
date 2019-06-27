/*
 * Standalone profile/widget driver
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
#include "saio.h"
#include "sys/pport.h"
#include "sys/d_profile.h"
#include "sys/profile.h"
#include "sys/cops.h"

#define min(a,b) ((a)<(b)?(a):(b))

/* Bytes to disk blocks */
#define	btod(x)	(((x)+(BSIZE-1))>>BSHIFT)

#define MAXREAD 32768

struct device_d *pro_da[NPPDEVS] = {		/* DEV	Description */
	PPADDR,					/* 0x00	parallel port */
	(struct device_d *)(STDIO+0x2000),	/* 0x10	FPC port 0 slot 1 */
	(struct device_d *)(STDIO+0x2800),	/* 0x20	FPC port 1 slot 1 */
	(struct device_d *)(STDIO+0x3000),	/* 0x30	FPC port 2 slot 1 */
	(struct device_d *)(STDIO+0x6000),	/* 0x40	FPC port 0 slot 2 */
	(struct device_d *)(STDIO+0x6800),	/* 0x50	FPC port 1 slot 2 */
	(struct device_d *)(STDIO+0x7000),	/* 0x60	FPC port 2 slot 2 */
	(struct device_d *)(STDIO+0xA000), 	/* 0x70	FPC port 0 slot 3 */
	(struct device_d *)(STDIO+0xA800),	/* 0x80	FPC port 1 slot 3 */
	(struct device_d *)(STDIO+0xB000)	/* 0x90	FPC port 2 slot 3 */
};

struct proheader ph;

proopen(io)
	register struct iob *io;
{
	if (io->i_unit < 0 || io->i_unit > NPPDEVS) {
		printf("proopen(%d): invalid unit\n",io->i_unit);
		return -1;
	}
	return proinit(pro_da[io->i_unit]);		/* initialize drive */

}

prostrategy(ip, func)
	register struct iob *ip;
{
	register i, cc;
	register daddr_t bn;
	caddr_t addr;
	int unit;

	addr = ip->i_ma;
	cc = ip->i_cc;
	bn = ip->i_bn;
	unit = ip->i_unit;
	if (unit < 0 || unit > NPPDEVS) {
		printf("prostrategy: i_unit=%d i_bn=%d i_cc=%d invalid unit\n",
			unit, bn, cc);
		return -1;
	}
		
	if (bn < 0 || bn + btod(cc) > MAXBLOCK) {
		printf("prostrategy: i_bn=%d i_cc=%d, bad blkno\n",
			bn, cc);
		return -1;
	}
	while (cc > 0) {
		if (cc > MAXREAD)
			i = MAXREAD;
		else
			i = cc;
		if (prorw(func, unit, addr, i, bn) != 0)
			goto out;
		cc -= i;
		addr += i;
		bn += btod(i);
	}
out:
	if (cc != 0)
		printf("prostrategy: bn=%d cc=%d resid=%d\n",
			ip->i_bn, ip->i_cc, cc);
	return ip->i_cc - cc;
}

proinit(devp)
	register struct device_d *devp;
{
	devp->d_pcr = 0x6B;	/* set controller CA2 pulse mode strobe */
	if (devp->d_pcr != 0x6B)
		return -1;
	devp->d_ddra = 0;	/* set port A bits to input **/
	devp->d_ddrb = 0x1C;
	devp->d_irb &= ~DEN;	/* set enable = true */
	devp->d_irb |= CMD|DRW;	/* set direction = out, command = false */
	devp->d_ier = 0;
	devp->d_acr = 0;
	return 0;
}

prorw(func, pun, addr, cnt, bn)
	caddr_t addr;
	daddr_t bn;
{
	register struct device_d *devp;
	register int i;
	register char zero;
	int state;
	register char *cp;
	struct cmd pc;
	int status;
	int retrys = 5;

	devp = pro_da[pun];
		/* Build Command (ok to send extra bytes on write cmd) */
	pc.p_cmd = (func&F_READ) ? PROREAD : PROWRITE;
	pc.p_high = (bn >> 16) & 0xFF;
	pc.p_mid = (bn >> 8) & 0xFF;
	pc.p_low = bn & 0xFF;
	pc.p_retry = 10;
	pc.p_thold = 3;
retry:
	while (((i=devp->d_irb)&BSY)!=BSY) ;	/* wait for controller */
			/* sync controller to cmd state */

	if (prochk(devp,SCMD) && prochk(devp,SCMD)) {
		printf("cant force disk into CMD state\n");
		return -1;
	}

	cp = (char *)(&pc);		/* send command */
	devp->d_ira = *cp++;
	devp->d_ira = *cp++;
	devp->d_ira = *cp++;
	devp->d_ira = *cp++;
	devp->d_ira = *cp++;
	devp->d_ira = *cp;

	state = (func&F_READ) ?SRDBLK :SWRTD;
	if (prochk(devp,state)) {
		printf("disk operation failed\n");
		return -1;
	}

	if (state == SRDBLK) {
fini:
		i = 0;
		devp->d_irb |= DRW;		/* set to read */
		devp->d_ddra = i;

		status = 0;
		cp = (char *)(&status);

		*cp++ = devp->d_ira;		/* get status of command */
		*cp++ = devp->d_ira;
		*cp++ = devp->d_ira;
		*cp = devp->d_ira;

		if (status & (GOAHEAD | FAIL | RESETP)) {
			printf("dev %d: status=x%x\n",pun,status);
			if (retrys-- > 0) goto retry;
			return -1;
		}

		if (state == SRDBLK) {	/* Read successful so pickup data*/
			i = HDRSIZE - 1;		/* sizeof header */
			do zero = devp->d_ira;
			while (--i != -1);	/* optimizes to DBRA */
			cp = addr;
			i = min(SECSIZE,cnt) - 1;
			do *cp++ = devp->d_ira;
			while (--i != -1);
		}
		return 0;
	}

	if (state == SWRTD) {		/* send data */
		state = SFINI;
		if (bn)
			ph.ph_fileid = 0;
		else
			ph.ph_fileid = 0xAAAA;
		
		devp->d_irb &= ~DRW;	/* set dir=out */
		devp->d_ddra = 0xFF;	/* set port A bits to output */

		i = HDRSIZE - 1;
		cp = (char *)(&ph);
		do devp->d_ira = *cp++;
		while (--i != -1);
		i = min(SECSIZE,cnt) - 1;
		cp = (char *)addr;	/* place to get data from */
		do devp->d_ira = *cp++;
		while (--i != -1);
		if (prochk(devp,SPERFORM)) {
			printf("dev %d blk %d: write op failed\n",pun,bn);
			return -1;
		}
		goto fini;
	}
	printf ("LOGIC ERR IN DISK DRIVER\n");
	return -1;
}

/*
 * Get in sync with disk.
 * (subroutine FINDD2 & CHKRSP from 'profrom.text' document)
 * Expects enable==true and Direction==in at start.
 * If disk response is 'state' then returns 0 (success)
 * otherwise fails (returns -1 if timeout and cur state if bad state).
 */
prochk(devp, state)
	register struct device_d *devp;
{
	register zero = 0;
	register i;
	int resp;

	if (devp->d_irb & OCD) {		/* cable disconnected ? */
		printf("unit at x%x doesn't appear to be plugged in\n",devp);
		return -1;
	}

	while ((devp->d_irb&BSY)==0) ;

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
		else
			printf("> prochk: state %d instead of %d\n",i,state);
		devp->d_irb &= ~(DRW|CMD);	/* set dir=out cmd=true */
		devp->d_ddra = 0xFF;		/* set port A bits to output */
		devp->d_ira = resp;		/* send reply (GO or RESET) */
		devp->d_ier = FCA1;
		devp->d_irb |= CMD;		/* sig disk to read resp */
		while (!(devp->d_irb&BSY)) ;	/* wait, action is performed */
		return (i == state) ?0 :i;
	}

	printf("EXCESSIVE DISK DELAY -- CHECK HARDWARE");
	devp->d_ddra = zero;		/* set port A bits to input */
	devp->d_irb |= CMD|DRW;		/* set dir=in, disable buffers */
	return (-1);
}
