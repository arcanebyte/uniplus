/* @(#)clock.c	1.1 */
#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/callo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/psl.h"
#include "sys/var.h"
#include "sys/reg.h"
#ifdef UCB_NET
#include "net/misc.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/if.h"
#include "net/in_systm.h"
#endif

#ifdef VIRTUAL451
#define args	buserr
#define a_ps	ber_sr
#define a_pc	ber_pc
#define a_dev	ber_dev
#endif

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	alarm clock signals
 *	jab the scheduler
 */

#define	PRF_ON	01
extern	prfstat;

time_t	time, lbolt;

#ifdef  UCB_NET
/*
 * Protoslow is like lbolt, but for slow protocol timeouts, counting
 * up to (hz/PR_SLOWHZ), then causing a pfslowtimo().
 * Protofast is like lbolt, but for fast protocol timeouts, counting
 * up to (hz/PR_FASTHZ), then causing a pffasttimo().
 */
extern int	protoslow;
extern int	protofast;
extern int	ifnetslow;
extern short	netoff;
#endif

clock(ap)
struct args *ap;
{
	register struct callo *p1, *p2;
	register struct user *up;
	register struct proc *pp;
	register a, ps;
	static short lticks;
	static rqlen, sqlen;

	/*
	 * if panic stop clock
	 */
	if (panicstr) {
		clkstop();
		return;
	}

	up = &u;
	ps = ap->a_ps;
	if (up->u_stack[0] != STKMAGIC)
		panic("Interrupt stack overflow");

#ifdef  UCB_NET
	/*
	 * Time moves on for protocols.
	 */
	if(!netoff) {
		--protoslow; --protofast; --ifnetslow;
		if(protoslow<=0 || protofast<=0 || ifnetslow<=0) {
			schednetisr(NETISR_CLOCK);
		}
	}
#endif

	/*
	 * callouts
	 * if none, just continue
	 * else update first non-zero time
	 */

	if(callout[0].c_func == NULL)
		goto out;
	p2 = &callout[0];
	while(p2->c_time<=0 && p2->c_func!=NULL)
		p2++;
	p2->c_time--;

	/*
	 * if ps is high, just return
	 */

	if (BASEPRI(ps))
		goto out;

	/*
	 * if any callout active, update first non-zero time
	 * then process necessary callouts
	 */

	spltty();
	if(callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
			(*p1->c_func)(p1->c_arg);
			p1++;
		}
		p2 = &callout[0];
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}

out:
	if (prfstat & PRF_ON)
		prfintr((caddr_t)ap->a_pc, ps);
	if (USERMODE(ps)) {
		a = CPU_USER;
		up->u_utime++;
		if (up->u_prof.pr_scale)
			addupc((unsigned)ap->a_pc, &up->u_prof, 1);
	} else {
		if (ap->a_dev != 0) {	/* dev has old idleflg in it */
			if (syswait.iowait+syswait.swap+syswait.physio) {
				a = CPU_WAIT;
				if (syswait.iowait)
					sysinfo.wait[W_IO]++;
				if (syswait.swap)
					sysinfo.wait[W_SWAP]++;
				if (syswait.physio)
					sysinfo.wait[W_PIO]++;
			} else
				a = CPU_IDLE;
		} else {
			a = CPU_KERNEL;
			up->u_stime++;
		}
	}
	sysinfo.cpu[a]++;
	pp = up->u_procp;
	if (pp->p_stat==SRUN) {
		up->u_mem += (unsigned)(v.v_usize+procsize(pp));
		if (pp->p_textp) {
			a = pp->p_textp->x_ccount;
			if (a==0 || a==1)
				up->u_mem += pp->p_textp->x_size;
			else
				up->u_mem +=
					(unsigned short)pp->p_textp->x_size /
					(short)a;
		}
	}
	if (pp->p_cpu < 80)
		pp->p_cpu++;
	lbolt++;	/* time in ticks */
	if (--lticks <= 0) {
		if (BASEPRI(ps))
			return;
		lticks += v.v_hz;
		++time;
		if ((time & 3) == 0)	/* entry to load average */
			loadav();
		runrun++;
		rqlen = 0;
		sqlen = 0;
		for(pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_stat) {
			if (pp->p_time != 127)
				pp->p_time++;
			if (pp->p_clktim)
				if (--pp->p_clktim == 0)
					psignal(pp, SIGALRM);
			pp->p_cpu >>= 1;
			if (pp->p_pri >= (PUSER-NZERO)) {
				pp->p_pri = (pp->p_cpu>>1) + PUSER +
					pp->p_nice - NZERO;
			}
			if (pp->p_stat == SRUN)
				if (pp->p_flag & SLOAD)
					rqlen++;
				else
					sqlen++;
		}
		if (rqlen) {
			sysinfo.runque += rqlen;
			sysinfo.runocc++;
		}
		if (sqlen) {
			sysinfo.swpque += sqlen;
			sysinfo.swpocc++;
		}
		if (runin!=0) {
			runin = 0;
			setrun(&proc[0]);
		}
#ifdef VIRTUAL451
		if (runout!=0) {
			runout = 0;
			setrun(&proc[0]);
		}
#endif VIRTUAL451
	}
}

/*
 * timeout is called to arrange that fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout structure.
 * The time in each structure entry is the number of HZ's more
 * than the previous entry. In this way, decrementing the
 * first entry has the effect of updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 */
timeout(fun, arg, tim)
int (*fun)();
caddr_t arg;
int tim;
{
	register struct callo *p1, *p2;
	register int t;
	int s;

	t = tim;
	p1 = &callout[0];
	s = spl7();
	while(p1->c_func != 0 && p1->c_time <= t) {
		t -= p1->c_time;
		p1++;
	}
	if (p1 >= (struct callo *)v.ve_call-1)
		panic("Timeout table overflow");
	p1->c_time -= t;
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) {
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	splx(s);
}

#define	PDELAY	(PZERO-1)
delay(ticks)
{
	extern wakeup();
	int s;

	if (ticks<=0)
		return;
	s = spl7();
	timeout(wakeup, (caddr_t)u.u_procp+1, ticks);
	(void) sleep((caddr_t)u.u_procp+1, PDELAY);
	splx(s);
}

/*
 * From here down is load average code
 */
struct lavnum {
	unsigned short high;
	unsigned short low;
};

struct lavnum avenrun[3];

/*
 * Constants for averages over 1, 5, and 15 minutes
 * when sampling at 4 second intervals.
 * (Using 'fixed-point' with 16 binary digits to right)
 */
struct lavnum cexp[3] = {
	{ 61309, 4227 },	/* (x = exp(-1/15) * 65536) , 1 - x */
	{ 64667, 869 },		/* (x = exp(-1/75) * 65536) , 1 - x */
	{ 65245, 291 },		/* (x = exp(-1/225) * 65536) , 1 - x */
};

/* called once every four seconds */
loadav()
{
	register struct lavnum *avg;
	register struct lavnum *rcexp;
	register unsigned int j;
	register unsigned short nrun;
	register struct proc *p;

	nrun = 0;
	for (p = &proc[0]; p < (struct proc *)v.ve_proc; p++) {
		if (p->p_flag & SSYS)
			continue;
		if (p->p_stat) {
			switch (p->p_stat) {
			case SSLEEP:
			case SSTOP:
				if (p->p_pri <= PZERO)
					nrun++;
				break;
			case SRUN:
			case SIDL:
				nrun++;
				break;
			}
		}
	}
	/*
	 * Compute a tenex style load average of a quantity on
	 * 1, 5 and 15 minute intervals.
	 * (Using 'fixed-point' with 16 binary digits to right)
	 */
	avg = avenrun;
	rcexp = cexp;
	for ( ; avg < &avenrun[3]; avg++, rcexp++) {
		j = ((avg->low * rcexp->high + 32768) >> 16)
		    + (avg->high * rcexp->high)
		    + (nrun * rcexp->low);
		avg->low = j & 65535;
		avg->high = j >> 16;
	}
}
