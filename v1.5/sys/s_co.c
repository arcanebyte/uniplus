/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * co.c - "console" (ie, bitmap screen and keyboard) driver for the lisa.
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
#include "setjmp.h"
#include "sys/ioctl.h"
#include "sys/kb.h"
#include "sys/al_ioctl.h"
#ifdef SUNIX
#include "sys/reboot.h"
#endif SUNIX
#include "sys/mmu.h"
#include "sys/cops.h"
#include "sys/l2.h"

int	coproc();
extern int co_cnt;
extern struct tty co_tty[];
extern struct ttyptr co_ttptr[];

extern char bmbck, bmnormal;
extern char *bmscrn;		/* pointer to screen -- initialized in bminit */

/* calls to the putc routine are made indirectly through
 * the te_putc pointer which is used to
 * keep track of the current state for escape character 
 * processing, ie, although initialized to point to 
 * the normal putc, an escape character causes other 
 * functions to process the next character(s)
 */
extern int vt_putc();
int (*te_putc)()=vt_putc;
/*#ifdef SUNIX
extern caddr_t start;
#endif SUNIX
*/


struct device {
	char	csr;		/* Command status register */
	char	idum[2];	/* fillers */
	char	dbuf;		/* data buffer */
};


/* ARGSUSED */
coopen(dev, flag)
register dev;
{
	register struct tty *tp;

	if (dev >= co_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = co_ttptr[dev].tt_tty;
	tp->t_index = dev;
	SPL6();
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		tp->t_proc = coproc;
		ttinit(tp);		
		tp->t_state = WOPEN | CARR_ON;
		if (dev == CONSOLE) {
			tp->t_iflag = ICRNL | ISTRIP;
			tp->t_oflag = OPOST | ONLCR | TAB3;
			tp->t_lflag = ISIG | ICANON | ECHO | ECHOK;
			tp->t_cflag = sspeed | CS8 | CREAD | HUPCL;
		}
	}
	SPL0();
	(*linesw[tp->t_line].l_open)(tp);
}

/* ARGSUSED */
coclose(dev, flag)
{
	register struct tty *tp;

	tp = co_ttptr[dev].tt_tty;
	(*linesw[tp->t_line].l_close)(tp);
}

coread(dev)
{
	struct tty *tp;

	tp = co_ttptr[dev].tt_tty;
	(*linesw[tp->t_line].l_read)(tp);
}

cowrite(dev)
{
	struct tty *tp;

	tp = co_ttptr[dev].tt_tty;
	(*linesw[tp->t_line].l_write)(tp);
}

/* ARGSUSED */
coioctl(dev, cmd, arg, mode)
{
	int i;

	switch (cmd) {

	case AL_SBVOL:
		l2_bvol = arg & 7;
		break;
	case AL_SBPITCH:
		l2_bpitch = arg & 0x1FFF;
		break;
	case AL_SBTIME:
		if (arg <= 0)
			arg = 1;
		if (arg > 10 * v.v_hz)		/* limit to 10 seconds */
			arg = 10 * v.v_hz;
		l2_btime = arg;
		break;
	case AL_SDIMTIME:
		l2_dtime = arg;
		l2_dtrap = lbolt + l2_dtime;
		break;
	case AL_SDIMCONT:
		l2_dimcont = (~arg) & 0xFF;
		break;
	case AL_SDIMRATE:
		l2_crate = arg;
		break;
	case AL_SCONTRAST:
		l2_defcont = (~arg) & 0xFF;
		l2_desired = l2_defcont;
		l2ramp(0);
		break;
	case AL_SREPWAIT:
		kb_repwait = arg;
		break;
	case AL_SREPDELAY:
		kb_repdlay = arg;
		break;
	case AL_GBVOL:
		i = l2_bvol;
		if (copyout((caddr_t)&i, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GBPITCH:
		if (copyout((caddr_t)&l2_bpitch, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GBTIME:
		if (copyout((caddr_t)&l2_btime, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GDIMTIME:
		if (copyout((caddr_t)&l2_dtime, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GDIMCONT:
		i = (~l2_dimcont) & 0xFF;
		if (copyout((caddr_t)&i, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GDIMRATE:
		if (copyout((caddr_t)&l2_crate, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GCONTRAST:
		i = (~l2_defcont) & 0xFF;
		if (copyout((caddr_t)&i, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GREPWAIT:
		i = kb_repwait & 0xFFFF;
		if (copyout((caddr_t)&i, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GREPDELAY:
		i = kb_repdlay & 0xFFFF;
		if (copyout((caddr_t)&i, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_GBMADDR:
		if (copyout((caddr_t)&bmscrn, (caddr_t)arg, 4))
			u.u_error = EFAULT;
		break;
	case AL_REVVIDEO:
		if (arg > 0)
			i = 0;
		else if (arg == 0)
			i = -1;
		else
			i = (bmbck)? 0 : -1;
		if (bmbck != i) {
			bmswitch();
			bmsinv();
		}
		bmbck = i;
		bmnormal = bmbck;
		break;
#ifdef SUNIX
	case RESTART:		/* jump to the start of unix */
		reinit();
		((int (*)())0xC000)();
		break;
#endif SUNIX
	default:
		(void) ttiocom(co_ttptr[0].tt_tty, cmd, arg, mode);
		break;
	}
}

coproc(tp, cmd)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	extern ttrstrt();

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
		if (tp->t_state & (TTSTOP|TIMEOUT|BUSY))
			break;
		tbuf = &tp->t_tbuf;
		if ((tbuf->c_ptr == 0) || (tbuf->c_count == 0)) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
				break;
		}
		(*te_putc)((*tbuf->c_ptr++)&0x7f);
		tbuf->c_count--;
		sysinfo.xmtint++;	/* this is the xmit interrupt */
		splx(spl1());
		goto start;

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		break;

	case T_RFLUSH:
		if (!(tp->t_state&TBLOCK))
			break;
		/* fall through */

	case T_UNBLOCK:
		break;

	case T_BREAK:
		break;
	}
}

cointr(dev)
{
	register struct ccblock *cbp;
	register int c, lcnt, flg;
	struct tty *tp;
	register char ctmp;
	char lbuf[3];

	sysinfo.rcvint++;
	c = kb_chrbuf;
	tp = co_ttptr[dev].tt_tty;
	if (tp->t_rbuf.c_ptr == NULL)
		return;
#undef NULLDEBUG
#ifdef NULLDEBUG
	if( c == 0x00 ) {
		sccdebug();
		return;
	}
#endif
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
	if (flg&ISTRIP)
		c &= 0177;
	else {
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

/*
 * This version of putchar writes directly to the bitmap display
 * for those last-ditch situations when you just have to get stuff to the CRT.
 */
coputchar(c)
register c;
{
	(*te_putc)(c & 0x7F);
	if (c == '\n')
		(*te_putc)('\r');
}
