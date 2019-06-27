/*
 * Priam Datatower Disk and Streaming Tape Driver
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
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
#include "streamer.h"
#include "sys/cops.h"
#include "sys/local.h"
#include "sys/priam.h"

/*#define DELAY()	asm("\tnop");*/
#define DELAY()	;

char pmiflag[NPM];
char pmresults[NPMRES];		/* result regs. for last command comp ack */
int pmnblks[NPM];		/* number of blocks on disk */
int pmcblkcnt;			/* current block count */
struct pmheader pmhdr;		/* write header (write boot fileid for blk 0) */

/* variables needed to share info for interrupt routines */
struct pm_base *pmhwbase;
struct datacopyparms dcparms;
struct pmfmtstat dcstat;
struct results reslt;
char	pmstate;		/* idling or waiting for interrupt */
char	pmxstate;		/* idling or waiting for interrupt */
char	pmunit;			/* card (0,1,2) */
daddr_t	pmbn;			/* bn for pmwdsect, block 0 gets special ID in header */
caddr_t	pmma;			/* memory address */
char	pmierror;		/* error value from intr */

pmopen(iobp)
register struct iob *iobp;
{
	register unit = iobp->i_unit;
	register short slotid;
	int pm0intr(), pm1intr(), pm2intr();
	register long *ip;
	register short *waddr, i;

	if (unit >= NPM) {
		printf("Invalid device 0x%x\n", unit);
		return -1;
	}
	slotid = (unit==0) ? *SLOTIDS : 
		((unit==1) ? *SLOTID2 : *SLOTID3);
	if ((slotid & SLOTMASK) != ID_PRIAM) {
		printf("pmopen: not a Priam card\n");
		return -1;
	}
	ip = &((long *) 0)[EXPIVECT+devtoslot(unit)];	/* point to int vect */
	*ip = (unit==0) ? (long)pm0intr :	/* set to Priam intr */
		((unit==1) ? (long)pm1intr : (long)pm2intr);
	waddr = (short *)&pmhdr;	/* clear header to zeroes */
	i = HDRSIZE / sizeof(short);
	do {
		*waddr++ = 0;
	} while ( --i );
	if (pmiflag[unit] == 0) {
		if (pmcinit(unit))
			return(-1);
		if (pmdinit(unit))
			return(-1);
		pmiflag[unit]++;
	}
	return 0;
}

/*
 * pmcinit - initialize controller (turn on motor)
 * from PRIAMCARDASM.TEXT - INITCONT
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
err:	printf("can't initialize controller\n");
	return(-1);
}

/*
 * pmdinit - initialize device (sequence up)
 * from PRIAMASM.TEXT - INITDEV
 */
pmdinit(unit)
register unit;
{
	register struct pm_base *addr;
	register tmp;
	register pmnheads, pmncyls, pmnsecs;	/* no. of heads, cylinders, sectors */

	pmhwbase = pmaddr(unit);	/* base address for this card */
	addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if (pmwaitrf(addr)) {
		printf("timeout before read dev parms\n");
		goto err;
	}
/*****/	addr->p0 = 0;			/* device always 0 */
	addr = pmBIaddr(pmhwbase);	/* byte mode - intr enabled */
	pmunit = unit;			/* save for pmiintr */
	pmierror = 0;			/* clear for pmiintr */
	pmstate = INITING;
	tmp = spl2();
	addr->cmdreg = PMRDEVPMS;	/* sequence up disk & read dev parms */
	while (pmstate != IDLING)	;
	splx(tmp);
	if (pmierror) {
		printf("transaction status = 0x%x\n", pmresults[0]);
		goto err;
	}
	pmnheads = PMNH(pmresults[1]);
	pmncyls = PMNC(pmresults[1], pmresults[2]);
	pmnsecs = PMNS(pmresults[3]);
	pmnblks[unit] = pmnheads * pmncyls * pmnsecs;

	return 0;
err:	printf("can't initialize disk\n");
	return(-1);
}

/*
 * disk functions
 */

pmdstrategy(ip, func)
register struct iob *ip;
{
	register unit = ip->i_unit;
	register daddr_t bn;
	register resid;
	int offset;
	caddr_t addr;

	if (unit >= NPM)
		return(-1);
	pmhwbase = pmaddr(unit);	/* base address for this card */
	if (ip->i_bn < 0) {
		printf("pmdstrategy: bn < 0\n");
		return(-1);
	}
	resid = ip->i_cc;
dloop:
	if (resid < PBSIZE)
		goto dout;
	offset = ip->i_cc - resid;
	addr = ip->i_ma + offset;
	bn = ip->i_bn + (offset >> PBSHIFT);
	if (bn >= pmnblks[unit]) {
		printf("pmdstrategy: %d blocks on disk\n", pmnblks[unit]);
		goto dout;
	}
	if (pmdrw(unit, bn, func&F_READ, addr) != 0)
		return(-1);
	resid -= PBSIZE;
	goto dloop;

dout:
	if (resid != 0)
		printf("pmdstrategy: bn=%d resid=%d\n", ip->i_bn, resid);
	return(ip->i_cc - resid);
}

pmdrw(unit, bn, rflag, addr)
register unit;
daddr_t bn;
register rflag;
register caddr_t addr;
{
	register struct pm_base *hwaddr;
	register int tmp;

	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if (pmwaitrf(hwaddr)) {
		printf("pmdrw: timeout before setting %s parameters ",
			rflag?"read":"write");
		goto derr;
	}
/*****/	hwaddr->p0 = DISK;		/* for disk operations */
	tmp = bn;
	hwaddr->p3 = tmp & 0xFF;	/* most sig. byte */
	tmp = tmp >> 8;
	hwaddr->p2 = tmp & 0xFF;	/* middle byte */
	tmp = tmp >> 8;
	hwaddr->p1 = tmp & 0xFF;	/* least sig. byte */
	hwaddr->p4 = 1;			/* operation count = 1 sector */
	hwaddr->p5 = 0;
	pmcblkcnt = 1;
	hwaddr = pmBIaddr(pmhwbase);	/* byte mode - intr enabled */
	pmunit = unit;			/* save for pmrwintr */
	pmbn = bn;			/* pmwdsect - block 0 is special */
	pmma = addr;			/* save for pmrwintr */
	pmierror = 0;			/* clear for pmrwintr */
	pmstate = rflag ? DREADING : DWRITING;
	tmp = spl2();
	hwaddr->cmdreg = rflag ? PMREAD : PMWRITE;	/* start r/w */
	while (pmstate != IDLING)	;
	splx(tmp);
	if (pmierror) {
		if (pmierror & PMPERROR)
			printf("parity error\n");
		if (pmierror & 0x40)
			printf("previous error\n");
		if (tmp = (pmierror & ~PMPERROR))
			printf("error code 0x%x\n", tmp);
		printf("pmdrw: %s failed on ", rflag?"read":"write");
		goto derr;
	}
	return(0);

derr:	printf("card %d, bn %d, addr 0x%x\n", unit, bn, addr);
	return(-1);
}

/* interrupt on card 0 */
pm0intr()
{
	if ((pmunit != 0) || (pmstate == IDLING)) {
		register struct pm_base *addr;

		printf("unexpected interrupt, unit 0, pmunit %d\n", pmunit);
		addr = pmBWaddr(pmaddr(0));
		addr->cmdreg = 0;	/* clear spurious intr (clrb) */
	} else if (pmstate == INITING)	/* initialization */
		pmiintr();
	else if (pmstate == TCONT)	/* tape control */
		pmtintr();
	else if (pmstate == PMPKTXFER)
		pmxintr();
	else	/* pmstate is READING, WRITING, RFLUSH or WFLUSH */
		pmrwintr();
	asm("unlk a6");
	asm("rte");
}

/* interrupt on card 1 */
pm1intr()
{
	if ((pmunit != 1) || (pmstate == IDLING)) {
		register struct pm_base *addr;

		printf("unexpected interrupt, unit 1, pmunit %d\n", pmunit);
		addr = pmBWaddr(pmaddr(1));
		addr->cmdreg = 0;	/* clear spurious intr (clrb) */
	} else if (pmstate == INITING)	/* initialization */
		pmiintr();
	else if (pmstate == TCONT)	/* tape control */
		pmtintr();
	else if (pmstate == PMPKTXFER)
		pmxintr();
	else	/* pmstate is READING, WRITING, RFLUSH or WFLUSH */
		pmrwintr();
	asm("unlk a6");
	asm("rte");
}

/* interrupt on card 2 */
pm2intr()
{
	if ((pmunit != 2) || (pmstate == IDLING)) {
		register struct pm_base *addr;

		printf("unexpected interrupt, unit 2, pmunit %d\n", pmunit);
		addr = pmBWaddr(pmaddr(2));
		addr->cmdreg = 0;	/* clear spurious intr (clrb) */
	} else if (pmstate == INITING)	/* initialization */
		pmiintr();
	else if (pmstate == TCONT)	/* tape control */
		pmtintr();
	else if (pmstate == PMPKTXFER)
		pmxintr();
	else	/* pmstate is READING, WRITING, RFLUSH or WFLUSH */
		pmrwintr();
	asm("unlk a6");
	asm("rte");
}

/*
 * interrupt on read/write operation
 */
pmrwintr()
{
	register struct pm_base *hwaddr;
	register status;

	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if ((hwaddr->status & CMD_DONE) == 0) {	/* block transfer interrupt */
		if (!pmcblkcnt)		/* 2nd interrupt */
			goto done;
		/* 1st interrupt */
		pmcblkcnt--;
		switch (pmstate) {
			case DREADING:
				pmrdsect();	/* sets pmierror for parity */
				break;
			case DWRITING:
				pmwdsect();
				break;
			case TREADING:
				pmrtsect();
				break;
			case TWRITING:
				pmwtsect();
				break;
		}
		hwaddr->cmdreg = PMCBTI;	/* clear block transfer intr */
		hwaddr = pmBWIaddr(pmhwbase);	/* setup for next intr */
		status = hwaddr->status;
	} else {		/* cmd completed (error) */
		pmierror |= 0x40;
	done:
#ifdef notdef
		if (pmierror) {
			if (pmstate == READING)
				pmrflush();
			else
				pmwflush();
			hwaddr->cmdreg = PMCBTI;	/* clear block transfer intr */
			hwaddr = pmBWIaddr(pmhwbase);	/* setup for next intr */
			status = hwaddr->status;
		}
#endif notdef
		if (status = pmackcc(hwaddr)) {	/* ack intr */
			if ((pmstate==DREADING) && (pmresults[0] == PMECCERR))
				/* ECC err requiring scavenger write */
				status = PMECCERR;
			else if (pmresults[0] == PMDTIMOUT)
				/* dev timeout - they reissue read/write */
				status = PMDTIMOUT;
		}
		pmierror |= status;
		if (pmierror) {
			pmcinit(pmunit);
		}
		pmstate = IDLING;
	}
}

/*
 * interrupt on initialize (read drive parameters) operation
 */
pmiintr()
{
	register struct pm_base *addr;

	addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if (pmierror = pmackcc(addr)) {	/* ack intr */
		printf("pmiintr: read dev parms failed\n");
	}
	pmstate = IDLING;
}

/*
 * read block to memory at pmma from disk
 * from PRIAMASM.TEXT - READ_SECT
 */
pmrdsect()
{
	register struct pm_base *hwaddr;
	register short *waddr;
	register i;

	if ((long)pmma & 0x1) {		/* cannot handle odd memory addr */
		printf("odd addr=0x%x\n", pmma);
		pmierror = 1;
		pmstate = IDLING;
		return;
	}
	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmPaddr(pmhwbase);	/* parity checking enabled */
	waddr = (short *)pmma;
	*waddr = hwaddr->data;		/* start read of first word */
	i = 12;
	do {				/* read and throw away header */
		DELAY();
		*waddr = hwaddr->data;
	} while ( --i );		/* read 24-byte header */
	waddr = (short *)pmma;
	i = 255;
	do {
		DELAY();
		*waddr++ = hwaddr->data;
	} while ( --i );		/* read 510 bytes */
	hwaddr = pmNaddr(hwaddr);	/* read last 2 avoiding device cycle */
	*waddr = hwaddr->data;
	hwaddr = pmpaddr(pmunit);	/* parity in lo select space */
	pmierror |= (hwaddr->parity & PMPERROR);
}

/*
 * write block from memory at pmma from disk
 * from PRIAMASM.TEXT - WRITE_SECT
 */
pmwdsect()
{
	register struct pm_base *hwaddr;
	register short *waddr;
	register i;

	if ((long)pmma & 0x1) {		/* cannot handle odd memory addr */
		printf("odd addr=0x%x\n", pmma);
		pmierror = 1;
		pmstate = IDLING;
		return;
	}
	if (pmbn == 0)		/* boot block 0 gets boot ID in header */
		pmhdr.pm_fileid = FILEID;
	else
		pmhdr.pm_fileid = 0;
	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmhwbase;
	waddr = (short *)&pmhdr;
	i = HDRSIZE / sizeof(short);
	do {
		DELAY();
		hwaddr->data = *waddr++;
	} while ( --i );		/* write 24 byte header */
	waddr = (short *)pmma;
	i = 256;
	do {
		DELAY();
		hwaddr->data = *waddr++;
	} while ( --i );		/* write 512 bytes */
}

/*
 * after error flush controller with dummy read to complete request from disk
 * from PRIAMASM.TEXT - READ_FLUSH
 */
pmrflush()
{
	register struct pm_base *hwaddr;
	register short tmp;
	register i;

	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmPaddr(pmhwbase);	/* parity checking enabled */
	tmp = hwaddr->data;		/* start read of first word */
	i = 267;
	do {
		DELAY();
		tmp = hwaddr->data;
	} while ( --i );		/* read 534 bytes */
	hwaddr = pmNaddr(hwaddr);	/* read last 2 avoiding device cycle */
	tmp = hwaddr->data;
}

/*
 * after error flush controller to complete request from disk
 * from PRIAMASM.TEXT - WRITE_FLUSH
 */
pmwflush()
{
	register struct pm_base *hwaddr;
	register i;
	register short tmp = 0;

	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmhwbase;
	i = 268;
	do {
		DELAY();
		hwaddr->data = tmp;
	} while ( --i );		/* write 536 bytes */
}

/*
 * streaming tape functions
 */

char pmcmd[20];

pmtstrategy(ip, func)
register struct iob *ip;
{
	register unit = ip->i_unit;
	register resid;
	int offset;
	caddr_t addr;

	if (unit >= NPM)
		return(-1);
	pmhwbase = pmaddr(unit);	/* base address for this card */
	resid = ip->i_cc;
tloop:
	if (resid < PBSIZE)
		goto tout;
	offset = ip->i_cc - resid;
	addr = ip->i_ma + offset;
	if (pmtrw(unit, func&F_READ, addr) != 0)
		return(-1);
	resid -= PBSIZE;
	goto tloop;

tout:
	if (resid != 0)
		printf("pmtstrategy: resid=%d\n", resid);
	return(ip->i_cc - resid);
}

pmtrw(unit, rflag, addr)
register unit;
register rflag;
register caddr_t addr;
{
	register struct pm_base *hwaddr;
	register int tmp;

	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	if (pmwaitrf(hwaddr)) {
		printf("pmtrw: timeout before setting %s parameters ",
			rflag?"read":"write");
		goto terr;
	}
/*****/	hwaddr->p0 = (TAPE << 4)| unit;	/* for tape operations */
	hwaddr->p3 = 0;			/* most sig. byte */
	hwaddr->p2 = 0;			/* middle byte */
	hwaddr->p1 = 0;			/* least sig. byte */
	hwaddr->p4 = 1;			/* operation count = 1 sector */
	hwaddr->p5 = 0;
	pmcblkcnt = 1;
	hwaddr = pmBIaddr(pmhwbase);	/* byte mode - intr enabled */
	pmunit = unit;			/* save for pmrwintr */
	pmma = addr;			/* save for pmrwintr */
	pmierror = 0;			/* clear for pmrwintr */
	pmstate = rflag ? TREADING : TWRITING;
	tmp = spl2();
	hwaddr->cmdreg = rflag ? PMNREAD : PMNWRITE;	/* start r/w */
	while (pmstate != IDLING)	;
	splx(tmp);
	if ((pmierror = pmresults[0] & 0x3f)) {
		if (pmierror == 4)
			printf("pmtrw: READ failure due to EOF being encountered\n");
		else if (pmierror == 5)
			printf("pmtrw: WRITE error due to end of tape\n");
		else if (pmierror == 0x14) {
			switch (pmresults[1]) {
				case 0: case 1: printf("Unrecoverable data error ");
						break;
				case 2: printf("No recorded data error ");
					break;
				case 3: printf("Error in positioning tape deck ");
					break;
				case 4: printf("QIC status error ");
					break;
				case 5: printf("Tape Drive reset ");
					break;
				case 6: printf("Operation aborted ");
					break;
				case 7: printf("Command sequence error ");
					break;
				case 8: printf("No tape in the drive ");
					break;
				case 9:
				default: printf("Undefined command ");
			}
			printf("pmtrw: %s failed on ", rflag?"read":"write");
		}
		goto terr;
	}
	return(0);

terr:	printf("card %d, addr 0x%x\n", unit, addr);
	return(-1);
}

abortpkt()
{
	register struct pm_base *hwaddr;
	register int s;

	hwaddr = pmBWaddr(pmhwbase);		/* byte mode - waiting */
	if (pmwaitrf(hwaddr)) {
		printf("abortpkt: timeout before sending abort command\n");
		return;
	}
	hwaddr->p0 = 0;				/* packet #, ALWAYS 0 */
	hwaddr = pmBIaddr(pmhwbase);		/* byte mode - interrupt */
	pmstate = PMPKTXFER;
	pmxstate = ABORT;
	pmierror = 0;				/* for interrupt routine */
	s = spl2();
	hwaddr->cmdreg = PMPKTABRT;
	while (pmstate != IDLING)	;
	splx(s);
}

pmioctl(unit, cmd, parm)
int unit, cmd, parm;
{
	register struct pm_base *hwaddr;
	register int s;

	if (unit >= NPM)
		return(-1);
	pmhwbase = pmaddr(unit);		/* base address for this card */
	hwaddr = pmBWaddr(pmhwbase);		/* byte mode - waiting */
	if (pmwaitrf(hwaddr)) {
		printf("pmioctl: timeout before setting %s parameters ", pmcmd);
		goto err;
	}
	hwaddr->p0 = unit | (TAPE << 4);	/* unit # */
	if (cmd == PMADVANCE)			/* file mark count */
		hwaddr->p4 = parm;
	hwaddr = pmBIaddr(pmhwbase);		/* byte mode - interrupt */
	pmstate = TCONT;
	pmierror = 0;				/* for interrupt routine */
	s = spl2();
	hwaddr->cmdreg = cmd;
	while (pmstate != IDLING)	;
	splx(s);
	if ((pmierror = pmresults[0] & 0x3f)) {
		if (pmierror == 4)
			printf("pmtioctl: READ failure due to EOF being encountered\n");
		else if (pmierror == 5)
			printf("pmtioctl: WRITE error due to end of tape\n");
		else if (pmierror == 0x14) {
			printf("pmioctl: ");
			switch (pmresults[1]) {
				case 0: case 1: printf("Unrecoverable data error ");
						break;
				case 2: printf("No recorded data error ");
					break;
				case 3: printf("Error in positioning tape deck ");
					break;
				case 4: printf("QIC status error ");
					break;
				case 5: printf("Tape Drive reset ");
					break;
				case 6: printf("Operation aborted ");
					break;
				case 7: printf("Command sequence error ");
					break;
				case 8: printf("No tape in the drive ");
					break;
				case 9:
				default: printf("Undefined command ");
			}
		}
		goto err;
	}
	return(0);

err:	printf("card %d\n", unit);
	return(-1);
}

/*
 * interrupt on tape control operations
 */
pmtintr()
{
	register struct pm_base *addr;

	addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	(void)pmackcc(addr);
	pmstate = IDLING;
}

/*
 * Interrupt on packet operations
 */

pmxintr()
{
	register struct pm_base *addr;
	register char status;
	register cnt;

	addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	switch (pmxstate) {
		case ISSUE:		/* intr to transfer packet */
			cnt = TIMELIMIT;
			while ((addr->status & DTREQ) == 0 && --cnt) ;
			if(cnt <= 0) {
				pmxstate = RESUME;
				goto resume;
			}
			{		/* send packet */
				register char *cp = (char *)&dcparms;
				register i = sizeof(struct datacopyparms);
				do {
					addr->pdata = *cp++;
				} while ( --i );
			}
			(void)pmackdt(addr);	/* Apple's code doesn't check */
	wait:		pmxstate++;	/* not done yet - wait for next intr */
			return;
		case ABORT:		/* intr after abort command */
			 (void)pmackcc(addr);
			 if(pmresults[0] & 0x30) {
				reslt.errcode = -1;
				break;
			}
			pmxstate = RESUME;
			goto getstat;	/* get packet status block */
		case RESUME:		/* intr after packet complete */
	resume:		 (void)pmackcc(addr);
			if(pmresults[0] != 0x8 && pmresults[0] != 0x28) {
				if(pmresults[0] == 0x29 && pmresults[1] == 0x2F)
					printf("Resume timeout error, start again\n");
				reslt.errcode = -1;
				break;
			}
	getstat:
			(void)pmwaitrf(addr);
			addr->p0 = 0;	/* packet ID = 0 */
			addr = pmBIaddr(pmhwbase);
			addr->cmdreg = PMPKTRST;/* read packet status */
			goto wait;
		case RPSTAT:		/* intr when status can be read */
			if ((addr->status & DTREQ) == 0)
				goto ackcc;	/* if not waiting for data */
			{		/* read packet status */
				register char *cp = (char *)&dcstat;
				register i = sizeof(struct pmfmtstat);
				do {
					*cp++ = addr->pdata;
				} while ( --i );
			}
			(void)pmackdt(addr);	/* Apple's code doesn't check */
			goto wait;
		case RPSTAT2:		/* intr after read packet status */
	ackcc:		(void)pmackcc(addr);
			if(pmresults[0] & 0x30) {
				reslt.errcode = -1;
				break;
			}
			if((reslt.errcode = dcstat.pm_pstate) == 0x0D 
						|| reslt.errcode == 0x03)
				break;
			else reslt.errcode = -1;
		}
		pmstate = IDLING;
}

/*
 * read block to memory at pmma from tape
 * from ARCHIVEASM.TEXT - READ_SECT
 */
pmrtsect()
{
	register struct pm_base *hwaddr;
	register short *waddr;
	register i;

/*	if ((long)pmma & 0x1) {		/* cannot handle odd memory addr
		printf("odd addr=0x%x\n", pmma);
		pmierror = 1;
		pmstate = IDLING;
		return;
	}
*/	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmPaddr(pmhwbase);	/* parity checking enabled */
	waddr = (short *)pmma;
	*waddr = hwaddr->data;		/* start read of first word */
	i = 255;
	do {
		DELAY();
		*waddr++ = hwaddr->data;
	} while ( --i );		/* read 510 bytes */
	hwaddr = pmNaddr(hwaddr);	/* read last 2 avoiding device cycle */
	*waddr = hwaddr->data;
	hwaddr = pmpaddr(pmunit);	/* parity in lo select space */
	pmierror = pmresults[0];
}

/*
 * write block from memory at pmma from tape
 * from ARCHIVEASM.TEXT - WRITE_SECT
 */
pmwtsect()
{
	register struct pm_base *hwaddr;
	register short *waddr;
	register i;

/*	if ((long)pmma & 0x1) {		/* cannot handle odd memory addr
		printf("odd addr=0x%x\n", pmma);
		pmierror = 1;
		pmstate = IDLING;
		return;
	}
*/	hwaddr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
	while ((hwaddr->status & DTREQ) == 0)	;
	hwaddr = pmhwbase;
	waddr = (short *)pmma;
	i = 256;
	do {
		DELAY();
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
