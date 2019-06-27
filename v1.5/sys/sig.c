/* @(#)sig.c	1.3 */
#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/reg.h"
#include "sys/text.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/psl.h"
#include "sys/scat.h"

/*
 * Priority for tracing
 */
#define	IPCPRI	PZERO

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
struct ipctrace
{
	int	ip_data;
	int	ip_lock;
	int	ip_req;
	int	*ip_addr;
} ipc;

/*
 * Send the specified signal to
 * all processes with 'pgrp' as
 * process group.
 * Called by tty.c for quits and
 * interrupts.
 */
signal(pgrp, sig)
register pgrp;
{
	register struct proc *p;

	if (pgrp == 0)
		return;
	for(p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
		if (p->p_pgrp == pgrp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
register struct proc *p;
register sig;
{

	sig--;
	if (sig < 0 || sig >= NSIG)
		return;
	if (((p->p_sigign >> sig) & 1) && sig != (SIGCLD - 1))
		return;
	p->p_sig |= 1L<<sig;
	if (p->p_stat == SSLEEP && p->p_pri > PZERO) {
		if (p->p_pri > PUSER)
			p->p_pri = PUSER;
		setrun(p);
	}
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
issig()
{
	register n;
	register struct proc *p, *q;

	p = u.u_procp;
	while(p->p_sig) {
		n = fsig(p);
		if (n == SIGCLD) {
			if (u.u_signal[SIGCLD-1]&01) {
				for (q = &proc[1];
				 q < (struct proc *)v.ve_proc; q++)
					if (p->p_pid == q->p_ppid &&
					 q->p_stat == SZOMB)
						freeproc(q, 0);
			} else if (u.u_signal[SIGCLD-1])
				return(n);
		} else if (n == SIGPWR) {
			if (u.u_signal[SIGPWR-1] && (u.u_signal[SIGPWR-1]&1)==0)
				return(n);
		} else if ((u.u_signal[n-1]&1) == 0 || (p->p_flag&STRC))
			return(n);
		p->p_sig &= ~(1L<<(n-1));
	}
	return(0);
}

/*
 * Enter the tracing STOP state.
 * In this state, the parent is
 * informed and the process is able to
 * receive commands from the parent.
 */
stop()
{
	register struct proc *pp, *cp;

loop:
	cp = u.u_procp;
	if (cp->p_ppid != 1)
	for (pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_pid == cp->p_ppid) {
			wakeup((caddr_t)pp);
			cp->p_stat = SSTOP;
			swtch();
			if ((cp->p_flag&STRC)==0 || procxmt())
				return;
			goto loop;
		}
	exit(fsig(u.u_procp));
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 */
psig()
{
	register n, p;
	register struct proc *rp;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	rp = u.u_procp;
#ifdef FLOAT		/* sky floating point board */
	if (u.u_fpinuse && u.u_fpsaved==0) {
		savfp();
		u.u_fpsaved = 1;
	}
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	if (fp881)
		fpsave();
#endif mc68881
	if (rp->p_flag&STRC)
		stop();
	n = fsig(rp);
	if (n==0)
		return;
	rp->p_sig &= ~(1L<<(n-1));
	if ((p=u.u_signal[n-1]) != 0) {
		if (p & 1)
			return;
		u.u_error = 0;
		if (n != SIGILL && n != SIGTRAP)
			u.u_signal[n-1] = 0;
		sendsig((caddr_t)p, n);
		return;
	}
	switch(n) {

	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
		if (core())
			n += 0200;
	}
	exit(n);
}

/*
 * find the signal in bit-position
 * representation in p_sig.
 */
fsig(p)
struct proc *p;
{
	register short i;
	register long n;

	n = p->p_sig;
	i = NSIG - 1;
	do {
		if (n & 1L)
			return(NSIG - i);
		n >>= 1;
	} while (--i != -1);
	return(0);
}

/*
 * Create a core image on the file "core"
 *
 * It writes USIZE (v.v_usize) block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
core()
{
	register struct inode *ip;
	register struct user *up;
	register s;
	extern schar();

	up = &u;
	if (up->u_uid != up->u_ruid)
		return(0);
	up->u_error = 0;
	up->u_dirp = "core";
	ip = namei(schar, 1);
	if (ip == NULL) {
		if (up->u_error)
			return(0);
		ip = maknode(0666);
		if (ip==NULL)
			return(0);
	}
	if (!access(ip, IWRITE) && (ip->i_mode&IFMT) == IFREG) {
		itrunc(ip);
		up->u_offset = 0;
		up->u_base = (caddr_t)up;
		up->u_count = ctob(v.v_usize);
		up->u_segflg = 1;
		up->u_limit = (daddr_t)ctod(MAXMEM);
		up->u_fmode = FWRITE;
		/* make register pointer relative for adb */
		up->u_ar0 = (int *)((int)up->u_ar0 - (int)up);
		up->u_usrtop = btoc(v.v_uend);
		writei(ip);
		up->u_ar0 = (int *)((int)up->u_ar0 + (int)up);
		s = up->u_procp->p_size - v.v_usize;
		(void) estabur((unsigned)0, (unsigned)s, (unsigned)0, 0, RO);
		up->u_base = (caddr_t)v.v_ustart;
		up->u_count = ctob(s);
		up->u_segflg = 0;
		writei(ip);
	} else
		up->u_error = EACCES;
	iput(ip);
	return(up->u_error==0);
}

/*
 * grow the stack to include the SP
 * true return if successful.
 */
#ifdef NONSCATLOAD
grow(sp)
unsigned sp;
{
	register struct user *up;
	register struct proc *p;
	register si, i, a1;

	up = &u;
	if ((v.v_uend-sp) < ctob(up->u_ssize))
		return(0);
	si = btoct(v.v_uend-sp) - up->u_ssize + SINCR;
	if (si <= 0)
		return(0);
	if (estabur(up->u_tsize, up->u_dsize, up->u_ssize+si, 0, RO))
		return(0);
	p = up->u_procp;
	expand((int)(p->p_size+si));
	a1 = p->p_addr + p->p_size;
	for(i=up->u_ssize; i; i--) {
		a1--;
		copyseg(a1-si, a1);
	}
	for(i=si; i; i--)
		clearseg(--a1);
	up->u_ssize += si;
	return(1);
}
#else
grow(sp)
unsigned sp;
{
	register struct user *up;
	register struct proc *p;
	register si, i, a1;
	register struct scatter *s;
	register a2, n;
	short t;

	up = &u;
	if ((v.v_uend-sp) < ctob(up->u_ssize))
		return(0);
	si = btoct(v.v_uend-sp) - up->u_ssize + SINCR;
	if (si <= 0)
		return(0);
	if (chksize(up->u_tsize, up->u_dsize, up->u_ssize+si))
		return(0);
	p = up->u_procp;
	expand((int)(p->p_size+si));
	/*
	 * locate last click of old data size
	 */
	s = scatmap;
	a1 = p->p_scat;
	n = v.v_usize + up->u_dsize;
	for (i=1; i<n; i++)
		a1 = s[a1].sc_index;
	/*
	 * locate last click of old stack
	 */
	a2 = s[a1].sc_index;
	n = up->u_ssize;
	if (n == 0)
		printf("grow:ssize not expected to be zero\n");
	for (i=1; i<n; i++)
		a2 = s[a2].sc_index;
	/*
	 * chain new clicks into stack space following data space
	 */
	t = s[a1].sc_index;
	s[a1].sc_index = s[a2].sc_index;
	n = si;
	for (i=0; i<n; i++)
		clearseg(ixtoc(a1 = s[a1].sc_index));
	if (s[a1].sc_index != SCATEND)
		printf("grow failure\n");
	s[a1].sc_index = t;
	s[a2].sc_index = SCATEND;
	p->p_flag &= ~SCONTIG;
	up->u_ssize += si;
	(void) estabur(up->u_tsize, up->u_dsize, up->u_ssize, 0, RO);
	return(1);
}
#endif

/*
 * sys-trace system call.
 */
ptrace()
{
	register struct ipctrace *ipcp;
	register struct user *up;
	register struct proc *p;
	register struct a {
		int	req;
		int	pid;
		int	*addr;
		int	data;
	} *uap;

	up = &u;
	uap = (struct a *)up->u_ap;
	if (uap->req <= 0) {
		up->u_procp->p_flag |= STRC;
		return;
	}
	for (p=proc; p < (struct proc *)v.ve_proc; p++) 
		if (p->p_stat==SSTOP
		 && p->p_pid==uap->pid
		 && p->p_ppid==up->u_procp->p_pid)
			goto found;
	up->u_error = ESRCH;
	return;

    found:
	ipcp = &ipc;
	while (ipcp->ip_lock)
		(void) sleep((caddr_t)ipcp, IPCPRI);
	ipcp->ip_lock = p->p_pid;
	ipcp->ip_data = uap->data;
	ipcp->ip_addr = uap->addr;
	ipcp->ip_req = uap->req;
	p->p_flag &= ~SWTED;
	setrun(p);
	while (ipcp->ip_req > 0)
		(void) sleep((caddr_t)ipcp, IPCPRI);
	up->u_rval1 = ipcp->ip_data;
	if (ipcp->ip_req < 0)
		up->u_error = EIO;
	ipcp->ip_lock = 0;
	wakeup((caddr_t)ipcp);
}

/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */
procxmt()
{
	register struct ipctrace *ipcp;
	register struct user *up;
	register int i;
	register *p;
	register struct text *xp;

	up = &u;
	ipcp = &ipc;
	if (ipcp->ip_lock != up->u_procp->p_pid)
		return(0);
	i = ipcp->ip_req;
	ipcp->ip_req = 0;
	wakeup((caddr_t)ipcp);
	switch (i) {

	/* read user I */
	case 1:
		ipcp->ip_data = fuiword((caddr_t)ipcp->ip_addr);
		break;

	/* read user D */
	case 2:
		ipcp->ip_data = fuword((caddr_t)ipcp->ip_addr);
		break;

	/* read u */
	case 3:
		i = (int)ipcp->ip_addr;
		if (i<0 || i >= ctob(v.v_usize))
			goto error;
		ipcp->ip_data = *((int *)((char *)up + (i & ~1)));
		break;

	/* write user I */
	/* Must set up to allow writing */
	case 4:
		/*
		 * If text, must assure exclusive use
		 */
		if (xp = up->u_procp->p_textp) {
			if (xp->x_count!=1 || xp->x_iptr->i_mode&ISVTX)
				goto error;
			xp->x_iptr->i_flag &= ~ITEXT;
		}
		(void) estabur(up->u_tsize, up->u_dsize, up->u_ssize, 0, RW);
		i = suiword((caddr_t)ipcp->ip_addr, 0);
		(void) suiword((caddr_t)ipcp->ip_addr, ipcp->ip_data);
		(void) estabur(up->u_tsize, up->u_dsize, up->u_ssize, 0, RO);
		if (i<0)
			goto error;
		if (xp)
			xp->x_flag |= XWRIT;
		break;

	/* write user D */
	case 5:
		if (suword((caddr_t)ipcp->ip_addr, 0) < 0)
			goto error;
		(void) suword((caddr_t)ipcp->ip_addr, ipcp->ip_data);
		break;

	/* write u */
	case 6:
		i = (int)ipcp->ip_addr;
		p = (int *)((char *)up + (i & ~1));
		for (i=0; i<17; i++)
			if (p == &up->u_ar0[regloc[i]])
				goto ok;
		if (p == &up->u_ar0[RPS]) {
			/* assure user space and priority 0 */
			ipcp->ip_data &= ~0x2700;
			goto ok;
		}
		goto error;

	ok:
		*p = ipcp->ip_data;
		break;

	/* set signal and continue */
	/* one version causes a trace-trap */
	case 9:
		up->u_ar0[RPS] |= PS_T;
	case 7:
		if ((int)ipcp->ip_addr != 1)
			up->u_ar0[PC] = (int)ipcp->ip_addr;
		up->u_procp->p_sig = 0L;
		if (ipcp->ip_data)
			psignal(up->u_procp, ipcp->ip_data);
		return(1);

	/* force exit */
	case 8:
		exit(fsig(up->u_procp));

	/* read u registers */
	case 10:
		if ((i = (int)ipcp->ip_addr) < 0 || i > 17)
			goto error;
		if (i == 17)
			ipcp->ip_data = up->u_ar0[regloc[17]] & 0xFFFF;
		else
			ipcp->ip_data = up->u_ar0[regloc[i]];
		break;

	/* write u registers */
	case 11:
		
		if ((i = (int)ipcp->ip_addr) < 0 || i > 17)
			goto error;
		if (i == 17) {
			ipcp->ip_data &= ~0x2700;	/* user only */
			up->u_ar0[regloc[17]] =
				(up->u_ar0[regloc[17]] & ~0xFFFF) |
				(ipcp->ip_data & 0xFFFF);
		} else
			up->u_ar0[regloc[i]] = ipcp->ip_data;
		break;
		
	default:
	error:
		ipcp->ip_req = -1;
	}
	return(0);
}
