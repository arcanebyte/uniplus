/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Interrupt handler for level 1 interrupts.
 *	parallel port 0, sony, and video circuit controller
 *
 * Since the Parallel Port hard disk interrupt comes in at this level
 * as well as the Sony and vertical retrace (clock) we have included
 * this so we may only call the actual interrupt routine when really
 * necessary.
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
#include "setjmp.h"
#include "sys/mmu.h"
#include <sys/pport.h>
#include <sys/cops.h>
#include <sys/mouse.h>

unsigned char msvrcnt;
extern struct msparms mparm;
extern struct proc *msproc;
extern char msblkd;
extern char msvrmsk;

l1intr(ap)
struct args *ap;
{
	unsigned char a;
	register struct device_d *devp = PPADDR;
	register struct device_e *f = COPSADDR;

	a = devp->d_ifr;		/* read intr flag register */
	if (a & (FCA1 | FTIMER1)) {
		ap->a_dev = 0;		/* set to port number instead of clock value */
		ppintr(ap);		/* built in parallel port */
	} else {
		if ((f->e_irb & FDIR) != 0)
			snintr();	/* sony interrupt */
		else {			/* verticle retrace interrupt */
			if ((mparm.mp_flags & MF_VRT) != 0) {
				VROFF = 1;
				if (msvrcnt++ >= msvrmsk) {
					msvrcnt = 0;
					if (msblkd) {
						msblkd = 0;
						wakeup((caddr_t)&msblkd);
					}
					psignal(msproc,SIGMOUS);
				}
				VRON = 1;
			} else {
				do {
					VROFF = 1;
					VRON = 1;
				} while ((STATUS & S_VR) == 0);
			}
			clock(ap);
		}
	}
}
