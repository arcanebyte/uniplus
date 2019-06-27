/* @(#)slp.c	1.8 */
#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/context.h"
#include "sys/text.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/errno.h"
#include "sys/scat.h"

typedef int	mem_t;

#define	NHSQUE	64	/* must be power of 2 */
#define	sqhash(X)	(&hsque[((int)(X) >> 3) & (NHSQUE-1)])
struct proc *hsque[NHSQUE];
char	runin, runout, runrun, curpri;
struct proc *curproc, *runq;

/*
 * sleep according to a cpu adjusted priority
 */
asleep(chan, pri)
caddr_t chan;
{
	return(sleep(chan, pri + ((u.u_procp->p_cpu&0xFF) >> 5)));
}

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
#define	TZERO	10
sleep(chan, disp)
caddr_t chan;
{
	register struct proc *rp = u.u_procp;
	register struct proc **q = sqhash(chan);
	register s;

	s = splhi();
	if (panicstr) {
		SPL0();
		splx(s);
		return(0);
	}
	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_link = *q;
	*q = rp;
	if (rp->p_time > TZERO)
		rp->p_time = TZERO;
	if ((rp->p_pri = (disp&PMASK)) > PZERO) {
		if (rp->p_sig && issig()) {
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			*q = rp->p_link;
			SPL0();
			goto psig;
		}
		SPL0();
		if (runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		swtch();
		if (rp->p_sig && issig())
			goto psig;
	} else {
		SPL0();
		swtch();
	}
	splx(s);
	return(0);

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * if PCATCH is set, return 1, else
	 * execute non-local goto to the qsav location.
	 */
psig:
	splx(s);
	if (disp&PCATCH)
		return(1);
#ifdef NONSCATLOAD
	resume(u.u_procp->p_addr, u.u_qsav);
#else
	resume(ixtoc(u.u_procp->p_scat), u.u_qsav);
#endif
	/* NOTREACHED */
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
register caddr_t chan;
{
	register struct proc *p;
	register struct proc **q;
	register s;

	s = splhi();
	for (q = sqhash(chan); p = *q; )
		if (p->p_wchan==chan && p->p_stat==SSLEEP) {
			p->p_stat = SRUN;
			p->p_wchan = 0;
			/* take off sleep queue, put on run queue */
			*q = p->p_link;
			p->p_link = runq;
			runq = p;
			if (!(p->p_flag&SLOAD)) {
				p->p_time = 0;
				/* defer setrun to avoid breaking link chain */
				if (runout > 0)
					runout = -runout;
			} else if (p->p_pri < curpri)
				runrun++;
		} else
			q = &p->p_link;
	if (runout < 0) {
		runout = 0;
		setrun(&proc[0]);
	}
	splx(s);
}

setrq(p)
register struct proc *p;
{
	register struct proc *q;
	register s;

	s = splhi();
	for(q=runq; q!=NULL; q=q->p_link)
		if (q == p) {
			printf("proc on q\n");
			goto out;
		}
	p->p_link = runq;
	runq = p;
out:
	splx(s);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
register struct proc *p;
{
	register struct proc **q;
	register s;

	s = splhi();
	if (p->p_stat == SSLEEP) {
		/* take off sleep queue */
		for (q = sqhash(p->p_wchan); *q != p; q = &(*q)->p_link) ;
		*q = p->p_link;
		p->p_wchan = 0;
	} else if (p->p_stat == SRUN) {
		/* already on run queue - just return */
		splx(s);
		return;
	}
	/* put on run queue */
	p->p_stat = SRUN;
	p->p_link = runq;
	runq = p;
	if (!(p->p_flag&SLOAD)) {
		p->p_time = 0;
		if (runout > 0) {
			runout = 0;
			setrun(&proc[0]);
		}
	} else if (p->p_pri < curpri)
		runrun++;
	splx(s);
}

/*
 * The main loop of the scheduling (swapping) process.
 * The basic idea is:
 *  see if anyone wants to be swapped in;
 *  swap out processes until there is room;
 *  swap him in;
 *  repeat.
 * The runout flag is set whenever someone is swapped out.
 * Sched sleeps on it awaiting work.
 *
 * Sched sleeps on runin whenever it cannot find enough
 * memory (by swapping out or otherwise) to fit the
 * selected swapped process.  It is awakened when the
 * memory situation changes and in any case once per second.
 */
sched()
{
	register struct proc *rp, *p;
	register outage, inage;
	int maxbad;
	int tmp;

	/*
	 * find user to swap in;
	 * of users ready, select one out longest
	 */

loop:
	SPL6();
	outage = -20000;
#ifdef NONSCATLOAD
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++)
	if (rp->p_stat==SRUN && (rp->p_flag&SLOAD) == 0 &&
	    rp->p_time > outage) {
		p = rp;
		outage = rp->p_time;
	}
#else
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++)
	if ((rp->p_flag&(SSWAPIT|SLOAD)) == (SSWAPIT|SLOAD)) {
		p = rp;
		SPL0();
		goto swapit;
	} else if (rp->p_stat==SRUN && (rp->p_flag&SLOAD) == 0 &&
	    rp->p_time > outage) {
		p = rp;
		outage = rp->p_time;
	}
#endif
	/*
	 * If there is no one there, wait.
	 */
	if (outage == -20000) {
		runout++;
		(void) sleep((caddr_t)&runout, PSWP);
		goto loop;
	}
	SPL0();

	/*
	 * See if there is memory for that process;
	 * if so, swap it in.
	 */

	if (swapin(p))
		goto loop;

	/*
	 * none found.
	 * look around for memory.
	 * Select the largest of those sleeping
	 * at bad priority; if none, select the oldest.
	 */

	SPL6();
	p = NULL;
	maxbad = 0;
	inage = 0;
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++) {
		if (rp->p_stat==SZOMB ||
		    (rp->p_flag&(SSYS|SLOCK|SLOAD))!=SLOAD)
			continue;
		if (rp->p_textp && rp->p_textp->x_flag&XLOCK)
			continue;
		if (rp->p_stat==SSLEEP || rp->p_stat==SSTOP) {
			tmp = rp->p_pri - PZERO + rp->p_time;
			if (maxbad < tmp) {
				p = rp;
				maxbad = tmp;
			}
		} else if (maxbad<=0 && rp->p_stat==SRUN) {
			tmp = rp->p_time + rp->p_nice - NZERO;
			if (tmp > inage) {
				p = rp;
				inage = tmp;
			}
		}
	}
	SPL0();
	/*
	 * Swap found user out if sleeping at bad pri,
	 * or if he has spent at least 2 seconds in memory and
	 * the swapped-out process has spent at least 2 seconds out.
	 * Otherwise wait a bit and try again.
	 */
	if (maxbad>0 || (outage>=2 && inage>=2)) {
#ifndef NONSCATLOAD
	swapit:
#endif
		p->p_flag &= ~SLOAD;
		xswap(p, 1, 0);
		goto loop;
	}
	SPL6();
	runin++;
	(void) sleep((caddr_t)&runin, PSWP);
	goto loop;
}

/*
 * Swap a process in.
 */
#ifdef NONSCATLOAD
swapin(p)
register struct proc *p;
{
	register struct text *xp;
	register int a, x;
	int ta;

	if ((a = malloc(coremap, p->p_size)) == NULL)
		return(0);
	if (xp = p->p_textp) {
		xlock(xp);
		if (!xmlink(xp) && xp->x_ccount==0) {
			if ((x = malloc(coremap, xp->x_size)) == NULL) {
				mfree(coremap, p->p_size, a);
				if ((x = malloc(coremap, xp->x_size)) == NULL) {
					xunlock(xp);
					return(0);
				}
				if ((a = malloc(coremap, p->p_size)) == NULL) {
					mfree(coremap, xp->x_size, x);
					xunlock(xp);
					return(0);
				}
			}
			xp->x_caddr = x;
			if ((xp->x_flag&XLOAD)==0)
				swap(xp->x_daddr, x, xp->x_size, B_READ);
		}
		xp->x_ccount++;
		xunlock(xp);
	}
	if (p->p_xaddr[0]) {
		ta = a;
		for (x=0; x < NSCATSWAP; x++) {
			if (p->p_xaddr[x] == 0)
				continue;
			swap(p->p_xaddr[x], a, p->p_xsize[x], B_READ);
			mfree(swapmap, ctod(p->p_xsize[x]), (int)p->p_xaddr[x]);
			a += p->p_xsize[x];
			p->p_xaddr[x] = 0;
		}
		p->p_addr = ta;
	} else {
		swap((daddr_t)p->p_dkaddr, a, p->p_size, B_READ);
		mfree(swapmap, ctod(p->p_size), (int)p->p_dkaddr);
		p->p_addr = a;
	}
	cxrelse(p->p_context);
	p->p_flag |= SLOAD;
	p->p_time = 0;
	return(1);
}
#else
swapin(p)
register struct proc *p;
{
	register struct text *xp;
	register int a, x;
	int ta;

	if (p->p_flag&SCONTIG) {
		if ((a = cmemalloc(p->p_size)) == NULL)
			return(0);
	} else if ((a = memalloc(p->p_size)) == NULL)
		return(0);
	if (xp = p->p_textp) {
		xlock(xp);
		if (!xmlink(xp) && xp->x_ccount==0) {
			if ((x = memalloc(xp->x_size)) == NULL) {
				memfree(a);
				xunlock(xp);
				return(0);
			}
			xp->x_scat = x;
			if ((xp->x_flag&XLOAD)==0)
				(void) swap(xp->x_daddr, x, xp->x_size, B_READ);
		}
		xp->x_ccount++;
		xunlock(xp);
	}
	p->p_flag |= SNOMMU;	/* swapping in, do not set mmu registers */
	if (p->p_xaddr[0]) {
		ta = a;
		for (x=0; x < NSCATSWAP; x++) {
			if (p->p_xaddr[x] == 0)
				continue;
			a = swap(p->p_xaddr[x], a, p->p_xsize[x], B_READ);
			mfree(swapmap, ctod(p->p_xsize[x]), (int)p->p_xaddr[x]);
			p->p_xaddr[x] = 0;
		}
		p->p_scat = ta;
	} else {
		(void) swap((daddr_t)p->p_dkaddr, a, p->p_size, B_READ);
		mfree(swapmap, ctod(p->p_size), (int)p->p_dkaddr);
		p->p_scat = a;
	}
	p->p_flag &= ~SNOMMU;
	p->p_addr = ixtoc(p->p_scat);
	cxrelse(p->p_context);
	p->p_flag |= SLOAD;
	p->p_time = 0;
	return(1);
}
#endif

/*
 * put the current process on
 * the Q of running processes and
 * call the scheduler.
 */
qswtch()
{
	setrq(u.u_procp);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */
swtch()
{
	register n;
	register struct proc *p, *q, *pp, *pq;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	/*
	 * If not the idle process, resume the idle process.
	 */
	sysinfo.pswitch++;
	if (u.u_procp != &proc[0]) {
		if (save(u.u_rsav)) {
			sureg();
			return;
		}
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
#ifdef NONSCATLOAD
		resume(proc[0].p_addr, u.u_qsav);
#else
		resume(ixtoc(proc[0].p_scat), u.u_qsav);
#endif
	}
	/*
	 * The first save returns nonzero when proc 0 is resumed
	 * by another process (above); then the second is not done
	 * and the process-search loop is entered.
	 *
	 * The first save returns 0 when swtch is called in proc 0
	 * from sched().  The second save returns 0 immediately, so
	 * in this case too the process-search loop is entered.
	 * Thus when proc 0 is awakened by being made runnable, it will
	 * find itself and resume itself at rsav, and return to sched().
	 */
	if (save(u.u_qsav) == 0 && save(u.u_rsav))
		return;
loop:
	SPL6();
	runrun = 0;
	/*
	 * Search for highest-priority runnable process
	 */
	if (p = runq) {
		q = NULL;
		pp = NULL;
		n = 128;
		do {
			if ((p->p_flag&SLOAD) && p->p_pri <= n) {
				pp = p;
				pq = q;
				n = p->p_pri;
			}
			q = p;
		} while (p = p->p_link);
	} else
		goto cont;
	/*
	 * If no process is runnable, idle.
	 */
	if (pp == NULL) {
cont:		curpri = PIDLE;
		curproc = &proc[0];
		idle();
		goto loop;
	}
	p = pp;
	q = pq;
	if (q == NULL)
		runq = p->p_link;
	else
		q->p_link = p->p_link;
	curpri = n;
	curproc = p;
	SPL0();
	/*
	 * The rsav (ssav) contents are interpreted in the new address space
	 */
	n = p->p_flag&SSWAP;
	p->p_flag &= ~SSWAP;
#ifdef NONSCATLOAD
	resume(p->p_addr, n? u.u_ssav: u.u_rsav);
#else
	resume(ixtoc(p->p_scat), n? u.u_ssav: u.u_rsav);
#endif
}

int mpid;	/* external for sunix so it can be reinitialized */
/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
newproc(i)
{
	register struct proc *rpp, *rip;
	register struct user *up;
	register n;
	register a;
	struct proc *pend;
	/* static mpid; */

	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
	up = &u;
	rpp = NULL;
retry:
	mpid++;
	if (mpid >= MAXPID) {
		mpid = 0;
		goto retry;
	}
	rip = &proc[0];
	n = (struct proc *)v.ve_proc - rip;
	a = 0;
	do {
		if (rip->p_stat == NULL) {
			if (rpp == NULL)
				rpp = rip;
			continue;
		}
		if (rip->p_pid==mpid)
			goto retry;
		if (rip->p_uid == up->u_ruid)
			a++;
		pend = rip;
	} while(rip++, --n);
	if (rpp==NULL) {
		if ((struct proc *)v.ve_proc >= &proc[(short)v.v_proc]) {
			if (i) {
				syserr.procovf++;
				up->u_error = EAGAIN;
				return(-1);
			} else
				panic("no procs");
		}
		rpp = (struct proc *)v.ve_proc;
	}
	if (rpp > pend)
		pend = rpp;
	pend++;
#ifdef lint
	v.ve_proc = pend;
#else
	v.ve_proc = (char *)pend;
#endif
	if (up->u_uid && up->u_ruid) {
		if (rpp == &proc[(short)(v.v_proc-1)] || a > v.v_maxup) {
			up->u_error = EAGAIN;
			return(-1);
		}
	}
	/*
	 * make proc entry for new proc
	 */

	rip = up->u_procp;
	rpp->p_stat = SRUN;
	rpp->p_clktim = 0;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_suid = rip->p_suid;
	rpp->p_pgrp = rip->p_pgrp;
	rpp->p_nice = rip->p_nice;
	rpp->p_textp = rip->p_textp;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_time = 0;
	rpp->p_cpu = rip->p_cpu;
	rpp->p_sigign = rip->p_sigign;
	rpp->p_pri = PUSER + rip->p_nice - NZERO;
#ifndef NONSCATLOAD
	rpp->p_scat = rip->p_scat;
#endif
	rpp->p_addr = rip->p_addr;
	rpp->p_size = rip->p_size;

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(n=0; n<NOFILE; n++)
		if (up->u_ofile[n] != NULL)
			up->u_ofile[n]->f_count++;
	if (rpp->p_textp != NULL) {
		rpp->p_textp->x_count++;
		rpp->p_textp->x_ccount++;
	}
	up->u_cdir->i_count++;
	if (up->u_rdir)
		up->u_rdir->i_count++;

	shmfork(rpp, rip);

	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	up->u_procp = rpp;
	curproc = rpp;
	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */
	if (save(up->u_ssav)) {
		sureg();
		return(1);
	}
	/*
	 * If there is not enough memory for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	if (procdup(rpp) == NULL) {
		rip->p_stat = SIDL;
		xswap(rpp, 0, 0);
		rip->p_stat = SRUN;
	}
	up->u_procp = rip;
	curproc = rip;
	if (rip != &proc[0])	/* only do if not scheduler */
		sureg();
	setrq(rpp);
	rpp->p_flag |= SSWAP;
	up->u_rval1 = rpp->p_pid;	/* parent returns pid of child */
	return(0);
}

/*
 * Change the size of the data+stack regions of the process.
 * If the size is shrinking, it's easy-- just release the extra core.
 * If it's growing, and there is core, just allocate it
 * and copy the image, taking care to reset registers to account
 * for the fact that the system's stack has moved.
 * If there is no memory, arrange for the process to be swapped
 * out after adjusting the size requirement-- when it comes
 * in, enough memory will be allocated.
 *
 * After the expansion, the caller will take care of copying
 * the user's stack towards or away from the data area.
 */
#ifdef NONSCATLOAD
expand(newsize)
register newsize;
{
	register struct proc *p;
	register a1, a2;
	register i, n;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	if (n == newsize)
		return;
	a1 = p->p_addr;
	if (n >= newsize) {
#ifdef EXPANDTRACE
		printf("expand:shrinking process by %d clicks\n", n-newsize);
#endif
		mfree(coremap, (mem_t)(n-newsize), (mem_t)(a1+newsize));
		return;
	}
	if (save(u.u_ssav)) {
		sureg();
		return;
	}
	/*
	 * See if can just expand in place
	 */
loop:
	a2 = mallocat(coremap, newsize-n, (mem_t)(a1+n));
	if (a2 != NULL) {
#ifdef EXPANDTRACE
		printf("expanding in place by %d clicks at click %d\n",
			newsize-n, a1+n);
#endif
		cxrelse(p->p_context);
		sureg();
		return;
	}
	/*
	 * Will we be releasing shared text space anyway.
	 * If so, then release it now and try in place
	 * expansion again.
	 */
	if ((a2 = domall(coremap, (mem_t)newsize)) == NULL)
		if (xmrelse())
			goto loop;
	if (a2 == NULL && (a2 = malloc(coremap, (mem_t)newsize)) == NULL) {
#ifdef EXPANDTRACE
		printf("expand:calling xswap\n");
#endif
		xswap(p, 1, n);
		p->p_flag |= SSWAP;
		qswtch();
		/* no return */
	}
	p->p_addr = a2;
	for(i=0; i<n; i++)
		copyseg(a1+i, a2+i);
#ifdef EXPANDTRACE
	printf("expand:copyseg %d from 0x%x to 0x%x\n", n, a1, a2);
#endif
	mfree(coremap, (mem_t)n, (mem_t)a1);
	cxrelse(p->p_context);
	resume((mem_t)a2, u.u_ssav);
}
#else
expand(newsize)
register newsize;
{
	register struct scatter *s;
	register struct proc *p;
	register a1, a2;
	register i, n;
	int t;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	if (n == newsize)
		return;
	s = scatmap;
	a1 = p->p_scat;
	if (n >= newsize) {
		/*
		 * shrink memory
		 */
		for (i=1; i<newsize; i++)
			a1 = s[a1].sc_index;
		t = scatfreelist.sc_index;
		scatfreelist.sc_index = s[a1].sc_index;
		i = a1;
		while ((a2 = s[a1].sc_index) != SCATEND)
			a1 = a2;
		s[i].sc_index = SCATEND;
		s[a1].sc_index = t;
		nscatfree += n-newsize;
		/*
		 * Wake scheduler when freeing memory
		 */
		if (runin) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		return;
	}
	if (save(u.u_ssav)) {
		sureg();
		return;
	}
	/*
	 * expand memory
	 */
	if (a2 = memalloc(newsize-n)) {
		/* printf("expanding from %d clicks to %d clicks\n",
			n, newsize); */
		for (i=1; i<n; i++)
			a1 = s[a1].sc_index;
		if (s[a1].sc_index != SCATEND)
			printf("expand:SCATEND expected\n");
		s[a1].sc_index = a2;
		return;
	}
	xswap(p, 1, n);
	p->p_flag |= SSWAP;
	qswtch();
	/* no return */
}
#endif

#ifndef NONSCATLOAD
checkscat(s)
char *s;
{
	register struct proc *p;
	register struct text *xp;
	register i;

	i = countscat(scatfreelist.sc_index);
	printf("%s nscatfree=%d actual=%d\n", s, nscatfree, i);
	if (nscatfree < 30) {
		printf("    freelist chain is ");
		dumpscat(scatfreelist.sc_index);
	}
	for (p = &proc[0]; p < (struct proc *)v.ve_proc; p++) {
		if (p->p_stat==0)
			continue;
		xp = p->p_textp;
		printf("    pid=%d %d used (%d text)\n",
			p->p_pid, countscat(p->p_scat),
			xp ? countscat(xp->x_scat) : 0);
		dumpscat(p->p_scat);
		if (xp) {
			dumpscat(xp->x_scat);
		}
	}
}

dumpscat(a1)
register a1;
{
	register struct scatter *s;

	s = scatmap;
	printf("     ");
	while (a1 != SCATEND) {
		printf(" %d,%x", a1, ixtoc(a1));
		a1 = s[a1].sc_index;
	}
	printf("\n");
}

countscat(a1)
register a1;
{
	register struct scatter *s;
	register i;

	i = 0;
	s = scatmap;
	while (a1 != SCATEND) {
		a1 = s[a1].sc_index;
		i++;
	}
	return(i);
}
#endif
