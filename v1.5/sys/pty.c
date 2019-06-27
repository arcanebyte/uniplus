/*	pty.c	4.21	82/03/23	*/

/*
 * Pseudo-teletype Driver
 * (Actually two drivers, requiring two entries in 'cdevsw')
 */

/*
 *  billn -- 12/15/82.  Mercilessly hacked for system 3.  To do
 *  Remote input editing,  etc, need to rework it again
 *  from the original.
 */

#include "net/pty.h"

#if NPTY > 0
#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/tty.h"
#include "sys/ttold.h"
#include "sys/termio.h"
#include "sys/ioctl.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "net/misc.h"

#define BUFSIZ 100		/* Chunk size iomoved from user */

/*
 * pts == /dev/tty[pP]?
 * ptc == /dev/pty[pP]?
 */
struct  tty pt_tty[NPTY];
struct	pt_ioctl {
	int	pt_flags;
	int	pt_gensym;
	struct	proc *pt_selr, *pt_selw;
	int	pt_send;
} pt_ioctl[NPTY];

#define	PF_RCOLL	0x01
#define	PF_WCOLL	0x02
#define	PF_NBIO		0x04
#define	PF_PKT		0x08		/* packet mode */
#define	PF_STOPPED	0x10		/* user told stopped */
#define	PF_REMOTE	0x20		/* remote and flow controlled input */
#define	PF_NOSTOP	0x40
#define PF_WTIMER       0x80            /* waiting for timer to flush */
/* billn -- kludge */
#define	PF_PTSOPEN	0x100		/* pts side is open */
#define	PF_PTCOPEN	0x200		/* ptc side is open */

/*ARGSUSED*/
ptsopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

	if (dev >= NPTY) {
		u.u_error = ENXIO;
		return;
	}
	tp = &pt_tty[dev];
	if ((tp->t_state & ISOPEN) == 0) {
		/* No features (nor raw mode) */
		tp->t_cflag = 0; tp->t_oflag = 0;
		tp->t_iflag = 0; tp->t_lflag = 0;
		ttinit(tp);		/* Set up default chars */
	}
	if (tp->t_proc)			/* Ctrlr still around. */
		tp->t_state |= CARR_ON;
	while ((tp->t_state & CARR_ON) == 0) {
		tp->t_state |= WOPEN;
		(void) sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	(*linesw[tp->t_line].l_open)(tp);
	pti->pt_flags |= PF_PTSOPEN;
}

ptsclose(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

	tp = &pt_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);
	pti->pt_flags &= ~PF_PTSOPEN;
	if ((pti->pt_flags & PF_PTCOPEN) == 0)	/* other end already gone? */
		tp->t_proc = 0;
}

ptsread(dev)
	dev_t dev;
{
	register struct tty *tp = &pt_tty[dev];
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

	if (tp->t_proc)
		(*linesw[tp->t_line].l_read)(tp);
	wakeup((caddr_t)&tp->t_rawq.c_cf);
	if (pti->pt_selw) {
		selwakeup(pti->pt_selw, pti->pt_flags & PF_WCOLL);
		pti->pt_selw = 0;
		pti->pt_flags &= ~PF_WCOLL;
	}
}

/*
 * Write to pseudo-tty.
 * Wakeups of controlling tty will happen
 * indirectly, when tty driver calls ptsstart.
 */
ptswrite(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = &pt_tty[dev];
	if (tp->t_proc)
		(*linesw[tp->t_line].l_write)(tp);
}


ptcwakeup(tp)
	struct tty *tp;
{
	struct pt_ioctl *pti = &pt_ioctl[tp - &pt_tty[0]];
	int s = spl5();         /* any NZ spl will lockout ptctimer */

	if (pti->pt_selr) {
		selwakeup(pti->pt_selr, pti->pt_flags & PF_RCOLL);
		pti->pt_selr = 0;
		pti->pt_flags &= ~PF_RCOLL;
	}
	if (pti->pt_selw) {
		selwakeup(pti->pt_selw, pti->pt_flags & PF_WCOLL);
		pti->pt_selw = 0;
		pti->pt_flags &= ~PF_WCOLL;
	}
	wakeup((caddr_t)&tp->t_outq.c_cf);
	splx(s);
}

ptctimer()
{
	register struct tty *tp = &pt_tty[0];
	register struct pt_ioctl *pti = &pt_ioctl[0];
	register i;

	timeout(ptctimer, (caddr_t)0, v.v_hz >> 2);
	for (i=0; i<NPTY; i++, pti++, tp++) {
		if ((pti->pt_flags & PF_WTIMER) == 0)
			continue;
		pti->pt_flags &= ~PF_WTIMER;
		if (tp->t_proc == 0)
			continue;
		ptcwakeup(tp);
	}
}

/*ARGSUSED*/
ptcopen(dev, flag)
dev_t dev;
int flag;
{
	register struct tty *tp;
	struct pt_ioctl *pti;
	static first;
	extern int ptsstart();

	if (first == 0) {
		first++;
		ptctimer();
	}
	if (dev >= NPTY) {
		u.u_error = ENXIO;
		return;
	}
	tp = &pt_tty[dev];
	if (tp->t_proc) {
		u.u_error = EIO;
		return;
	}
	tp->t_iflag = ICRNL|ISTRIP|IGNPAR;
	tp->t_oflag = OPOST|ONLCR|TAB3;
	tp->t_lflag = ISIG|ICANON; /* no echo */
	tp->t_proc = ptsstart;
	if (tp->t_state & WOPEN)
		wakeup((caddr_t)&tp->t_rawq);
	tp->t_state |= CARR_ON;
	pti = &pt_ioctl[dev];
	pti->pt_flags = 0;
	pti->pt_send = 0;
	pti->pt_flags |= PF_PTCOPEN;
}

ptcclose(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

	tp = &pt_tty[dev];
	if (tp->t_state & ISOPEN)
		signal(tp->t_pgrp, SIGHUP);
	tp->t_state &= ~CARR_ON;	/* virtual carrier gone */
	ttyflush(tp, FREAD|FWRITE);
	pti->pt_flags &= ~PF_PTCOPEN;
	if ((pti->pt_flags & PF_PTSOPEN) == 0)	/* other end already gone? */
	    tp->t_proc = 0;		/* mark closed */
}

ptcread(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct pt_ioctl *pti;
	register c;

	tp = &pt_tty[dev];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return;
	pti = &pt_ioctl[dev];
	if (pti->pt_flags & PF_PKT) {
		if (pti->pt_send) {
			(void) passc(pti->pt_send);
			pti->pt_send = 0;
			return;
		}
		(void) passc(0);
	}
	while (tp->t_outq.c_cc == 0 || (tp->t_state&TTSTOP)) {
		if (pti->pt_flags&PF_NBIO) {
			u.u_error = EWOULDBLOCK;
			return;
		}
		(void) sleep((caddr_t)&tp->t_outq.c_cf, TTIPRI);
	}
	while (tp->t_outq.c_cc && (c = getc(&tp->t_outq)) >= 0)
		if (passc(c) < 0)
			break;
	tp->t_state &= ~BUSY;
	if (tp->t_state&OASLP) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)&tp->t_outq);
	}
	if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
}


/*
 * System 5 does not have nbio normally
 */
/* #define IF_NBIO */

ptcwrite(dev)
	dev_t dev;
{
	register struct tty *tp;
	register char *cp, *ce;
	register int cc, c, ctmp;
	char locbuf[BUFSIZ];
#ifdef IF_NBIO
	int cnt = 0;
#endif

	tp = &pt_tty[dev];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return;
	do {
		cc = MIN(u.u_count, BUFSIZ);
		cp = locbuf;
		iomove(cp, cc, B_WRITE);
		if (u.u_error)
			break;
		ce = cp + cc;
again:
		while (cp < ce) {
			while (tp->t_delct && tp->t_rawq.c_cc >= TTYHOG - 2) {
				wakeup((caddr_t)&tp->t_rawq);
#ifdef IF_NBIO
				if (tp->t_state & TS_NBIO) {
					u.u_count += ce - cp;
					if (cnt == 0)
						u.u_error = EWOULDBLOCK;
					return;
				}
#endif
				/* Better than just flushing it! */
				/* Wait for something to be read */
				(void) sleep((caddr_t)&tp->t_rawq.c_cf, TTOPRI);
				goto again;
			}
			c = *cp++;
			if (tp->t_iflag & IXON) {
				ctmp = c & 0177;
				if (tp->t_state & TTSTOP) {
					if (c == CSTART || tp->t_iflag & IXANY)
						(*tp->t_proc)(tp, T_RESUME);
				} else
					if (c == CSTOP)
						(*tp->t_proc)(tp, T_SUSPEND);
				if (c == CSTART || c == CSTOP)
					continue;
			}
			if (tp->t_rbuf.c_ptr != NULL) {
				if (tp->t_iflag&ISTRIP)
					c &= 0177;
				*tp->t_rbuf.c_ptr = c;
				tp->t_rbuf.c_count--;
				(*linesw[tp->t_line].l_input)(tp);
			}
#ifdef IF_NBIO
			cnt++;
#endif
		}
	} while (u.u_count);
}

ptcioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t dev;
{
	register struct tty *tp = &pt_tty[minor(dev)];
	register struct pt_ioctl *pti = &pt_ioctl[dev];

	if (cmd == TIOCPKT) {
		int packet;
		if (copyin((caddr_t)addr, (caddr_t)&packet, sizeof (packet))) {
			u.u_error = EFAULT;
			return;
		}
		if (packet)
			pti->pt_flags |= PF_PKT;
		else
			pti->pt_flags &= ~PF_PKT;
		return;
	}
	if (cmd == FIONBIO) {
		int nbio;
		if (copyin((caddr_t)addr, (caddr_t)&nbio, sizeof (nbio))) {
			u.u_error = EFAULT;
			return;
		}
		if (nbio)
			pti->pt_flags |= PF_NBIO;
		else
			pti->pt_flags &= ~PF_NBIO;
		return;
	}
	/* IF CONTROLLER STTY THEN MUST FLUSH TO PREVENT A HANG ???  */
        if ((cmd==TIOCSETP)||(cmd==TCSETAW)) {
		while (getc(&tp->t_outq) >= 0);
		tp->t_state &= ~BUSY;
	}
	ptsioctl(dev, cmd, addr, flag);
}

/*ARGSUSED*/
ptsioctl(dev, cmd, addr, flag)
register caddr_t addr;
register dev_t dev;
{
	register struct tty *tp = &pt_tty[dev];
	register struct pt_ioctl *pti = &pt_ioctl[dev];
	register int stop;

	if (ttiocom(tp, cmd, (int)addr, dev) == 0)
		;
	/* else...?? */
	stop = tp->t_iflag&IXON;
	if (pti->pt_flags & PF_NOSTOP) {
		if (stop) {
			pti->pt_send &= TIOCPKT_NOSTOP;
			pti->pt_send |= TIOCPKT_DOSTOP;
			pti->pt_flags &= ~PF_NOSTOP;
			ptcwakeup(tp);
		}
	} else {
		if (stop == 0) {
			pti->pt_send &= ~TIOCPKT_DOSTOP;
			pti->pt_send |= TIOCPKT_NOSTOP;
			pti->pt_flags |= PF_NOSTOP;
			ptcwakeup(tp);
		}
	}
}

ptsstart(tp, cmd)
register struct tty *tp;
{
	register struct pt_ioctl *pti = &pt_ioctl[tp - &pt_tty[0]];
        extern ttrstrt();

        switch(cmd) {
        case T_TIME:
                tp->t_state &= ~TIMEOUT;
                goto start;

        case T_WFLUSH:
		if (tp->t_outq.c_cc) {
			while (getc(&tp->t_outq) >= 0)
				;
			tp->t_state &= ~BUSY;
		}
		/* fall through */

        case T_RESUME:
                tp->t_state &= ~TTSTOP;
		wakeup((caddr_t)&tp->t_outq.c_cf);
		/* fall through */

        case T_OUTPUT:
start:
                if (tp->t_state&(TIMEOUT|TTSTOP|BUSY))
                        break;
		if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
		if (pti->pt_flags & PF_STOPPED) {
			pti->pt_flags &= ~PF_STOPPED;
			pti->pt_send = TIOCPKT_START;
		}
		if (tp->t_outq.c_cc < 200) {
			pti->pt_flags |= PF_WTIMER;
			return;
		}
		pti->pt_flags &= ~PF_WTIMER;
		tp->t_state |= BUSY;
		ptcwakeup(tp);
                if (tp->t_state&OASLP &&
                    tp->t_outq.c_cc <= ttlowat[tp->t_cflag&CBAUD]) {
                        tp->t_state &= ~OASLP;
                        wakeup((caddr_t)&tp->t_outq);
                }
                break;

        case T_SUSPEND:
                tp->t_state |= TTSTOP;
		pti->pt_flags |= PF_STOPPED;
		pti->pt_send |= TIOCPKT_STOP;
                break;

        case T_BLOCK:
                tp->t_state |= TBLOCK;
                tp->t_state &= ~TTXON;
                if(tp->t_outq.c_cc > 0)
                        wakeup((caddr_t)&tp->t_outq.c_cf);
                break;

        case T_RFLUSH:
                if (!(tp->t_state&TBLOCK))
                        break;

        case T_UNBLOCK:
                tp->t_state &= ~(TTXOFF|TBLOCK);
                if(tp->t_outq.c_cc > 0)
                        wakeup((caddr_t)&tp->t_outq.c_cf);
                break;

        case T_BREAK:
                tp->t_state |= TIMEOUT;
                timeout(ttrstrt, (caddr_t)tp, v.v_hz/4);
                break;
        }
}

ptcselect(dev, rw)
	dev_t dev;
	int rw;
{
	register struct tty *tp = &pt_tty[minor(dev)];
	struct pt_ioctl *pti = &pt_ioctl[minor(dev)];
	struct proc *p;
	int s;

	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return (1);   	/* ??? billn */
	s = spl7();
	switch (rw) {

	case FREAD:
		if (tp->t_outq.c_cc && (tp->t_state&TTSTOP) == 0) {
			splx(s);
			return (1);
		}
		if ((p = pti->pt_selr) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_RCOLL;
		else
			pti->pt_selr = u.u_procp;
		break;

	case FWRITE:
		splx(s);
		return 1;
#ifdef notdef
		/*
		 * only do this if using "PF_REMOTE" ala 4.1a .  So why keep
		 * it?  -- history...
		 */
		if (tp->t_rawq.c_cc == 0) {
			splx(s);
			return (1);
		}
		if ((p = pti->pt_selw) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_WCOLL;
		else
			pti->pt_selw = u.u_procp;
#endif
		break;
	}
	splx(s);
	return (0);
}
#endif
