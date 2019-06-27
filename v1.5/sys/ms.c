/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Mouse Driver
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/reg.h"
#include "sys/buf.h"
#include "setjmp.h"
#include "sys/cops.h"
#include "sys/local.h"
#include "sys/elog.h"
#include "sys/ms.h"
#include "sys/mouse.h"
#include "sys/mmu.h"
#include "sys/kb.h"
#include "sys/al_ioctl.h"

#define splms	spl2

	/* flags local to ms.c */
char mouseopen;			/* active flag */
char msblkd;			/* flag to sleep on when blocked */

	/* also used in l1.c (vertical retrace interrupt code) */
struct msparms mparm;		/* place to save ioctl data */
struct proc *msproc;		/* controlling process for mouse */
char msvrmsk;			/* mask of low bits of retrace counter */

	/* values set in kb.c */
extern char ms_plg, ms_btn;	/* mouse plugged-in and button state */
extern short ms_row, ms_col;	/* last received row and column */
extern time_t lbolt;

#define QUELEN 128
typedef unsigned long mem_t;
int mqlen;
mem_t *mqlo, *mqhi;		/* high and low pointers into ... */
mem_t mqbuf[QUELEN];		/* ring buffer for mouse data */
mem_t mtim[QUELEN];

msopen(dev, flag)
dev_t dev;
{
	if (dev != 0) {		/* minor device number is wrong */
		u.u_error = ENXIO;
		return;
	}
	if (ms_plg) {		/* mouse is not plugged in */
		u.u_error = ENODEV;
		return;
	}
	if (flag != 1) {	/* open for writing !! */
		u.u_error = EINVAL;
		return;
	}
	if (u.u_ttyd != CONSOLE) {	/* Controlling tty not bitmap */
		u.u_error = EPERM;
		return;
	}
	if (mouseopen++ > 0) {	/* already opened */
		u.u_error = EBUSY;
		return;
	}
	splms();
	msproc = u.u_procp;
	mqhi = mqlo = mqbuf;
	mqlen = 0;
	mparm.mp_irate = 28;
	mparm.mp_fdlay = 300;
	mparm.mp_flags = MF_BUT;
	l2copscmd(MOUSEON);
	spl0();
}

/*
 * Reset everything since code elsewhere (in l1.c) uses it.
 */
/* ARGSUSED */
msclose(dev, flag)
{
	if (mouseopen <= 0)
		u.u_error = EINVAL;
	else {
		SPL6();
		mouseopen = 0;		/* only accessed in ms.c */
		mqhi = mqlo = mqbuf;
		mqlen = 0;
		mparm.mp_irate = 0;
		mparm.mp_fdlay = 0;
		mparm.mp_flags = 0;
		msvrmsk = 0;
		l2copscmd(MOUSEOFF);
		SPL0();
	}
}

msstrategy(bp)
register struct buf *bp;
{
	register mem_t *rp;		/* reciever buffer pointer */
	register int i;			/* record count */

/* Make sure the mouse is plugged in and the read request is for a
 * multiple of the record size.
 */
	if (ms_plg) {			/* mouse unplugged */
		u.u_error = ENODEV;
		goto fail;
	}
	if (bp->b_bcount & 3) {		/* count not multiple of record size */
		u.u_error = EINVAL;
fail:
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return;
	}
	if (mqlo == mqhi) {		/* queue empty */
		if ((int)mparm.mp_irate <= 0)	/* mouse shutdown */
			u.u_error = EIO;
		splms();
		if (mparm.mp_flags & MF_BLK) {	/* block till record avail */
			msblkd = 1;
			while (msblkd)
				(void) sleep((caddr_t)&msblkd, TTIPRI);
			goto ok;
		}
		spl0();
		goto fail;
	}

	splms();				/* so msintr doesnt screw up que ptrs */
ok:
	if (lbolt - mtim[mqlo-mqbuf] >= mparm.mp_fdlay) {
		if (mparm.mp_fdlay)
			msintr(M_FLUSH);
	}
	rp = (mem_t *)bp->b_un.b_addr;	/* start of receiver buffer */
	for (i=bp->b_bcount; i>0 && mqlo != mqhi; i -= 4) {
		*rp++ = *mqlo++;
		if (mqlo >= mqbuf + QUELEN) mqlo = mqbuf;
	}
	bp->b_resid = i;
	mqlen -= (bp->b_bcount - i) >> 2;
	spl0();
	iodone(bp);
	return;
}

struct buf rmsbuf;			/* Buffer for raw input-output */
msread(dev)
{
	if (msproc != u.u_procp) {
		u.u_error = EIO;
		return;
	}
	rmsbuf.b_flags &= ~B_BUSY;
	physio(msstrategy, &rmsbuf, dev, B_READ);
}

msintr(kind)
{
	register struct msrecord *mp;
	union {
		struct msrecord mf_rec;
		unsigned long mf_long;
	} msr;

	if (!mouseopen || mparm.mp_irate == 0) {/* mouse not active */
		return;
	}

	if (ms_plg) {
		mqlo = mqhi = mqbuf;
		mqlen = 0;
		return;
	}

	msr.mf_long = 0;		/* clear new record */
	mp = &msr.mf_rec;		/* get pointer to record structure */

	switch (kind) {			/* kind of interrupt */
	case M_FLUSH:			/* auto flush (called from msintr,msstrategy) */
		mp->mr_but = 1;		
	case M_PLUG:			/* automatic reset (called from kbintr) */
		mp->mr_reset = 1;
		mqlo = mqhi = mqbuf;
		mqlen = 1;
		mtim[0] = lbolt; 
		*mqhi++ = msr.mf_long;
		return;
	case M_BUT:			/* mouse button changed (called from kbintr) */
		if ((mparm.mp_flags & MF_BUT) == 0)
			return;
		break;
	case M_CTL:			/* control key changed (called from kbintr) */
		if ((mparm.mp_flags & MF_CTL) == 0)
			return;
		break;
	case M_SFT:			/* shift key changed (called from kbintr) */
		if ((mparm.mp_flags & MF_SFT) == 0)
			return;
		break;
	}

	if (mqlen) {			/* check for invalid queue data */
		if (lbolt - mtim[mqlo-mqbuf] >= mparm.mp_fdlay) {
			if (mparm.mp_fdlay)
				msintr(M_FLUSH);
		}
		if (mqlen >= QUELEN)
			return;
	} else {			/* queue was empty */
		if (mparm.mp_flags & MF_SIG)
			psignal(msproc, SIGMOUS);
	}

	mp->mr_but = ms_btn ? 1 : 0;
	mp->mr_ctl = kb_ctrl ? 1 : 0;
	mp->mr_sft = kb_shft ? 1 : 0;
	mp->mr_time = lbolt & 0xFFF;
	if (kind == M_MOVE) {		/* called from kbintr */
		mp->mr_row = ms_row;
		mp->mr_col = ms_col;
	}
	mtim[mqhi - mqbuf] = lbolt;
	*mqhi++ = msr.mf_long;
	mqlen++;
	if (mqhi >= mqbuf + QUELEN)
		mqhi = mqbuf;
	if (msblkd) {
		msblkd = 0;
		wakeup((caddr_t)&msblkd);
	}
}

/* ARGSUSED */
msioctl(dev, cmd, addr, flag)
dev_t dev;
int cmd;
caddr_t addr;
int flag;
{
	register struct msparms *mp = &mparm;
	int dlay, rate;

	switch (cmd) {
	case AL_GMOUSE:
		if (mp->mp_qlen = mqlen) {
			mp->mp_otime = lbolt - mtim[mqlo-mqbuf];
			mp->mp_ytime = lbolt - mtim[mqhi-mqbuf-1];
		} else {
			mp->mp_otime = 0;
			mp->mp_ytime = 0;
		}
		if (copyout((char *)mp, addr, sizeof(struct msparms)))
			u.u_error = EFAULT;
		break;
	case AL_SMOUSE:
		dlay = mp->mp_fdlay;
		rate = mp->mp_irate;
		if (copyin(addr, (char *)mp, sizeof(struct msparms)))
			u.u_error = EFAULT;
		else {
			if (mp->mp_fdlay) {
				if (mp->mp_fdlay < 20) {
					splms();
					mp->mp_fdlay = dlay;	/* restore */
					mqlen = 0;
					mqhi = mqlo = mqbuf;
					spl0();
				} else if (mp->mp_fdlay > 4095)
					mp->mp_fdlay = 4095;
			}
			if (mp->mp_irate != rate) {	/* did int rate chg */
				mp->mp_irate = (mp->mp_irate + 3) & ~3;
				if (mp->mp_irate > 28)
					mp->mp_irate = 28;
				splms();
				if (mp->mp_irate)
					l2copscmd((char)((MOUSEON&~7)|(mp->mp_irate>>2)));
				else
					l2copscmd(MOUSEOFF);
				spl0();
			}
			splms();
			if (mp->mp_flags & MF_VRT)
				msvrmsk = mp->mp_flags & MF_VRATE;
			spl0();
		}
		break;
	default:
		u.u_error = ENOTTY;
	}
}
