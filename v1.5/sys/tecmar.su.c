/*
 *	Lisa INS8250A device driver
 *
 *	Copyright 1984 UniSoft Corporation
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
#include "setjmp.h"
#include "sys/reg.h"
#include "sys/mmu.h"
#include "sys/proc.h"

int	teproc();

/*
 * structure to access Tecmar device registers
 */
struct tedevice {
	char	fill1;
	char	te_rbr;		/* +1 data register */
#define te_dvlsb te_rbr		/*    lsb of divisor latch */
	char	fill2;
	char	te_ier;		/* +3 interrupt enable register */
#define te_dvmsb te_ier		/*    msb of divisor latch */
	char	fill3;
	char	te_iir;		/* +5 interrupt id register */
	char	fill4;
	char	te_lcr;		/* +7 line control register */
	char	fill5;
	char	te_mcr;		/* +9 modem control register */
	char	fill6;
	char	te_lsr;		/* +11 line status register */
	char	fill7;
	char	te_msr;		/* +13 modem status register */
	char	fill8;
	char	te_scrat;	/* +15 scratch register (8250-A only) */
	char	fill[0x200-16];	/* sized to make tedevice be 0x200 long */
};

/*
 * structure to access the interrupt reset bit
 */
struct teidevice {
	struct tedevice fill[7];/* filler */
	char	skip;
	char	te_intr;	/* interrupt reset location */
};

/*
 * array used to remap ivec interrupt board slot number to tty slot
 */
int te_remap[3];

/*
 * Slot id to interrupt reset address
 */
struct teidevice *te_idevice[3] = {
	(struct teidevice *)(STDIO),
	(struct teidevice *)(STDIO+0x4000),
	(struct teidevice *)(STDIO+0x8000)
};

extern struct tty te_tty[1];
extern struct ttyptr te_ttptr[1];
extern char te_dparam[1];
extern char te_modem[1];
extern int te_cnt;

#define MODEM		0x80		/* modem control on bit */
#define tedev(d)	((d)&0x7f)	/* from unix device number to device */

#define BAUD50		2304
#define BAUD75		1356
#define BAUD110		1047
#define BAUD134		857
#define BAUD150		768
#define BAUD300		384
#define BAUD600		192
#define BAUD1200	96
#define BAUD1800	64
#define BAUD2000	58
#define BAUD2400	48
#define BAUD3600	32
#define BAUD4800	24
#define BAUD7200	16
#define BAUD9600	12
#define BAUD19200	6
#define BAUD38400	3
#define BAUD56000	2

/* -1 means hangup, -2 means invalid */
int	tebaudmap[] = {
		-1, BAUD50, BAUD75, BAUD110, BAUD134, BAUD150, BAUD134,
		BAUD300, BAUD600, BAUD1200, BAUD1800, BAUD2400,
		BAUD4800, BAUD9600, BAUD19200, BAUD38400
	};

	/* Interrupt enable register bits */
#define ERBFI	0x01	/* Enable received data available interrupt */
#define ETBEI	0x02	/* Enable transmitter enable holding register empty interrupt */
#define ELSI	0x04	/* Enable receiver line status interrupt */
#define EDSSI	0x08	/* Enable modem status interrupt */

	/* Interrupt ident register bits */
#define IRQ	0x01	/* Interrupt request, 0 if interrupt pending */
#define THE	0x02	/* Transmitter holding register empty */
#define IID	0x06	/* Interrupt ID bit mask */
#define RID	0x04	/* Interrupt receive bit mask */

	/* Line control register bits */
#define BITS5	0x00	/* 5 bits */
#define BITS6	0x01	/* 6 bits */
#define BITS7	0x02	/* 7 bits */
#define BITS8	0x03	/* 8 bits */
#define STOP1	0x00	/* One stop bit */
#define STOP2	0x04	/* Two stop bit */
#define PEN	0x08	/* Parity enable */
#define EPS	0x10	/* Even parity select */
#define SPS	0x20	/* Stick parity */
#define SBRK	0x40	/* Set break */
#define DLAB	0x80	/* Divisor latch access. i/o direction bit */

	/* Modem control register bits */
#define DTR	0x01	/* Data terminal ready */
#define RTS	0x02	/* Request to send */

	/* Line status register bits */
#define DATARDY	0x01	/* Data ready */
#define OV_ERR	0x02	/* Overrun error on receiver */
#define PE_ERR	0x04	/* Parity error */
#define FR_ERR	0x08	/* Framing error */
#define BR_INT	0x10	/* Break interrupt */
#define THRE	0x20	/* Transmitter holding register */
#define TEMT	0x40	/* Transmitter empty */

	/* Modem status register bits */
#define DCTS	0x01	/* Delta clear to send */
#define DDSR	0x02	/* Delta data set ready */
#define TERI	0x04	/* Trailing edge ring indicator */
#define DDCD	0x08	/* Delta data carrier detect */
#define CTS	0x10	/* Clear to send */
#define DSR	0x20	/* Data set ready */
#define RI	0x40	/* Ring indicator */
#define DCD	0x80	/* Data carrier detect */

int teslotsused = 0;

/*
 * Initialize the baud rate
 *     slot = 0, 1, or 2
 */
teinit(slot)
{
	register i, val;

	if (teslotsused+4 > te_cnt) {
		printf("\n\nSystem only configured for %d tecmar ports\n\n", te_cnt);
		return(1);
	}
	te_remap[slot] = teslotsused;
	val = STDIO + slot*0x4000 + 0x200;
	for (i = teslotsused; i < teslotsused+4; i++) {
		te_ttptr[i].tt_addr = val;
		te_ttptr[i].tt_tty = &te_tty[i];
		val += 0x200;
	}
	teslotsused += 4;
	return(0);
}

/* ARGSUSED */
teopen(dev, flag)
dev_t dev;
{
	register struct tedevice *addr;
	register struct tty *tp;
	register d;
#ifdef SINGLEUSER
	register struct proc *p;
#endif SINGLEUSER

	d = tedev(dev);
	if (d >= te_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = te_ttptr[d].tt_tty;
#ifdef SINGLEUSER
	p = u.u_procp;
	if ((p->p_pid == p->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0)) {
		u.u_error = ENOTTY;
		return;
	}
#endif SINGLEUSER
	addr = (struct tedevice *)te_ttptr[d].tt_addr;
	if (tp == 0 || addr == 0) {
		u.u_error = ENXIO;
		return;
	}
	tp->t_index = d;
	SPL5();
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		tp->t_proc = teproc;
		ttinit(tp);
		tp->t_iflag = ICRNL | ISTRIP;
		tp->t_oflag = OPOST | ONLCR | TAB3;
		tp->t_lflag = ISIG | ICANON | ECHO | ECHOK;
		tp->t_cflag = sspeed | CS8 | CREAD | HUPCL;
		teparam(dev);
	}
	te_modem[d] = dev & MODEM;
	if ((dev&MODEM)==0 || addr->te_msr&DSR)
		tp->t_state |= CARR_ON;
	else
		tp->t_state &= ~CARR_ON;
	if (!(flag & FNDELAY))
		while ((tp->t_state&CARR_ON) == 0) {
			tp->t_state |= WOPEN;
			(void) sleep((caddr_t)&tp->t_rawq, TTOPRI);
		}
	SPL0();
	(*linesw[tp->t_line].l_open)(tp);
}

/* ARGSUSED */
teclose(dev, flag)
dev_t dev;
int flag;
{
	register struct tedevice *addr;
	register struct tty *tp;
	register d;

	d = tedev(dev);
	tp = te_ttptr[d].tt_tty;
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag&HUPCL) {
		addr = (struct tedevice *)te_ttptr[d].tt_addr;
		d = 0;
		addr->te_mcr = d;
	}
}

teread(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = te_ttptr[tedev(dev)].tt_tty;
	(*linesw[tp->t_line].l_read)(tp);
}

tewrite(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = te_ttptr[tedev(dev)].tt_tty;
	(*linesw[tp->t_line].l_write)(tp);
}

teproc(tp, cmd)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register struct tedevice *addr;
	register dev_t dev;
	int s;
	extern ttrstrt();

	s = spltty();
	dev = tp->t_index;
	addr = (struct tedevice *)te_ttptr[dev].tt_addr;
	switch (cmd) {

	case T_TIME:
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
		if (tp->t_state & (TTSTOP|TIMEOUT|BUSY)) {
			/* if ((tp->t_state & BUSY) == 0) {
				addr = (struct tedevice *)((int)addr & 0xFCC000);
				((struct teidevice *)addr)->te_intr = dev;
			} */
			break;
		}
		if (tp->t_state & TTXOFF) {
			tp->t_state &= ~TTXOFF;
			tp->t_state |= BUSY;
			addr->te_rbr = CSTOP;
			break;
		}
		if (tp->t_state & TTXON) {
			tp->t_state &= ~TTXON;
			tp->t_state |= BUSY;
			addr->te_rbr = CSTART;
			break;
		}
		tbuf = &tp->t_tbuf;
		if ((tbuf->c_ptr == 0) || (tbuf->c_count == 0)) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp))) {
				break;
			}
		}
		tp->t_state |= BUSY;
		addr->te_ier = ERBFI | ETBEI | ELSI | EDSSI;
		addr->te_rbr = *tbuf->c_ptr++;
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
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, (caddr_t)tp, v.v_hz>>2);
		break;
	}
	splx(s);
}

teioctl(dev, cmd, arg, mode)
dev_t dev;
{
	if (ttiocom(te_ttptr[tedev(dev)].tt_tty, cmd, arg, mode))
		teparam(dev);
}

teparam(dev)
register dev_t dev;
{
	register struct tty *tp;
	register struct tedevice *addr;
	register int s, speed, oldpri;
	char c;

	tp = te_ttptr[tedev(dev)].tt_tty;
	addr = (struct tedevice *)te_ttptr[tedev(dev)].tt_addr;
	/* check for invalid speed */
	if ((speed = tebaudmap[tp->t_cflag & CBAUD]) == -2) {
		u.u_error = EINVAL;
		return;
	}
	s = 0;

	/*
	 * hangup the line
	 */
	if (speed == -1) {
		addr->te_mcr = s;
		return;
	}

	if (tp->t_state & BUSY) {
		te_dparam[tedev(dev)] = 1;
		return;
	}

	/*
	 * set new speed
	 */
	oldpri = spltty();
	addr->te_lcr = DLAB;
	addr->te_dvlsb = speed;
	addr->te_dvmsb = speed >> 8;
	addr->te_lcr = s;

	/*
	 * set line control information
	 */
	if ((tp->t_cflag & CSIZE) == CS8)
		s |= BITS8;
	else if ((tp->t_cflag & CSIZE) == CS7)
		s |= BITS7;
	else if ((tp->t_cflag & CSIZE) == CS6)
		s |= BITS6;
	else if ((tp->t_cflag & CSIZE) == CS5)
		s |= BITS5;
	if (tp->t_cflag & CSTOPB)
		s |= STOP2;
	if (tp->t_cflag & PARENB)
		if ((tp->t_cflag & PARODD) == 0)
			s |= PEN|EPS;
	addr->te_lcr = s;

	/*
	 * set modem control information
	 */
	addr->te_mcr = DTR | RTS;

	/*
	 * enable interrupts
	 */
	addr->te_ier = ERBFI | ETBEI | ELSI | EDSSI;

	/*
	 * reset pending interrupts
	 */
	c = addr->te_rbr;
	c = addr->te_lsr;
	c = addr->te_msr;
	c = addr->te_iir;

	splx(oldpri);
}


/* VARARGS */
teintr(ap)
struct args *ap;
{
	register struct tedevice *addr;
	register struct ccblock *cbp;
	register struct tty *tp;
	register int c, lcnt, flg, iir, lsr;
	register char ctmp;
	int i, any;
	struct teidevice *iaddr;
	int index, s;
	char lbuf[3];

	s = spl5();
	index = te_remap[ap->a_dev];
	iaddr = te_idevice[ap->a_dev];
	c = 0;
	iaddr->te_intr = c;	/* reset master interrupt */
again:
	any = 0;
	for (i = index; i < index+4; i++) {
		addr = (struct tedevice *)te_ttptr[i].tt_addr;
restart:
		iir = addr->te_iir;
		if (iir & IRQ)
			continue;
		tp = te_ttptr[i].tt_tty;
		lsr = addr->te_lsr;
		if (iir & RID) {
			sysinfo.rcvint++;
			c = addr->te_rbr & 0xFF;
			if (tp->t_rbuf.c_ptr == NULL) {
				any++;
				goto restart;
			}
			if ((lsr & DATARDY) == 0)
				c = 0;
			if (tp->t_iflag & IXON) {
				ctmp = c & 0177;
				if (tp->t_state & TTSTOP) {
					if (ctmp == CSTART || tp->t_iflag & IXANY)
						(*tp->t_proc)(tp, T_RESUME);
				} else {
					if (ctmp == CSTOP)
						(*tp->t_proc)(tp, T_SUSPEND);
				}
				if (ctmp == CSTART || ctmp == CSTOP) {
					any++;
					goto restart;
				}
			}
			/*
			 * Check for errors
			 */
			lcnt = 1;
			flg = tp->t_iflag;
			if (lsr & (PE_ERR|FR_ERR|OV_ERR|BR_INT)) {
				if ((lsr & PE_ERR) && (flg & INPCK))
					c |= PERROR;
				if (lsr & OV_ERR)
					c |= OVERRUN;
				if (lsr & FR_ERR)
					c |= FRERROR;
				if (lsr & BR_INT)
					c = OVERRUN;	/* reset char on a break */
			}
			if (c&(FRERROR|PERROR|OVERRUN)) {
				if ((c&0377) == 0) {
					if (flg&IGNBRK) {
						any++;
						goto restart;
					}
					if (flg&BRKINT) {
						signal(tp->t_pgrp, SIGINT);
						ttyflush(tp, (FREAD|FWRITE));
						any++;
						goto restart;
					}
				} else {
					if (flg&IGNPAR) {
						any++;
						goto restart;
					}
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
			if (cbp->c_ptr == NULL) {
				any++;
				goto restart;
			}
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
			any++;
			goto restart;
		}
		if (iir&THE) {
			sysinfo.xmtint++;
			tp->t_state &= ~BUSY;
			if (te_dparam[i]) {
				te_dparam[i] = 0;
				teparam(i);
			}
			teproc(tp, T_OUTPUT);
		} else {
			/*
			 * must be a modem transition interrupt
			 */
			temodem(tp, addr->te_msr);
		}
		any++;
		goto restart;
	}
	if (any != 0)
		goto again;
	splx(s);
}

temodem(tp, msr)
register struct tty *tp;
{
	if ((tp->t_state&(ISOPEN|WOPEN))==0)
		return;
	if (msr & DSR || te_modem[tp->t_index] == 0) {
		if ((tp->t_state&CARR_ON) == 0) {
			tp->t_state |= CARR_ON;
			if (tp->t_state&WOPEN)
				wakeup((caddr_t)&tp->t_rawq);
		}
	} else {
		if (tp->t_state&CARR_ON) {
			tp->t_state &= ~CARR_ON;
			if (tp->t_rbuf.c_ptr != NULL) {
				ttyflush(tp, FREAD|FWRITE);
				signal(tp->t_pgrp, SIGHUP);
			}
		}
	}
}
