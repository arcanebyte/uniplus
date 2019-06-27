#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "net/misc.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/ubavar.h"
#include "sys/map.h"
#include "sys/callo.h"
#include "net/if.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "sys/var.h"

/*
 *	SCCS id	@(#)syslocal.c	1.6 (Berkeley)	2/27/82
 */

/*
 * These routines implement local system calls
 */




short   netoff = 0;
int	protoslow;
int	protofast;
int	ifnetslow;
int	nselcoll;
char	netstak[3000];	/* stack when system is "in the network" (mch.s) */
char *	svstak;		/* globle where mch.s saves current stack */

/*
 * Initialize network code.  Called from main().
 */
netinit()
{
	extern struct uba_device ubdinit[];
	register struct uba_driver *udp;
	register struct uba_device *ui = &ubdinit[0];

	if(netoff) return;
	mbinit();
	for(ui = &ubdinit[0] ; udp = ui->ui_driver ; ui++) {
		/*  ud_probe could go here */
		ui->ui_alive = 1;
		udp->ud_dinfo[ui->ui_unit] = ui;
		(*udp->ud_attach)(ui);
	}
#ifdef INET
	loattach();			/* XXX */
	ifinit();
	pfinit();			/* must follow interfaces */
#endif
#ifdef TCPACKMOST
	/*
	 * Fool some systems into thinking that tcp_maxseg == sbspace ==
	 * max window.  Ergo, force acking of most packets when talking to
	 * (the vax)
	 */
	{
		extern int tcp_sendspace, tcp_recvspace;

		tcp_sendspace = tcp_recvspace = 1024;
	}
#endif
}

/* netreset -- network reset system call */
/* 3/23/83 billn: doesnt quite work yet */
netreset()
{
	register wait = 100000;
	register s = spl6();
	register struct file *fp;
	extern int netisr;

	netoff = 1;		/* prevent net traffic (during resets) */
	netisr = 0;

	/* unbeknownst sockets to UNIX */
	for (fp = &file[0]; fp < (struct file *)v.ve_file; fp++) {
	    if ((fp->f_flag & FSOCKET) && fp->f_count) {
		bzero ((char *)fp, sizeof (struct file));
	    }
	}

	mbinit();		/* relink mbufs */
#ifdef INET
	ifinit();		/* re-init interfaces */
	pfinit();		/* must follow interfaces */
#endif
	splx(s);
	while (wait--);		/* wait for pending ints. */
	netoff = 0;		/* allow traffic */
}

splimp()
{
	return spl6();
}

splnet()
{
	return spl6();
}

/*
 * Entered via software interrupt vector at spl1.  Check netisr bit array
 * for tasks requesting service.
 */
netintr()
{
	register int onetisr;

	while(spl7(),(onetisr = netisr)) {
		netisr = 0;
		(void) splnet();
		if (onetisr & (1<<NETISR_RAW))
			rawintr();
		if (onetisr & (1<<NETISR_IP))
			ipintr();
		if (protofast <= 0) {
			protofast = HZ / PR_FASTHZ;
			pffasttimo();
		}
		if (protoslow <= 0) {
			protoslow = HZ / PR_SLOWHZ;
			pfslowtimo();
		}
		if (ifnetslow <= 0) {
			ifnetslow = HZ / IFNET_SLOWHZ;
			if_slowtimo();
		}
	}
}



int     enprint = 0;            /* enable nprintf */

/*
 * net printf.  prints to net log area in memory (nlbase, nlsize).
 */
/* VARARGS1 */
nprintf (fmt, x1)
char *fmt;
unsigned x1;
{
	if (enprint == 0) return;
	/* billn -- do regular printf for now
	prf (fmt, &x1, 4);
	*/
	printf (fmt, x1);
}

/*
 * Select system call.
 */
select()
{
	register struct uap  {
		int	nfd;
		fd_set	*rp, *wp;
		long    timo;
	} *ap = (struct uap *)u.u_ap;
	fd_set rd, wr;
	int nfds = 0;
	long selscan();
	extern setrun();
	long readable = 0, writeable = 0;
	time_t t = time;
	int s, tsel, ncoll, rem;
	label_t lqsav;

	if (ap->nfd > NOFILE)
		ap->nfd = NOFILE;
	if (ap->nfd < 0) {
		u.u_error = EBADF;
		return;
	}
	if (ap->rp && copyin((caddr_t)ap->rp,(caddr_t)&rd,sizeof(fd_set)))
		return;
	if (ap->wp && copyin((caddr_t)ap->wp,(caddr_t)&wr,sizeof(fd_set)))
		return;
retry:
	ncoll = nselcoll;
	u.u_procp->p_flag |= SSEL;
	if (ap->rp)
		readable = selscan(ap->nfd, rd, &nfds, FREAD);
	if (ap->wp)
		writeable = selscan(ap->nfd, wr, &nfds, FWRITE);
	if (u.u_error)
		goto done;
	if (readable || writeable)
		goto done;
	rem = (ap->timo+999)/1000 - (time - t);
	if (ap->timo == 0 || rem <= 0)
		goto done;
	s = spl6();
	if ((u.u_procp->p_flag & SSEL) == 0 || nselcoll != ncoll) {
		u.u_procp->p_flag &= ~SSEL;
		splx(s);
		goto retry;
	}
	u.u_procp->p_flag &= ~SSEL;
	if (rem) {
		bcopy((caddr_t)u.u_qsav, (caddr_t)lqsav, sizeof (label_t));
		if (save(u.u_qsav)) {
			rm_callout(setrun, (caddr_t)u.u_procp);
			u.u_error = EINTR;
			splx(s);
			goto done;
		}
		rem = rem*v.v_hz;
		timeout(setrun, (caddr_t)u.u_procp, rem);
	}
	sleep((caddr_t)&selwait, PZERO+1);
	if (rem) {
		bcopy((caddr_t)lqsav, (caddr_t)u.u_qsav, sizeof (label_t));
		rm_callout(setrun, (caddr_t)u.u_procp);
	}
	splx(s);
	goto retry;
done:
	rd.fds_bits[0] = readable;
	wr.fds_bits[0] = writeable;
	u.u_rval1 = nfds;
	if (ap->rp)
		(void) copyout((caddr_t)&rd, (caddr_t)ap->rp, sizeof(fd_set));
	if (ap->wp)
		(void) copyout((caddr_t)&wr, (caddr_t)ap->wp, sizeof(fd_set));
}

/*
 * remove entry in callout vector
 * which is scanned by clock interrupt
 */
rm_callout(func,arg)
int (*func)();
caddr_t arg;
{
	register struct callo *p1, *p2;
	register int tt;
	int pri;

	p1 = &callout[0];
	pri = spl7();
	while(p1->c_func != 0) {
		if ((p1->c_func == func) && (p1->c_arg == arg))
			break;
		p1++;
	}
	if (p1 >= (struct callo *)v.ve_call-1) {
		printf("Timeout entry not found, not deleted\n");
		return;
	}
	/* copy everything that follows in the list up one
	   position, adding our unused time to theirs */
	tt = p1->c_time;
	p2 = p1;
	while(p1->c_func != 0) {
		p2++;
		p1->c_time = p2->c_time + tt;
		p1->c_func = p2->c_func;
		p1->c_arg = p2->c_arg;
		p1 = p2;
	}
	splx(pri);
}

long
selscan(nfd, fds, nfdp, flag)
	int nfd;
	fd_set fds;
	int *nfdp, flag;
{
	struct file *fp;
	struct inode *ip;
	long bits,res = 0;
	int i, able;
		
	bits = fds.fds_bits[0];
	while (i = ffs(bits)) {
		if (i >= nfd)
			break;
		bits &= ~(1L<<(i-1));
		fp = u.u_ofile[i-1];
		if (fp == NULL) {
			u.u_error = EBADF;
			return (0);
		}
		if (fp->f_flag & FSOCKET)
			able = soselect((struct socket *)fp->f_socket, flag);
		else {
			ip = fp->f_inode;
			switch (ip->i_mode & IFMT) {

			case IFCHR:
#ifdef notdef
				able =
				    (*cdevsw[major(ip->i_rdev)].d_select)
					((int)ip->i_rdev, flag);
#else
				/*
				 * for now the only char device we need
				 * to select on is the control side of
				 * ptys -- for the rlogin deamon.  At
				 * some point someone can put general
				 * select code into all char. devices.
				 */
				{
				extern int ptc_dev;

				if (major((int)(ip->i_rdev)) == ptc_dev)
				    able = ptcselect((int)ip->i_rdev, flag);
				else
				    able = 0;
				}
#endif
				break;

			case IFIFO:
			case IFBLK:
			case IFREG:
			case IFDIR:
				able = 1;
				break;
			}
		}
		if (able) {
			res |= (1L<<(i-1));
			(*nfdp)++;
		}
	}
	return (res);
}

ffs(mask)
	long mask;
{
	register int i;
	register imask;

	if (mask == 0) return (0);
	imask = loint(mask);
	for(i=1; i<=16; i++) {
		if (imask & 1)
			return (i);
		imask >>= 1;
	}
	imask = hiint(mask);
	for(; i<=32; i++) {
		if (imask & 1)
			return (i);
		imask >>= 1;
	}
	return (0);     /* can't get here anyway! */
}

#ifdef notdef
/*ARGSUSED*/
seltrue(dev, flag)
	dev_t dev;
	int flag;
{

	return (1);
}
#endif

selwakeup(p, coll)
	register struct proc *p;
	int coll;
{
	int s;

	if (coll) {
		nselcoll++;
		wakeup((caddr_t)&selwait);
	}
	s = spl6();
	if (p) {
		if (p->p_wchan == (caddr_t)&selwait)
			setrun(p);
		else {
			if (p->p_flag & SSEL)
				p->p_flag &= ~SSEL;
		}
	}
	splx(s);
}

#ifdef notdef
/* ARGSUSED */
nulselect(x, y)
{
	return 0;
}
#endif

char	hostname[32] = "hostnameunknown";
int	hostnamelen = 16;

gethostname()
{
	register struct a {
		char	*hostname;
		int	len;
	} *uap = (struct a *)u.u_ap;
	register int len;

	len = uap->len;
	if (len > hostnamelen)
		len = hostnamelen;
	if (copyout((caddr_t)hostname, (caddr_t)uap->hostname, len))
		u.u_error = EFAULT;
}

sethostname()
{
	register struct a {
		char	*hostname;
		int	len;
	} *uap = (struct a *)u.u_ap;

	if (!suser())
		return;
	if (uap->len > sizeof (hostname) - 1) {
		u.u_error = EINVAL;
		return;
	}
	hostnamelen = uap->len;
	if (copyin((caddr_t)uap->hostname, hostname, uap->len + 1))
		u.u_error = EFAULT;
}


/*
 *  Some misc. subroutines.  Prob should be in a sep module
 */

#ifdef notdef		/* system 3 has it's own */
/*
 * Provide about n microseconds of delay
 */
delay(n)
long n;
{
	register hi,low;

	low = (n&0177777);
	hi = n>>16;
	if(hi==0) hi=1;
	do {
		do { } while(--low);
	} while(--hi);
}
#endif

/*
 * compare bytes; same result as VAX cmpc3.
 */
bcmp(s1, s2, n)
register char *s1, *s2;
register n;
{
	do
		if(*s1++ != *s2++) break;
	while(--n);
	return(n);
}

/*
 * Insert an entry onto queue.
 */
_insque(e,prev)
	register struct vaxque *e,*prev;
{
	register x = spl7();
	e->vq_prev = prev;
	e->vq_next = prev->vq_next;
	if (prev->vq_next)
		prev->vq_next->vq_prev = e;
	prev->vq_next = e;
	splx(x);
}

/*
 * Remove an entry from queue.
 */
_remque(e)
	register struct vaxque *e;
{
	register x = spl7();

	e->vq_prev->vq_next = e->vq_next;
	if (e->vq_next)
		e->vq_next->vq_prev = e->vq_prev;
	splx(x);
}


struct proc *
pfind(pid)
	int pid;
{
	register struct proc *p;

	for (p=proc; p < &proc[v.v_proc]; p++) 
		if (p->p_pid == pid)
			return (p);
	return ((struct proc *)0);
}

/*  bzero(p,n) -- zero n bytes starting at p */

bzero(p,n)
register char * p;
register n;
{

	if (n)
		do {
			*p++ = 0;
		} while (--n);
}
/*
 *  iomalloc -- allocate clks of mem. for io.
 *  Right now, no dma.
*/
mbioalloc()
{
	return 0;
}

/*
 * mballoc should be combined with/done like memap(). THIS STUFF DEPENDS
 * ON MSIZE BEING A POWER OF 2.
 */
#define MBUFCONFIG 1		/* undef extern of Mbuf, mbuf */
#include <net/mbuf.h>

char	mbufbufs[(NMBUFS+1)*MSIZE];

struct mbuf *
mballoc()
{
	unsigned int location = (unsigned int)(&mbufbufs[0]);
	int slop = location & (MSIZE-1);

	/* round actual buffers to MSIZE boundry */
	return  ((struct mbuf*)(location + MSIZE - slop));
}

