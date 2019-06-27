/*
 * Copyright 1982 UniSoft Corporation
 *
 * Speaker Driver
 * Used to operate the lisa speaker.
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
#include "sys/callo.h"
#include "sys/ttold.h"
#include "setjmp.h"
#include "sys/mmu.h"
#include "sys/cops.h"
#include "sys/local.h"
#include "sys/speaker.h"
#include "sys/l2.h"

typedef unsigned long u_long;
u_long sktrap;			/* flag and slot to calculate spkr delay */

int sk_open;			/* active flag */

#define SKPRI (PZERO+8)
	
skopen(dev, flag)
dev_t dev;
{
	if (dev != 0) {		/* minor device number is wrong */
		u.u_error = ENXIO;
		return;
	}
	if (flag == 1) {	/* open for reading ?? */
		u.u_error = EINVAL;
		return;
	}
	if (sk_open++ > 0) {	/* already opened */
		u.u_error = EBUSY;
		return;
	}
}

/* ARGSUSED */
skclose(dev, flag)
{
	if (sk_open <= 0)
		u.u_error = EINVAL;
	else
		sk_open = 0;
}

/* ARGSUSED */
skwrite(dev)
{
	struct speaker spkr;

	while (u.u_count >= sizeof(spkr)) {
		if (copyin(u.u_base, (caddr_t)&spkr, sizeof(spkr))) {
			u.u_error = EFAULT;
			return;
		}
		u.u_base += sizeof(spkr);
		u.u_count -= sizeof(spkr);
		SPL2();
		while (sktrap)
			(void) sleep((caddr_t)&sktrap, SKPRI);
		COPSADDR->e_irb = (COPSADDR->e_irb & 0xF1) | ((spkr.sk_volume&7) << 1);
		sksound(spkr.sk_wavlen&MAXWLEN, spkr.sk_duration);
		SPL0();
	}
}

/* Produce a sound on the speaker at wavelength w microseconds
 * for duration d clock ticks
 */
sksound(w, d)
register unsigned w, d;
{
	extern int skquiet();

	w &= MAXWLEN;			/* max wavelength */
	if (w < MINWLEN)
		w = MINWLEN;
	sktrap = d;			/* call skquiet at now + d */
	if (sktrap <= 0) sktrap = 1;	/* in case < MILLIRATE */
	timeout(skquiet, (caddr_t)0, (int)sktrap);
	sktone (w);
}

/* Start a tone at wavelength w microseconds.  Sound is produced by rapidly
 * turning on and off the speaker.  The 6522 cops chip has an output to the
 * speaker connected to a shift register.  A built in timer controls the rate
 * at which the shift register bits are output to the speaker.
 */
sktone(w)
register unsigned w;
{
	register struct device_e *p = COPSADDR;
	register int cmd = 0x55;		/* sk shift reg */

	w = w >> 3;		/* wavelength resolution is 8 microseconds */
	if (w > 0xFF) {
		cmd = 0x33;
		w = w >> 1;
		if (w > 0xFF) {
			cmd = 0xF;
			w = w >> 1;
		}
	}

	p->e_acr |= 0x10;	/* enable */
	p->e_t2cl = w;		/* set timer */
	p->e_sr = cmd;		/* set output pattern */
}

skquiet()
{
	struct device_e *p = COPSADDR;
	p->e_acr &= 0xE3;
	sktrap = 0;
	wakeup((caddr_t)&sktrap);
}

beep()
{
	int sp;
	if (sktrap) return;
	sp = spl2();
	COPSADDR->e_irb = (COPSADDR->e_irb & 0xF1) | (l2_bvol << 1);
	sksound((unsigned)l2_bpitch, (unsigned)l2_btime);
	splx(sp);
}

#ifdef notdef
/*
 * Produce a short click to implement keyboard clicking
 */
click()
{
	register struct device_e *p = COPSADDR;
	int i = 20;
	int sp;

	if (sktrap) return;
	sp = spl2();
	p->e_acr |= 0x10;
	p->e_t2cl = 0x10;
	p->e_sr = 0x55;
	while (--i != -1) ;
	p->e_acr &= 0xE3;
	splx(sp);
}
#endif
