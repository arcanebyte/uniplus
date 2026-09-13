/*
 *	RK06/07 disk driver for standalone
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "../saio.h"


struct device
{
	int	hkcs1;
	int	hkwc;
	caddr_t	hkba;
	int	hkda;
	int	hkcs2;
	int	hkds;
	int	hkerr;
	int	hkas;
	int	hkdc;
	int	dum;
	int	hkdb;
	int	hkmr1;
	int	hkec1;
	int	hkec2;
	int	hkmr2;
	int	hkmr3;
};

#define	HKADDR	((struct device *)0177440)
#define	NHK	8
#define	NSECT	22
#define	NTRAC	3

#define P400	020
#define M400	0220
#define P800	040
#define M800	0240
#define P1200	060
#define M1200	0260


#define GO	01
#define SELECT	01
#define PAKACK	02
#define DCLR	04
#define RECAL	012
#define OFFSET	014
#define RCOM	020
#define WCOM	022
#define IEI	0100
#define CRDY	0200
#define CCLR	0100000
#define CERR	0100000

#define DLT	0100000
#define UPE	020000
#define NED	010000
#define PGE	02000
#define SCLR	040

#define SVAL	0100000
#define CDA	040000
#define PIP	020000
#define WLE	010000
#define DDT	0400
#define DRDY	0200
#define VV	0100
#define DOFST	04
#define DRA	01

#define DCK	0100000
#define DUNS	040000
#define OPI	020000
#define DTE	010000
#define DWLE	04000
#define ECH	0100
#define SKI	02
#define DRTER 040

int hk_drvtyp[NHK];
char hk_mntflg[NHK];


hkstrategy(io, func)
register struct iob *io;
{
	register unit,com;
	register i;
	daddr_t bn;
	int sn, cn, tn;

	unit = io->i_unit;
	if(hk_mntflg[unit] != '1')
	{
		hk_drvtyp[unit] = 0;
		HKADDR->hkcs2 = unit;
		HKADDR->hkcs1 = SELECT;
		while((HKADDR->hkcs1 & CRDY) == 0);
		if(HKADDR->hkcs1 & CERR && HKADDR->hkerr & DRTER)
		{
			hk_drvtyp[unit] = 02000;
		}
		hk_mntflg[unit] = '1';
	}
	bn = io->i_bn;
	HKADDR->hkcs2 = SCLR;
	while((HKADDR->hkcs1 & CRDY) == 0);
	HKADDR->hkcs2 = unit;
	HKADDR->hkcs1 = hk_drvtyp[unit]|SELECT;
	while((HKADDR->hkcs1 & CRDY) == 0);

	if((HKADDR->hkds & VV) == 0){
		HKADDR->hkcs1 = hk_drvtyp[unit]|PAKACK|GO;
		while((HKADDR->hkcs1 & CRDY) == 0);
	}
	cn = bn/(NSECT*NTRAC);
	sn = bn%(NSECT*NTRAC);
	tn = sn/NSECT;
	sn = sn%NSECT;

	HKADDR->hkdc = cn;
	HKADDR->hkda = (tn<<8) | sn;
	HKADDR->hkba = io->i_ma;
	HKADDR->hkwc = -(io->i_cc>>1);
	com = hk_drvtyp[unit]|(segflag << 8) | GO;
	if (func == READ)
		com |= RCOM;
	else if (func == WRITE)
		com |= WCOM;
	HKADDR->hkcs1 = com;

	while((HKADDR->hkcs1 & CRDY) == 0);

	if (HKADDR->hkcs1 & CERR){
		printf("disk error: cyl=%d track=%d sect=%d cs2=%d err=%o\n",
			cn, tn, sn, HKADDR->hkcs2, HKADDR->hkerr);
		return(-1);
	}
	return(io->i_cc);
}
