#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/tty.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/utsname.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/iobuf.h"
#include "sys/err.h"
#include "setjmp.h"

/*
 * Definitions of kernel variables to keep lint happy
 */
int tstb;
struct user u;
caddr_t end;
int dispatch[], pmvect[], refresh();
extern struct ttyptr *tty_stat[];
extern dev_t dumpdev;
extern int physmem;
extern int dump_addr;
extern int (*dump)();
extern int (*pwr_clr[])();
extern int (*dev_init[])();

#ifdef UCB_NET
extern char netstak[];
extern char *svstak;
#endif

/*
 * Use selected system variables
 */
idle()
{
	dumpdev = 0;
	/* termcnt = 0; */
	(*dump)();
	(*pwr_clr[0])();
	(*dev_init[0])();
	dump_addr = 0;
	tty_stat[0] = 0;
	physmem = 0;
	mmuinit((caddr_t)0);
	busaddr();
	clock((struct args *)0);
	nullsys();
	stray((physadr)0);
	syscall((int)0);
	trap((short)0, 0);
	ttioctl((struct tty *)0, 0, 0, 0);
	disksort((struct iobuf *)0, (struct buf *)0);
	deverror((struct buf *)0, 0, 0);
	logstray((physadr)0);
	(void) physck((daddr_t)0, 0);
	(void) iocheck((caddr_t)0);
	(void) passc(0);
	(void) cpass();
	(void) min(0, 0);
	(void) max(0, 0);
	(void) getmajor(idle);
	prcom(idle, (struct buf *)0, 0, 0);
	logberr((struct iobuf *)0, 0);
	fmtberr((struct iobuf *)0, (unsigned)0, (unsigned)0, (unsigned)0,
		(unsigned)0, (long)0, (struct deverreg *)0);
#ifdef UCB_NET
	netstak[0] = 0;
	svstak = (char *)0;
#endif
}

/* ARGSUSED */
save(lp) label_t lp; { return(0); }

/* ARGSUSED */
qsave(lp) label_t lp; { return(0); }

/* ARGSUSED */
resume(uaddr, lp) int uaddr; label_t lp; { }

/* ARGSUSED */
blt(to, from, ct) caddr_t to, from; int ct; { }

/* ARGSUSED */
clear(to, ct) caddr_t to; int ct; { }
