/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Driver for Centronix Citoh Dot Matrix Printer
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/reg.h"
#include "setjmp.h"
#include "sys/mmu.h"

#include "sys/cops.h"
#include "sys/pport.h"

extern struct device_d *pro_da[];

#define	LPPRI	(PZERO+8)
#define	LPLOWAT	40
#define	LPHIWAT	100
#define	LPMAX	NPPDEVS

struct lp {
	struct	clist l_outq;
	char	flag, ind;
	int	ccc, mcc, mlc;
	int	line, col;
	int	dev;
} lp_dt[LPMAX];

/*
 *	flag values - PORT, CAP and NOCR bits from minor device
 */
#define	PORT	0x0F		/* which port - 1,2,4,5,7,8 */
#define	CAP	0x10
#define	NOCR	0x20
#define	ASLP	0x40		/* only set within driver */
#define	OPEN	0x80		/* only set within driver */

#define	physical(d)	((minor(d)) & PORT)

#define	FORM	014
#define NO_OP asm("	nop ")
char lpflg[LPMAX];		/* whether we expect another interrupt */

/* ARGSUSED */
lpopen(dev, mode)
register dev_t dev;
{
	register unit;
	register struct lp *lp;
	register unsigned char zero;
	register struct device_d *p;
	int lpintr();
	extern char slot[];

	unit = physical(dev);
	SPL5();
	/* check expansion slot number and type (must be 2-port card) */
	if (!PPOK(unit) || (slot[PPSLOT(unit)] != PR0)) {
	err:
		u.u_error = EIO;
	fini:
		SPL0();
		return;
	}
	if ((lp = &lp_dt[unit])->flag) {
		goto err;
	}
	if (setppint(pro_da[unit],lpintr)) {	/* port is already busy */
		u.u_error = ENODEV;
		goto fini;
	}
	p = pro_da[unit];
	zero = 0;
	p->d_acr = 0;	/* no output latching */
	NO_OP;
	p->d_pcr = 0x6B;	/* set controller CA2 pulse mode strobe */
	NO_OP;
	p->d_ddra = -1;	/* set port A bits to output */
	NO_OP;
/*if (p == PPADDR)	from system III
p->d_ddrb &= 0x5C;	set BSY and OCD to input
else */
	p->d_ddrb &= 0xDC;		/* two or four port cards */
	NO_OP;
	p->d_ddrb |= 0x9C;		/* set port B bits 2,3,4,7 to out */
	NO_OP;
	p->d_irb &= ~(DEN|DRW);		/* disable buffers */
	NO_OP;
	p->d_irb |= WCNT;		/* set direction to output */
	NO_OP;
	p->d_ier = FIRQ|FCA1;		/* enable interrupt on busy */
	NO_OP;
	zero = p->d_irb;

	if (zero & OCD) {		/* out of paper ?? */
		printf("lpopen: cable disconnect or out of paper\n");
	out:
		freeppin(p);
		goto err;
	}

	if ((zero & PCHK) == 0) {		/* online ?? */
		printf("lpopen: (ddrb = %x) offline\n",zero);
		goto out;
	}
	lp->flag = (dev & (PORT | CAP | NOCR)) | OPEN;
	lp->ind = 4;
	lp->col = 80;
	lp->line = 66;
	lp->dev = dev;
	lpoutput(lp, FORM);
	SPL0();
}

lpclose(dev)
register dev_t dev;
{
	register unit;
	register struct lp *lp;

	unit = physical(dev);
	lp = &lp_dt[unit];
	lpoutput(lp, FORM);
	SPL5();
	while (lpflg[unit]) {
		lp->flag |= ASLP;
		(void) sleep((caddr_t)lp, LPPRI);
	}
	freeppin(pro_da[unit]);
	lp->flag = 0;
	SPL0();
}

lpwrite(dev)
register dev_t dev;
{
	register unit;
	register c;
	register struct lp *lp;

	unit = physical(dev);
	lp = &lp_dt[unit];
	while (u.u_count) {
		SPL5();
		while(lp->l_outq.c_cc > LPHIWAT) {
			lpintr(unit);
			lp->flag |= ASLP;
			(void) sleep((caddr_t)lp, LPPRI);
		}
		SPL0();
		c = fubyte(u.u_base++);
		if (c < 0) {
			u.u_error = EFAULT;
			break;
		}
		u.u_count--;
		lpoutput(lp, c);
	}
	SPL5();
	lpintr(unit);
	SPL0();
}

lpoutput(lp, c)
register struct lp *lp;
register c;
{
	if(lp->flag&CAP) {
		if(c>='a' && c<='z')
			c += 'A'-'a'; else
		switch(c) {
		case '{':
			c = '(';
			goto esc;
		case '}':
			c = ')';
			goto esc;
		case '`':
			c = '\'';
			goto esc;
		case '|':
			c = '!';
			goto esc;
		case '~':
			c = '^';
		esc:
			lpoutput(lp, c);
			lp->ccc--;
			c = '-';
		}
	}
	switch(c) {
	case '\t':
		lp->ccc = ((lp->ccc+8-lp->ind) & ~7) + lp->ind;
		return;
	case '\n':
		lp->mlc++;
		if(lp->mlc >= lp->line )
			c = FORM;
	case FORM:
		lp->mcc = 0;
		if (lp->mlc) {
			(void) putc(c, &lp->l_outq);
			if(c == FORM)
				lp->mlc = 0;
		}
	case '\r':
		lp->ccc = lp->ind;
		SPL5();
		lpintr(lp->dev);
		SPL0();
		return;
	case 010:
		if(lp->ccc > lp->ind)
			lp->ccc--;
		return;
	case ' ':
		lp->ccc++;
		return;
	default:
		if(lp->ccc < lp->mcc) {
			if (lp->flag&NOCR) {
				lp->ccc++;
				return;
			}
			(void) putc('\r', &lp->l_outq);
			lp->mcc = 0;
		}
		if(lp->ccc < lp->col) {
			while(lp->ccc > lp->mcc) {
				(void) putc(' ', &lp->l_outq);
				lp->mcc++;
			}
			(void) putc(c, &lp->l_outq);
			lp->mcc++;
		}
		lp->ccc++;
	}
}

lpintr(dev)
register dev;
{
	register struct lp *lp;
	register struct device_d *p;
	register c;

	dev = physical(dev);
	if (lpflg[dev]) return;
	lp = &lp_dt[dev];
	p = pro_da[dev];
	while ((c = getc(&lp->l_outq)) >= 0) {
		lpflg[dev] = 1;
		p->d_ira = c;
		c = 30;
		while ((p->d_ifr & FCA1) == 0) {
			if (--c <= 0)
				return;
		}
		lpflg[dev] = 0;
	}

	if (lp->l_outq.c_cc <= LPLOWAT && lp->flag&ASLP) {
		lp->flag &= ~ASLP;
		wakeup((caddr_t)lp);
	}
}

/* ARGSUSED */
lpioctl(dev, cmd, arg, mode)
{
}
