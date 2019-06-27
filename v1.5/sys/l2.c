/* #define HOWFAR */
/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Level 2 special functions handler
 * Used to drive keyboard, speaker, mouse, real time clock, screen contrast,
 * and soft on/off circuitry.
 * All processing is done through the Keyboard SY6522 (@addr DC00) with the
 * exception of the screen contrast which is sent through the Hard disk 6522.
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
#include "sys/mmu.h"
#include "sys/cops.h"
#include "sys/local.h"
#include "sys/l2.h"
#include "sys/kb.h"
#include "sys/sony.h"

#define DIMSCREEN

/* The contrast control latch is hanging off the Parallel Port VIA but
 * is only changed in response to a level2 timer interrupt. Syncronization
 * with the hard disk is necessary since they both use the parallel port data
 * bus.  This is done by having the disk driver maintain a variable called
 * `ppinuse'.  When this variable is zero we can ramp the console contrast.
 * Otherwise we set ppcontchg and the disk driver will call l2ramp as soon
 * as it's convenient.
 */
#include <sys/pport.h>
extern ppinuse;			/* flag preventing us from setting contrast */

/*
 * Setup Keyboard 6522 VIA appropriately.  This is mostly magic dependant on
 * how the chip is connected to its environment in hardware.  This procedure
 * is only called once to initialize the interface.
 */
l2init()
{
	register struct device_e *p = COPSADDR;
	register char romid;
	extern int l2dim();

	l2_dtime = 0x4650;
	l2_crate = 1;
	l2_dtrap = l2_dtime;
	l2_dimcont = (0x32 << 2);
	l2_desired = (ONCONT << 2);
	l2_defcont = l2_desired & 0xFC;
	l2_contrast = (ONCONT << 2);		/* power on contrast */
	timeout(l2dim, (caddr_t)1, 1);
	l2_bvol = 4;
	l2_bpitch = 1000;
	l2_btime = 6;
	kb_repdlay = 0x20;
	kb_repwait = 0x30;

	p->e_ddra = 0;			/* bytewide bus connecting to I/O COPS*/
	p->e_ddrb = 0x2E;		/* PP parity and speaker are outputs */
					/* PP reset, CRDY, FDIR, & KBIN: input*/
	p->e_irb = l2_bvol << 1;	/* init moderate speaker volumn */
	p->e_pcr = 0xA9;		/* Pulse speaker output, handshake */
					/* COPS line, and enable leading edge*/
	p->e_acr = 0x41;		/* Make Timer1 continous (i.e. self- */
					/* reset), enable latching of COPS bus*/
	p->e_ier = 0x7F;		/* disable all interrupts */
	
	romid = SNIOB->rom_id & ROMMASK;
	if ((romid == (ROMTW|ROMSLOW)) || ((romid&ROMTW)==0)) {	/* slow timer */
		p->e_t1cl = 0xCA;
		p->e_t1ch = 0x27;
		p->e_t1ll = 0xCA;
		p->e_t1lh = 0x27;
	} else {	/* fast timer (includes undefined romid==E0) */
		p->e_t1cl = 0x7B;
		p->e_t1ch = 0x63;
		p->e_t1ll = 0x7B;
		p->e_t1lh = 0x63;
	}

	p->e_ier = 0xC2;		/* Enable timer1 and COPS bus intr */
	p->e_ifr = 0x7F;		/* clear pending interrupt flags */
	l2ramp(0);			/* bring to user selected contrast */
}

/* This procedure is used to send commands to the Keyboard COPS.
 *	It is VERY timing-dependent.  In particular, nothing can be changed
 *	in the part which sets the data direction to out, since this must
 *	happen WHILE CRDY drops to the low state.  Interrupts are enabled while
 *	waiting for this because the line can stay high for as long as 2 ms,
 *	although it's typically about 800 microseconds while the mouse is in
 *	use.  It only stays in the low state for about 4 microseconds before
 *	returning to "not ready".
 */
l2copscmd(c)
register char c;	/* d7 */
{
	register struct device_e *p = COPSADDR;	/* a5 */
	register char *ddra = &p->e_ddra;	/* a4 */
	register unsigned short dir = 0xFFFF;	/* d6 data direction (out) */
	register short crdy = 6;		/* d5 CRDY bit for btst */
	register char i;			/* d4 */
	int pl;

try:	pl = spl7();
	p->e_aira = c;			/* load cmd into IORA (no handshake) */
	asm("	btst	d5,a5@(1)");	/* wait 'til not ready */
	asm("	beq	.L1");		/* if already low we're too late */
#define	chkrdy()	asm("	btst	d5,a5@(1)"); \
			asm("	beq	.L2")
	chkrdy();			/* as soon as it's ready (low), go to send */
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();
	chkrdy();			/* if still high give up, try again later */
	asm(".L1:");			/* again: */
	splx(pl);			/* restore intrs cause this may take a while */
	goto try;
send:	asm(".L2:");			/* send: */
	*ddra = dir;			/* switch dir 68K -> cops (send command) */
	i = 0x10;
	do i--; while(i>0);		/* wait a bit */
	dir = 0;			/* reset dir to cops -> 68K */
	*ddra = dir;
	splx(pl);
#ifdef lint
	if (crdy) goto send;
#endif lint
}

/* ARGSUSED */
l2dim(flg)
{
	extern time_t lbolt;			/* time in ticks */

#ifdef DIMSCREEN
	if (l2_dimmed == 0) {			/* not dimmed already */
		if (l2_dtrap > lbolt) {
			timeout(l2dim, (caddr_t)0, (int)(l2_dtrap - lbolt));
			return;
		}
		l2_desired = l2_dimcont;
		l2_dimmed = 1;
		l2ramp(0);
	}
#ifdef HOWFAR
	else printf("\nWHAT!! -- l2dim called while screen was dim\n");
#endif HOWFAR
#endif DIMSCREEN
}

l2undim()
{
#ifdef DIMSCREEN
	if (l2_dimmed != 0) {
		l2_dtrap = lbolt + l2_dtime;
		timeout(l2dim, (caddr_t)0, l2_dtime);
		l2_desired = l2_defcont;
		l2_dimmed = 0;
		l2ramp(0);
	}
#ifdef HOWFAR
	else printf("\nWHAT!! -- l2undim called while screen was bright\n");
#endif HOWFAR
#endif DIMSCREEN
}

l2ramp(tflag)
{
	register struct device_d *pp;	/* a5 */
	register int c;			/* d7 */
	int pl;

	switch (tflag) {		/* who called us */
	case 0:				/* somebody starting a ramp */
		if (l2_rcflag)
			return;
		break;
	case 1:				/* timeout (continue ramp) */
		l2_rcflag = 0;
		break;
	case 2:				/* profile driver (kludge) */
		c = l2_desired;		/* contrast desired */
		if (c == l2_contrast)	/* finished */
			return;
		goto skip;
	}

	c = l2_desired;			/* contrast desired */
	if (c == l2_contrast)		/* finished */
		return;

	if (ppinuse) {			/* port busy, try again in 50 ms */
		l2_rcflag = 1;
		timeout(l2ramp, (caddr_t)1, 1);	/* try again next clock tick */
		return;
	}

	if (c < l2_contrast) l2_contrast -= 4; else l2_contrast += 4;
	c = l2_contrast;
	l2_rcflag = 1;
	timeout(l2ramp, (caddr_t)1, l2_crate);
skip:

	pl = spl7();
	pp = PPADDR;
#ifdef lint
	c = (int)pp;
#endif
	asm(" bset	#2,a5@(0x11) ");
	asm(" bset	#2,a5@(1) ");
	asm(" movb	#0xff,a5@(0x19) ");
	asm(" movb	d7,a5@(9) ");
	asm(" bset	#7,a5@(0x11) ");
	asm(" bclr	#7,a5@(1) ");
	asm(" bset	#7,a5@(1) ");
	asm(" bclr	#7,a5@(0x11) ");
	asm(" bclr	#2,a5@(1) ");
	splx(pl);
	return;
}
