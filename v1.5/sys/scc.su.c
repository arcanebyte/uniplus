/*
 * SCC device driver
 *
 *	Copyright 1982 UniSoft Corporation
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
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
#include <sys/scc.h>
#include "sys/proc.h"
#include "setjmp.h"

int scproc();
extern int sc_cnt;
extern struct tty sc_tty[];
extern struct ttyptr sc_ttptr[];
extern char sc_modem[];
extern struct scline sc_line[];

#define MODEM	0x80			/* modem control on bit */
#define scdev(d)	((d)&0x7f)	/* from unix device number to device */

#define DELAY()	asm("\tnop");


#define SCTIME (v.v_hz*5)		/* scscan interval */

/*
 * Note: to select baud rate
 *	k = chip_input_frequency/(2 * baud * factor) - 2
 *	put factor in register 9 and k in registers D & C
 *	NOTE:
 *		normally, factor = 16
 *		for this driver, chip_input_frequency = 2400000 Hz
 * scspeeds is a table of these numbers by UNIX baud rate
 */
int scspeeds[] = {
	1,	50,	75,	110,	134,	150,	200,	300,
	600,	1200,	1800,	2400,	4800,	9600,	19200,	38400,
};

/*
 * table to initialize a port to 9600 baud
 */
char scitable[] = {
	W0NULL,
	9, 0,			/* set according to sc_line reset value */
#define SCCIRESET	scitable[2]
	4, W4CLK16 | W41STOP,
	10, 0,
	11, W11RBR | W11TBR,
	12, 0,			/* 12/13 are baud rate, from sc_line speed value */
#define SCCISPLO	scitable[10]
	13, 0,
#define SCCISPHI	scitable[12]
	14, W14BRGE | W14BRINT,
	3, W38BIT | W3RXENABLE,
	5, W58BIT | W5TXENABLE,
	1, W1RXIALL | W1TXIEN /*| W1EXTIEN*/,
	2, 0,				/* auto vector */
	0, W0RXINT,
	15, 0,
	9, W9MIE | W9DLC,
};

/*
 * we call this in startup() to preinitialize all the ports
 */
scinit()
{
	register struct device *addr;
	unsigned short nsc;

	for(nsc=0;nsc<500;nsc++);
	for (nsc = 0; nsc < sc_cnt; nsc++) {
		addr = (struct device *)sc_ttptr[nsc].tt_addr;
		/* do proper reset */
		scini1(addr, W9NV|sc_line[nsc].reset, (int)sc_line[nsc].speed);
	}
	printf("%d serial ports\n", nsc);
	/* scscan();	/* start modem scanning */
}

scini1(addr, reset, speed)
struct device *addr;
int reset, speed;
{
	register int i;
	int s;

	s = spl6();
	SCCIRESET = reset;
	speed = (speed/(9600<<1)) - 2;
	SCCISPHI = (speed >> 8) & 0xFF;
	SCCISPLO = speed & 0xFF;
	for (i = 0; i < sizeof(scitable); i++) {
		DELAY();
		addr->csr = scitable[i];
	}
	DELAY();
	splx(s);
}

/* ARGSUSED */
scopen(dev, flag)
register dev;
{
	register struct tty *tp;
	register struct device *addr;
	register d;
#ifdef SINGLEUSER
	register struct proc *p;
#endif SINGLEUSER

	d = scdev(dev);
	if (d >= sc_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = sc_ttptr[d].tt_tty;
#ifdef SINGLEUSER
	p = u.u_procp;
	if ((p->p_pid == p->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0)) {
		u.u_error = ENOTTY;
		return;
	}
#endif SINGLEUSER
	tp->t_index = d;
	SPL6();
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		tp->t_proc = scproc;
		ttinit(tp);		
		sc_modem[d] = dev & MODEM;
		addr = (struct device *)sc_ttptr[d].tt_addr;
		if (dev & MODEM && (addr->csr & R0DCD) == 0)
			tp->t_state = WOPEN;
		else
			tp->t_state = WOPEN | CARR_ON;
#ifdef SCC_CONSOLE
		if (d == CONSOLE) {
			tp->t_iflag = ICRNL | ISTRIP;
			tp->t_oflag = OPOST | ONLCR | TAB3;
			tp->t_lflag = ISIG | ICANON | ECHO | ECHOK;
			tp->t_cflag = sspeed | CS8 | CREAD | HUPCL;
		}
#endif SCC_CONSOLE
		scparam(dev);
	}
	if (!(flag & FNDELAY))
		while ((tp->t_state&CARR_ON) == 0)
			(void) sleep((caddr_t)&tp->t_rawq, TTOPRI);
	SPL0();
	(*linesw[tp->t_line].l_open)(tp);
}

/* ARGSUSED */
scclose(dev, flag)
{
	register struct tty *tp;
	register int d;

	d = scdev(dev);
	tp = sc_ttptr[d].tt_tty;
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag & HUPCL)
		schup(dev);			/* hang up */
}

scread(dev)
{
	register struct tty *tp;

	tp = sc_ttptr[scdev(dev)].tt_tty;
	(*linesw[tp->t_line].l_read)(tp);
}

scwrite(dev)
{
	register struct tty *tp;

	tp= sc_ttptr[scdev(dev)].tt_tty;
	(*linesw[tp->t_line].l_write)(tp);
}

scproc(tp, cmd)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register struct device *addr;
	register dev_t dev;
	extern ttrstrt();
	int s;

	s = spl6();
	dev = tp->t_index;
	addr = (struct device *)sc_ttptr[dev].tt_addr;
	switch (cmd) {

	case T_TIME:
		scw5(tp, addr, 0);	/* clear break */
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:
		tbuf = &tp->t_tbuf;
		tbuf->c_size -= tbuf->c_count;
		tbuf->c_count = 0;
		/* fall through */
	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		goto start;

	case T_OUTPUT:
start:
		if (tp->t_state & (TTSTOP|TIMEOUT|BUSY))
			break;
		if (tp->t_state & TTXOFF) {
			tp->t_state &= ~TTXOFF;
			tp->t_state |= BUSY;
			addr->data = CSTOP;
			break;
		}
		if (tp->t_state & TTXON) {
			tp->t_state &= ~TTXON;
			tp->t_state |= BUSY;
			addr->data = CSTART;
			break;
		}
		tbuf = &tp->t_tbuf;
		if ((tbuf->c_ptr == 0) || (tbuf->c_count == 0)) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
				break;
		}
		tp->t_state |= BUSY;
		addr->data = *tbuf->c_ptr++;
		tbuf->c_count--;
		break;

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
		tp->t_state |= TTXOFF;
		goto start;

	case T_RFLUSH:
		if (!(tp->t_state&TBLOCK))
			break;
		/* fall through */

	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;
		goto start;

	case T_BREAK:
		scw5(tp, addr, W5BREAK);
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, (caddr_t)tp, v.v_hz>>2);
		break;
	}
	splx(s);
}

scw5(tp, addr, d)
struct tty *tp;
struct device *addr;
int d;
{
	register int w5;
	int s;

	w5 = W5TXENABLE | d;
	switch(tp->t_cflag & CSIZE) {
		case CS5:
			w5 |= W55BIT; break;
		case CS6:
			w5 |= W56BIT; break;
		case CS7:
			w5 |= W57BIT; break;
		case CS8:
			w5 |= W58BIT; break;
	}
	s = spl5();
	addr->csr = 5;
	addr->csr = w5;
	splx(s);
}

scioctl(dev, cmd, arg, mode)
{
	if (ttiocom(sc_ttptr[scdev(dev)].tt_tty, cmd, arg, mode))
		scparam(dev);
}

scparam(dev)
{
	register flag;
	register struct tty *tp;
	register struct device *addr;
	int s;
	register int w4, w5, speed;

	tp = sc_ttptr[scdev(dev)].tt_tty;
	flag = tp->t_cflag;
	if (((flag&CBAUD) == B0) && (dev&MODEM)) { /* hang up line */
		schup(dev);
		return;
	}
	addr = (struct device *)sc_ttptr[scdev(dev)].tt_addr;
	w4 = W4CLK16;
	if (flag & CSTOPB)
		w4 |= W42STOP;
	else
		w4 |= W41STOP;
	w5 = W5TXENABLE;
	switch(flag & CSIZE) {
		case CS5:
			w5 |= W55BIT; break;
		case CS6:
			w5 |= W56BIT; break;
		case CS7:
			w5 |= W57BIT; break;
		case CS8:
			w5 |= W58BIT; break;
	}
	if (flag & PARENB)
		if (flag & PARODD)
			w4 |= W4PARENABLE;
		else
			w4 |= W4PARENABLE | W4PAREVEN;
	speed = sc_line[scdev(dev)].speed;
	speed = (speed/(scspeeds[flag&CBAUD]<<1)) - 2;
	s = spl6();
	addr->csr = 4;
	DELAY();
	addr->csr = w4;
	DELAY();
	addr->csr = 12;
	DELAY();
	addr->csr = speed;
	DELAY();
	addr->csr = 13;
	DELAY();
	addr->csr = speed >> 8;
	DELAY();
	addr->csr = 5;
	DELAY();
	addr->csr = w5;
	splx(s);
}

schup(dev)
{
	register struct device *addr;
	int s;

	dev = scdev(dev);
	addr = (struct device *)sc_ttptr[dev].tt_addr;
	s = spl6();
	addr->csr = 5;
	DELAY();
	addr->csr = W5TXENABLE | W58BIT;	/* turn off DTR/RTS */
	splx(s);
}

scintr(ap)
register struct args *ap;
{
	register struct device *addr;

	for (ap->a_dev = 0; ap->a_dev < sc_cnt; ap->a_dev++) {
		addr = (struct device *)sc_ttptr[ap->a_dev].tt_addr;
		while (addr->csr & R0RXRDY) {
			addr->csr = 1;
			DELAY();
			if (addr->csr & (R1PARERR|R1OVRERR|R1FRMERR))
				scsintr(ap);
			else
				scrintr(ap);
		}
		if (addr->csr & R0TXRDY)
			scxintr(ap);
	}
}

scrintr(ap)
register struct args *ap;
{
	register struct device *addr;
	register struct ccblock *cbp;
	register int c, lcnt, flg;
	struct tty *tp;
	register char ctmp;
	char lbuf[3];

	addr = (struct device *)sc_ttptr[ap->a_dev].tt_addr;
	addr->csr = W0RXINT;	/* reinable receiver interrupt */
	addr->csr = W0RIUS;	/* reset interrupt */
	c = addr->data & 0xFF;
	sysinfo.rcvint++;
	tp = sc_ttptr[ap->a_dev].tt_tty;
	if (tp->t_rbuf.c_ptr == NULL)
		return;
	if (tp->t_iflag & IXON) {
		ctmp = c & 0177;
		if (tp->t_state & TTSTOP) {
			if (ctmp == CSTART || tp->t_iflag & IXANY)
				(*tp->t_proc)(tp, T_RESUME);
		} else {
			if (ctmp == CSTOP)
				(*tp->t_proc)(tp, T_SUSPEND);
		}
		if (ctmp == CSTART || ctmp == CSTOP)
			return;
	}
	lcnt = 1;
	flg = tp->t_iflag;
	if (flg&ISTRIP)
		c &= 0177;
	else {
		/* c &= 0377; not needed */
		if (c == 0377 && flg&PARMRK) {
			lbuf[1] = 0377;
			lcnt = 2;
		}
	}
	/*
	 * Stash character in r_buf
	 */
	cbp = &tp->t_rbuf;
	if (lcnt != 1) {
		lbuf[0] = c;
		while (lcnt) {
			*cbp->c_ptr++ = lbuf[--lcnt];
			if (--cbp->c_count == 0) {
				cbp->c_ptr -= cbp->c_size;
				(*linesw[tp->t_line].l_input)(tp);
			}
		}
		if (cbp->c_size != cbp->c_count) {
			cbp->c_ptr -= cbp->c_size - cbp->c_count;
			(*linesw[tp->t_line].l_input)(tp);
		}
	} else {
		*cbp->c_ptr = c;
		cbp->c_count--;
		(*linesw[tp->t_line].l_input)(tp);
	}
}

scxintr(ap)
register struct args *ap;
{
	register short dev;
	struct tty *tp;
	register struct device *addr;

	sysinfo.xmtint++;
	dev = ap->a_dev;
	addr = (struct device *)sc_ttptr[dev].tt_addr;
	addr->csr = W0RTXPND;	/* reset transmitter interrupt */
	addr->csr = W0RIUS;	/* reset interrupt */
	tp = sc_ttptr[dev].tt_tty;
	tp->t_state &= ~BUSY;
	scproc(tp, T_OUTPUT);
}

scsintr(ap)
register struct args *ap;
{
	register struct ccblock *cbp;
	register int c, lcnt, flg;
	struct tty *tp;
	register char ctmp;
	char lbuf[3];
	register struct device *addr;
	unsigned char stat;

	sysinfo.rcvint++;
	addr = (struct device *)sc_ttptr[ap->a_dev].tt_addr;
	c = addr->data & 0xFF;	/* read data BEFORE reset error */
	addr->csr = 0x1;	/* cmd to read register 1 */
	stat = addr->csr;	/* read the status */
	addr->csr = W0RERROR;	/* reset error condition */
	addr->csr = W0RXINT;	/* reinable receiver interrupt */
	addr->csr = W0RIUS;	/* reset interrupt under service */
	tp = sc_ttptr[ap->a_dev].tt_tty;
	if (tp->t_rbuf.c_ptr == NULL)
		return;
	if (tp->t_iflag & IXON) {
		ctmp = c & 0177;
		if (tp->t_state & TTSTOP) {
			if (ctmp == CSTART || tp->t_iflag & IXANY)
				(*tp->t_proc)(tp, T_RESUME);
		} else {
			if (ctmp == CSTOP)
				(*tp->t_proc)(tp, T_SUSPEND);
		}
		if (ctmp == CSTART || ctmp == CSTOP)
			return;
	}
	/*
	 * Check for errors
	 */
	lcnt = 1;
	flg = tp->t_iflag;
	if (stat & (R1PARERR |R1OVRERR|R1FRMERR)) {
		if ((stat & R1PARERR ) && (flg & INPCK))
			c |= PERROR;
		if (stat & R1OVRERR)
			c |= OVERRUN;
		if (stat & R1FRMERR)
			c |= FRERROR;
	}
	if (c&(FRERROR|PERROR|OVERRUN)) {
		if ((c&0377) == 0) {
			if (flg&IGNBRK)
				return;
			if (flg&BRKINT) {
				signal(tp->t_pgrp, SIGINT);
				ttyflush(tp, (FREAD|FWRITE));
				return;
			}
		} else {
			if (flg&IGNPAR)
				return;
		}
		if (flg&PARMRK) {
			lbuf[2] = 0377;
			lbuf[1] = 0;
			lcnt = 3;
			sysinfo.rawch += 2;
		} else
			c = 0;
	} else {
		if (flg&ISTRIP)
			c &= 0177;
		else {
			/* c &= 0377; not needed */
			if (c == 0377 && flg&PARMRK) {
				lbuf[1] = 0377;
				lcnt = 2;
			}
		}
	}
	/*
	 * Stash character in r_buf
	 */
	cbp = &tp->t_rbuf;
	if (lcnt != 1) {
		lbuf[0] = c;
		while (lcnt) {
			*cbp->c_ptr++ = lbuf[--lcnt];
			if (--cbp->c_count == 0) {
				cbp->c_ptr -= cbp->c_size;
				(*linesw[tp->t_line].l_input)(tp);
			}
		}
		if (cbp->c_size != cbp->c_count) {
			cbp->c_ptr -= cbp->c_size - cbp->c_count;
			(*linesw[tp->t_line].l_input)(tp);
		}
	} else {
		*cbp->c_ptr = c;
		cbp->c_count--;
		(*linesw[tp->t_line].l_input)(tp);
	}
}

scscan()
{
	register int i;
	register struct tty *tp;
	register struct device *addr;

	timeout(scscan, (caddr_t)0, SCTIME);
	for (i = 0; i < sc_cnt; i++) {
		addr = (struct device *)sc_ttptr[i].tt_addr;
		addr->csr = W0REXT;	/* update DCD */
		tp = sc_ttptr[i].tt_tty;
		if (addr->csr & R0DCD) {
			if ((tp->t_state&CARR_ON) == 0) {
				tp->t_state |= CARR_ON;
				if (tp->t_state&WOPEN)
					wakeup((caddr_t)&tp->t_rawq);
			}
		} else {
			if (tp->t_state&CARR_ON && sc_modem[i]) {
				tp->t_state &= ~CARR_ON;
				if (tp->t_state&ISOPEN) {
					ttyflush(tp, FREAD|FWRITE);
					signal(tp->t_pgrp, SIGHUP);
				}
			}
		}
	}
}

#ifdef SCC_CONSOLE
scputchar(c)
{
	register struct device *addr ;
	int s, i;

	addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
	s = spl6();
	if (c == '\n')
		scputchar('\r');
	i = 100000;
	while ((addr->csr & R0TXRDY) == 0 && --i) ;
	addr->data = c;
	splx(s);
}
#endif SCC_CONSOLE
