/*
 *	mem, kmem and null devices.
 *
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/RATHOLE
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "setjmp.h"

/*
 * mmread - read mem, kmem or null.
 */
mmread(dev)
{
	if (minor(dev) != 2)		/* not /dev/null */
		mmmove(dev, B_READ);
}

/*
 * mmwrite - write mem, kmem or null.
 */
mmwrite(dev)
{
	if (minor(dev) == 2) {		/* /dev/null, just gobble chars */
		u.u_count = 0;
		return;
	}
	mmmove(dev, B_WRITE);
}

/*
 * mmmove - common routine for mmread and mmwrite
 */
mmmove(dev, flag)
dev_t dev;
{
	register int pageoffs, count, prot;
	jmp_buf jb;
	int *saved_jb;
	int s;

	if (minor(dev)==1 || minor(dev)==0) {	/* kmem, mem */
	    do {
		s = spl7();
		SEG1_0 = 1;	/* system context */
		/* SEG2_0 = 1;	/* system context */
		prot = getmmu((short *)(vtoseg(u.u_offset) | ACCLIM))&PROTMASK;
		SEG1_1 = 1;	/* user context */
		/* SEG2_0 = 1;	/* user context */
		splx(s);
/* printf("u_base=0x%x offset=0x%x prot=0x%x\n", u.u_base, u.u_offset, prot); */
		if ((unsigned)u.u_offset < (unsigned)STDIO && prot != ASRW)
			goto bad;
		pageoffs = u.u_offset & (ctob(1)-1);
		count = min((unsigned)(ctob(1) - pageoffs), u.u_count);
/* printf("pageoffs=%d count=%d u.u_count=%d\n", pageoffs, count, u.u_count); */
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			u.u_segflg = 0;
			iomove((caddr_t)u.u_offset, count, flag);
		} else
			u.u_error = ENXIO;
		u.u_segflg = 0;
		nofault = saved_jb;
	    } while(u.u_error == 0 && u.u_count);
	    return;
	}
bad:
	u.u_error = ENXIO;
}
