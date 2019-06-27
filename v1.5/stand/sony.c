/*
 * Sony driver
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
#include "sys/pport.h"
#include "sys/sony.h"
#include "sys/cops.h"

char initflg;

snopen(ip)
register struct iob *ip;
{
	register unit = ip->i_unit;

	if (unit >= NSN) {
		printf("Invalid device 0x%x\n", unit);
		return(-1);
	}
	if (initflg == 0) {
		if (sninit() < 0)
			return(-1);
		initflg++;
	}
	if (snwaitrdy()) {
		printf("snopen: wait to check disk insertion failed\n");
		return(-1);
	}
	if (SNIOB->diskin != 0xff) {
		printf("snopen: no diskette inserted\n");
		return(-1);
	}
	return(0);
}

sninit()
{
	char c = 0x00;			/* to avoid clear byte intstructions */
	register i;

	if(!SNIOB->type) {
		printf("sninit: not a sony drive\n");
		return(-1);
	}
	SNIOB->drive=0x80;		/* always lower drive */
	SNIOB->side = c;		/* always first side */
	SNIOB->mask= 0xff;		/* clear ints and enable */
	if(snwaitrdy()) {
		printf("sninit: wait to check drive connected failed\n");
		return(-1);
	}
	if((i=SNIOB->drv_connect) != 0xff) {
		printf("sninit: no drive connected (0x%x)\n", i);
		return(-1);
	}
	if(snwaitrdy()) {
		printf("sninit: wait to clear status failed\n");
		return(-1);
	}
	SNIOB->gobyte=SN_CLRST;
	if(snwaitrdy()) {
		printf("sninit: command to clear status failed\n");
		return(-1);
	}
	SNIOB->mask =0x80;		/* enable interrupts */
	if(snwaitrdy()) {
		printf("sninit: wait to enable interrupts failed\n");
		return(-1);
	}
	SNIOB->gobyte=SN_STMASK;
	return(0);
}

snstrategy(ip, func)
register struct iob *ip;
{
	register unit = ip->i_unit;
	register daddr_t bn;
	register resid;
	int offset;
	caddr_t addr;

	if (unit >= NSN)
		return(-1);
	if (ip->i_bn < 0) {
		printf("snstrategy: bn < 0\n");
		return(-1);
	}
	resid = ip->i_cc;
loop:
	if (resid < PBSIZE)
		goto out;
	offset = ip->i_cc - resid;
	addr = ip->i_ma + offset;
	bn = ip->i_bn + (offset >> PBSHIFT);
	if (bn >= SN_MAXBN)
		goto out;
	if ((bn == 0) && (ip->i_bblk == BOOTDISK))
		func |= F_BOOT;
	if (snrw(unit, bn, func, addr) != 0)
		return(-1);
	resid -= PBSIZE;
	goto loop;

out:
	if (resid != 0)
		printf("snstrategy: bn=%d resid=%d\n", ip->i_bn, resid);
	return(ip->i_cc - resid);
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
snrw(unit, bn, rw, addr)
int unit, rw;
register daddr_t bn;
register caddr_t addr;
{
	register char *pm;
	register struct sn_blockmap *p = sn_blockmap;
	register short i;
	int track, sector;
	char c = 0x00;
	struct sn_hdr sn_hdr;
	register caddr_t haddr;
	register orig_bn;
#ifdef DEBUG
char getsbuf[10];
#endif DEBUG

	orig_bn = bn;
	if(snwaitrdy())
		return(-1);
	SNIOB->side=c;
	SNIOB->drive=0x80;
	track = 0;
	while( bn >= p->maxblock ) {
		bn -= p->maxblock;
		track += 16;
		p++;
	}
	sector = bn % p->sectors;
	track += bn / p->sectors;
	SNIOB->track = track; 
	SNIOB->sector= sector;
	if ((rw&~F_BOOT) == READ) {
#ifdef DEBUG
printf("r%d ", orig_bn);
#endif DEBUG
		SNIOB->cmd=SN_READ;
	} else {
#ifdef DEBUG
printf("w%d ", orig_bn);
if ((sector==0) && (track==0))
printf("writing actual block 0\n");
#endif DEBUG
		SNIOB->cmd=SN_WRITE;
		if (orig_bn == 0) {
			if (rw & F_BOOT)
				sn_hdr.fileid = FILEID;
			else
				sn_hdr.fileid = 0;
#ifdef DEBUG
printf("snrw: file id = 0x%x\n", sn_hdr.fileid);
printf("sector=%d, track=%d\n", sector, track);
gets(getsbuf);
#endif DEBUG
			sn_hdr.version = 0;
			sn_hdr.volume = 0;
			sn_hdr.relpg = 0;
			sn_hdr.dum1 = 0;
			sn_hdr.dum2 = 0;
			i = sizeof(sn_hdr) - 1;
			pm = (char *)SN_HDRBUF;	/* write special 12-byte hdr */
			haddr = (char *)&sn_hdr;
			do {
				*pm++;		/* cmos ram, every other byte */
				*pm++ = *haddr++;
			} while (--i != -1);
		}
		i = 511;
		pm = (char *)SN_DATABUF;	/* write reg 512-byte buffer */
		do {
			*pm++;			/* cmos ram, every other byte */
			*pm++ = *addr++;
		} while (--i != -1);
	}
	SNIOB->gobyte = SN_CMD;
	while ((COPSADDR->e_irb & FDIR) == 0)	;
	if( i = SNIOB->status) {
		printf("SNIOB->status = 0x%x\n",i);
		return(-1);
	}
	if ((rw&~F_BOOT) == READ) {
#ifdef DEBUG
if ((sector==0) && (track==0))
printf("actual block 0 read in\n");
#endif DEBUG
		i = 511;
		pm = (char *)SN_DATABUF;	/* write reg 512-byte buffer */
		do {
			*pm++;			/* cmos ram, every other byte */
			*addr++ = *pm++;
#ifdef DEBUG
if ((sector==0) && (track==0))
printf("0x%x ", *(addr-1));
#endif DEBUG
		} while (--i != -1);
	}
	SNIOB->mask = SN_CLEARMSK;
	SNIOB->gobyte = SN_CLRST;
	return (0);
}

snwaitrdy()
{
	register int i, j, k;

	/* Don't give up - may take a while before disk insertion check
	 * (in open) can be made.
	 */
	k = 0xC0000;
	do {
		i = 1000;
		while (((PPADDR)->d_irb & DSKDIAG) == 0) {	/* busy */
			for(j=0;j<1024;j++);		/* don't access flpy */
			if (--i < 0) {
				printf("snwaitrdy: DSKDIAG not ready\n");
				return(1);
			}
		}
		if (SNIOB->gobyte == 0)
			return(0);
	} while (--k);
	printf("snwaitrdy: drive not ready\n");
	return (1);
}

sneject(ip)
register struct iob *ip;
{
	register unit = ip->i_unit;
	register short i;

	if (unit >= NSN) {
		printf("Invalid device 0x%x\n", unit);
		return(-1);
	}
	if (initflg == 0) {
		if (sninit() < 0)
			return(-1);
		initflg++;
	}
	if (snwaitrdy()) {
		printf("sneject: wait to check disk insertion failed\n");
		return(-1);
	}
	if (SNIOB->diskin != 0xff) {
		printf("sneject: no diskette inserted\n");
		return(-1);
	}
	SNIOB->cmd=SN_EJECT;
	SNIOB->gobyte = SN_CMD;
	while ((COPSADDR->e_irb & FDIR) == 0)	;
	if( i = SNIOB->status) {
		printf("SNIOB->status = 0x%x\n",i);
		return(-1);
	}
	SNIOB->mask = SN_CLEARMSK;
	SNIOB->gobyte = SN_CLRST;
	return (0);
}

eject(unit)
register unit;
{
	struct iob ioblk;

	ioblk.i_unit = unit;
	if (sneject(&ioblk))
		return(-1);
	return(0);
}
