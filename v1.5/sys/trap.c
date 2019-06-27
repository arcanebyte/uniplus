/*#define HOWFAR*/
/*#define SYSCALLS  */

/* @(#)trap.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/trap.h"
#ifdef mc68881
#include "sys/fptrap.h"
#endif mc68881
#include "sys/seg.h"
#include "sys/sysinfo.h"
#ifndef VIRTUAL451
#include "sys/buserr.h"
#endif

#define	EBIT	1		/* user error bit in PS: C-bit */
#define	USER	0x1000		/* user-mode flag added to number */
#define NSYSENT	128

#ifdef SYSCALLS
static char reserved[] = "reserved";

char *callnames[] = {
	/*  0 */	"indir", "exit", "fork", "read",
	/*  4 */	"write", "open", "close", "wait",
	/*  8 */	"creat", "link", "unlink", "exec",
	/* 12 */	"chdir", "time", "mknod", "chmod",
	/* 16 */	"chown", "break", "stat", "seek",
	/* 20 */	"getpid", "mount", "umount", "setuid",
	/* 24 */	"getuid", "stime", "ptrace", "alarm",
	/* 28 */	"fstat", "pause", "utime", "stty",
	/* 32 */	"gtty", "access", "nice", "sleep",
	/* 36 */	"sync", "kill", "csw", "setpgrp",
	/* 40 */	"tell", "dup", "pipe", "times",
	/* 44 */	"prof", "lock", "setgid", "getgid",
	/* 48 */	"sig", "msgsys", reserved, "acct",
	/* 52 */	"shmsys", "semsys", "ioctl", "phys",
	/* 56 */	"locking", "utssys", reserved, "exece",
	/* 60 */	"umask", "chroot", "fcntl", "ulimit",

	/* 64 */	"reboot", reserved, reserved, reserved,
#ifdef UCB_NET
	/* 68 */	reserved, reserved, "select", "gethostname",
	/* 72 */	"sethostname", "socket", "accept", "connect",
	/* 76 */	"receive", "send", "socketaddr", "netreset",
#else
	/* 68 */	reserved, reserved, reserved, reserved,
	/* 72 */	reserved, reserved, reserved, reserved,
	/* 76 */	reserved, reserved, reserved, reserved,
#endif
	/* 80 */	reserved, reserved, reserved, reserved,
	/* 84 */	reserved, reserved, reserved, reserved,
	/* 88 */	reserved, reserved, reserved, reserved,
	/* 92 */	reserved, reserved, reserved, reserved,
	/* 96 */	reserved, reserved, reserved, reserved,
	/* 100 */	reserved, reserved, reserved, reserved,
	/* 104 */	reserved, reserved, reserved, reserved,
	/* 108 */	reserved, reserved, reserved, reserved,
	/* 112 */	reserved, reserved, reserved, reserved,
	/* 116 */	reserved, reserved, reserved, reserved,
	/* 120 */	reserved, reserved, reserved, reserved,
	/* 124 */	reserved, reserved, reserved, reserved
};
#endif

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[8+8+1+1] = {
	R0, R1, R2, R3, R4, R5, R6, R7,
	AR0, AR1, AR2, AR3, AR4, AR5, AR6, SP, PC,
	RPS
};

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(number, regs)
short number;
{
	register struct user *up;
	extern int parityno;
	register i;
	time_t syst;
	int retval;
	int *oldar0;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	up = &u;
	retval = 0;
	syst = up->u_stime;
#ifdef FLOAT		/* sky floating point board */
	up->u_fpsaved = 0;
#endif
	oldar0 = up->u_ar0;
	up->u_ar0 = &regs;
	if (USERMODE(up->u_ar0[RPS]))
		number |= USER;
#ifdef HOWFAR
	if (number != RESCHED && number != RESCHED+USER) {
		printf("trap number=0x%x ps=0x%x\n", number,
			up->u_ar0[RPS]&0xFFFF);
		showregs(1);
	}
#endif
	/*
	 * Handle parity specially to make it processor independent
	 */
	if (number==parityno || number==(parityno|USER)) {
		if ((i = parityerror()) == 0) {
			logparity((paddr_t)&up->u_ar0[PC]);
			goto out;
		}
		if (i > 0) {
			number = i | (number & USER);
			goto sw;
		}
		if (number & USER) {
			logparity((paddr_t)&up->u_ar0[PC]);
			i = SIGBUS;
		} else {
			if (nofault) {
				up->u_ar0 = oldar0;
				longjmp(nofault, -1);
			}
			showbus();
			panic("kernel parity error");
		}
	} else {
sw:
	switch(number) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	default:
		if ((number & USER) == 0) {
			panicstr = "trap";	/* fake it for printfs */
			printf("\ntrap type %d\n", number);
			showregs(1);
			panic("unexpected kernel trap");
		}

	case CHK + USER:	/* CHK instruction */
	case TRAPV + USER:	/* TRAPV instruction */
	case PRIVVIO + USER:	/* Priviledge violation */
	case L1010 + USER:	/* Line 1010 emulator */
	case L1111 + USER:	/* Line 1111 emulator */
	case TRAP4 + USER:
	case TRAP5 + USER:
	case TRAP6 + USER:
	case TRAP7 + USER:
	case TRAP8 + USER:
	case TRAP9 + USER:
	case TRAP10 + USER:
	case TRAP11 + USER:
	case TRAP12 + USER:
	case TRAP13 + USER:
	case TRAP14 + USER:
	case TRAP15 + USER:
	case ILLINST + USER:	/* illegal instruction */
		i = SIGILL;
		break;

	case DIVZERO + USER:	/* zero divide */
		i = SIGFPE;
		break;

#ifdef mc68881		/* MC68881 floating-point coprocessor */
	case FPBSUN + USER:	/* Branch or Set on Unordered Condition */
	case FPINEX + USER:	/* Inexact Result */
	case FPDZ + USER:	/* Floating Point Divide by Zero */
	case FPUNFL + USER:	/* Underflow */
	case FPOPERR + USER:	/* Operand Error */
	case FPOVFL + USER:	/* Overflow */
	case FPSNAN + USER:	/* Signalling NAN (Not-A-Number) */
		up->u_fpexc = number & 0xFF;
		i = SIGFPE;
		break;
#endif mc68881

	case TRCTRAP:		/* trace out of kernel mode - */
		up->u_ar0 = oldar0;
		return(retval);	/* this is happens when a trap instruction */
		  	 	/* is executed with the trace bit set */

	case TRCTRAP + USER:	/* trace */
	case TRAP1 + USER:	/* bpt - trap #1 */
		i = SIGTRAP;
		up->u_ar0[RPS] &= ~PS_T;
		break;

	case TRAP2 + USER:	/* iot - trap #2 */
		i = SIGIOT;
		break;

	case TRAP3 + USER:	/* emt - trap #3 */
		i = SIGEMT;
		break;

	case SYSCALL + USER:	/* sys call - trap #0 */
		panic("syscall\n");

	/*
	 * Allow process switch
	 */
	case RESCHED + USER:
	case RESCHED:
		qswtch();
		goto out;

#ifdef VIRTUAL451
	/*
	 * If the user SP is below the stack segment
	 * and within STACKGAP clicks of the bottom
	 * of the stack segment, then grow the
	 * stack automatically.
	 */
	case ADDRERR + USER:	 /* bus error - address error */
		i = SIGBUS;
		retval = 1;
		trapaddr((struct buserr *)&regs);
		break;

	case BUSERR + USER:	/* memory management error - bus error */
		if(pagein((int)(((struct buserr *)&regs)->ber_faddr))) {
			up->u_ar0 = oldar0;
			return(0);
		}
		i = SIGSEGV;
		retval = 1;
		trapaddr((struct buserr *)&regs);
		break;

	case ADDRERR:	/* bus error - address error */
		trapaddr((struct buserr *)&regs);
		printf("kernel address error\n");
		showbus();
		panic("kernel memory management error");

	case BUSERR:	/* memory management error - bus error in kernel */
		if (nofault) {
			up->u_ar0 = oldar0;
			longjmp(nofault, -1);
		}
		trapaddr((struct buserr *)&regs);
		berdump((struct buserr *)&regs);
		printf("kernel bus error\n");
		showbus();
		panic("kernel memory management error");
#else

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 68000 this is not the case and
	 * the routine machdep/backup() will fail.
	 */

	case ADDRERR + USER:	 /* bus error - address error */
		i = SIGBUS;
		retval = 1;
		trapaddr((struct buserr *)&regs);
		break;

	case BUSERR + USER:	/* memory management error - bus error */
		if (i = backup(up->u_ar0))
		    if (grow((unsigned)(up->u_ar0[SP]+i)))
			goto out;
		i = SIGSEGV;
		retval = 1;
		trapaddr((struct buserr *)&regs);
		break;

	case ADDRERR:	/* kernel address error */
	case BUSERR:	/* kernel bus error */
		if (nofault)
			longjmp(nofault, -1);
		trapaddr((struct buserr *)&regs);
		showbus();
		panic("kernel memory management error\n");
#endif

	/*
	 * Unused trap vectors generate this trap type.
	 * Reciept of this trap is a
	 * symptom of hardware problems and may
	 * represent a real interrupt that got
	 * sent to the wrong place.  Watch out
	 * for hangs on disk completion if this message appears.
	 */
	case SPURINT:
	case SPURINT + USER:
		printf("\nRandom interrupt ignored\n");
		up->u_ar0 = oldar0;
		return(retval);
	}
	}	/* end else ...			*/
	psignal(up->u_procp, i);

out:
	if (up->u_procp->p_sig && issig())
		psig();
	if (up->u_prof.pr_scale)
		addupc((unsigned)up->u_ar0[PC], &up->u_prof, (int)(up->u_stime-syst));
#ifdef FLOAT		/* sky floating point board */
	if (up->u_fpinuse && up->u_fpsaved)
		restfp();
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	if (fp881)
		fprest();
#endif mc68881
	up->u_ar0 = oldar0;
	return(retval);
}

/*
 * process a system call
 */
syscall(regs)
{
	register struct proc *pp;
	register struct user *up;
	register *regp, *argp;
	register i;
	int *oldar0;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	up = &u;
	sysinfo.syscall++;
	up->u_error = 0;
#ifdef FLOAT		/* sky floating point board */
	up->u_fpsaved = 0;
#endif
	oldar0 = up->u_ar0;
	up->u_ar0 = regp = &regs;
	up->u_ap = argp = up->u_arg;
	i = regp[R0] & 0377;
	if (i >= NSYSENT)
		i = 0;
	argp[0] = regp[AR0];
	argp[1] = regp[R1];
	argp[2] = regp[AR1];
	argp[3] = regp[R2];
	argp[4] = regp[AR2];
	argp[5] = regp[R3];

#ifdef SYSCALLS
	printf("***** %s: %x %x %x %x\n", callnames[i],
		argp[0], argp[1], argp[2], argp[3], argp[4], argp[5]);
#endif
	up->u_dirp = (caddr_t)argp[0];
	up->u_rval1 = 0;
	up->u_rval2 = regp[R1];
	if (qsave(up->u_qsav)) {
		/*
		 * restore registers not saved by qsave
		 */
		up = &u;
		regp = &regs;
		argp = up->u_arg;
		if (up->u_error==0)
			up->u_error = EINTR;
	} else {
		(*(sysent[i].sy_call))();
	}
	if (up->u_error) {
		regp[R0] = up->u_error;
		regp[RPS] |= PS_C;		/* carry bit */
		if (++up->u_errcnt > 16) {
			up->u_errcnt = 0;
			runrun++;
		}
#ifdef SYSCALLS
		printf("      syscall error = %d, pc = 0x%x\n",
			up->u_error, regp[PC]);
#endif
	} else {
#ifdef SYSCALLS
		printf("      syscall success, pc = 0x%x\n",
			regp[PC]);
#endif
		regp[RPS] &= ~PS_C;		/* carry bit */
		regp[R0] = up->u_rval1;
		regp[R1] = up->u_rval2;
	}
	pp = up->u_procp;
	/*
	 * Test if the trap instruction was executed with the
	 * trace bit set (the trace trap was already ignored)
	 * and set the trace signal to avoid missing the trace
	 * on the trap instruction.
	 */
	pp->p_pri = (pp->p_cpu>>1) + (PUSER - NZERO) + pp->p_nice;
	curpri = pp->p_pri;
	if (pp->p_sig && issig())
		psig();
	up->u_ar0 = oldar0;
	if (runrun)
		qswtch();
#ifdef FLOAT		/* sky floating point board */
	if (up->u_fpinuse && up->u_fpsaved)
		restfp();
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	if (fp881)
		fprest();
#endif mc68881
}

/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

/*
 * Ignored system call
 */
nullsys()
{
}

stray(addr)
physadr addr;
{
	logstray(addr);
	printf("stray interrupt at %x\n", addr);
}

/*
 * trapaddr - Save the info from a 68010 bus or address error.
 */
trapaddr(ap)
register struct buserr *ap;
{
	extern int cputype;

	if (cputype == 0)
		return;
	u.u_fcode = ap->ber_sstat;
	u.u_aaddr = ap->ber_faddr;
	u.u_ireg = ap->ber_iib;
}

/*
 * showbus - print out status on mmgt error
 */
showbus()
{
	register struct user *up;

	up = &u;
	printf("vaddr = 0x%x paddr = 0x%x ireg = 0x%x fcode = 0x%x\n",
		up->u_aaddr, vtop((caddr_t)up->u_aaddr), up->u_ireg&0xFFFF,
		up->u_fcode&0xF);
	showregs(1);
}

/*
 * Show a processes registers
 */
showregs(mmuflg)
int mmuflg;
{
	register struct user *up;
	register int i, j;
	char command[DIRSIZ+1];

	up = &u;
#ifdef HOWFAR
	if (mmuflg)
		dumpmm(-1);
#endif HOWFAR
#ifdef lint
	dumpmm(mmuflg);
#endif lint
	for (i=0; i<DIRSIZ; i++) {
		j = up->u_comm[i];
		if (j<=' ' || j>=0x7F)
			break;
		command[i] = j;
	}
	command[i] = 0;
	/*
	 * separate prints in case up or u_procp trashed
	 */
	printf("pc = 0x%x sr = 0x%x up->u_procp = 0x%x",
		up->u_ar0[PC], up->u_ar0[RPS]&0xFFFF, up->u_procp);
	printf(" pid = %d exec = '%s'\n", up->u_procp->p_pid, command);
	for (i = 0; i < 16; i++) {
		printf("0x%x ", up->u_ar0[i]);
		if (i == 7 || i == 15) printf("\n");
	}
}

#ifdef DUMPSTK
#include <sys/var.h>
#ifndef VIRTUAL451
#include <sys/mmu.h>
#endif
#include <sys/sysmacros.h>
/*
 * dump the present contents of the stack
 */
dumpstack(ret)
{
	register unsigned short *ip;

	ip = (unsigned short *)&ret;
	ip -= 4;
	printf("\n%x  ", ip);
	while (ip < (unsigned short *)((int)&u+ctob(v.v_usize))) {
		if (((int)ip & 31) == 0)
			printf("\n%x  ", ip);
		printf(" %x", *ip++);
	}
	printf("\n");
	if (ret != 0)
		panic("**** ABORTING ****");
}

/*
 * dump the present contents of the user stack
 */
dumpustack(max)
unsigned max;
{
	register unsigned short *ip, *jp;
	register unsigned i, n;

	ip = (unsigned short *)(u.u_ar0[SP] - 20);
	jp = ip + 64;
	if (jp < (unsigned short *)max)
		jp = (unsigned short *)max;
	if (jp > (unsigned short *)v.v_uend)
		jp = (unsigned short *)v.v_uend;
	printf("\n%x  ", ip);
	while (ip < jp) {
		i = (fuword(ip++) >> 16) & 0xFFFF;
		if (((int)ip & 31) == 0)
			printf("\n%x  ", ip);
		printf(" %x", i);
	}
	printf("\n");
}
#endif
