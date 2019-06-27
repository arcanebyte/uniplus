/* #define HOWFAR */

/* @(#)sys1.c	1.8 */
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
#include "sys/context.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/scat.h"


/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

#define	NCABLK	(NCARGS+BSIZE-1)/BSIZE
exece()
{
	register unsigned nc;
	register char *cp;
	register struct buf *bp;
	register struct execa *uap;
	register struct user *up;
	int na, ne, ucp, ap, c, bno;
	struct inode *ip;
	extern struct inode *gethead();
	char *namep;
	extern int (*putchar)();

	sysinfo.sysexec++;
	if ((ip = gethead()) == NULL)
		return;
	up = &u;
	bp = 0; na = 0; nc = 0; ne = 0;
	uap = (struct execa *)up->u_ap;
	/* collect arglist */
	if ((bno = swapalloc(NCABLK, 0)) == 0) {
		printf("No swap space for exec args\n");
		iput(ip);
		up->u_error = ENOMEM;
		return;
	}
	if (uap->argp) for (;;) {
		ap = NULL;
		if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap==NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) == NULL)
				break;
			uap->envp++;
			ne++;
		}
		if (ap==NULL)
			break;
		na++;
		if (ap == -1)
			up->u_error = EFAULT;
		do {
			if (nc >= NCARGS-1)
				up->u_error = E2BIG;
			if ((c = fubyte((caddr_t)ap++)) < 0)
				up->u_error = EFAULT;
			if (up->u_error)
				goto bad;
			if ((nc&BMASK) == 0) {
				if (bp)
					bdwrite(bp);
				bp = getblk(swapdev,
					(daddr_t)(swplo+bno+(nc>>BSHIFT)));
				cp = bp->b_un.b_addr;
			}
			nc++;
			*cp++ = c;
		} while (c>0);
	}
	if (bp)
		bdwrite(bp);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	getxfile(ip, (int)(nc + sizeof(char *)*na));
	if (up->u_error) {
		printf("exec error:  u_error %d  u_dent.d_name ", up->u_error);
		for (namep = &up->u_dent.d_name[0]; namep && *namep && namep < &up->u_dent.d_name[DIRSIZ]; namep++)
			(*putchar)(*namep);
		(*putchar)('\n');
		psignal(up->u_procp, SIGKILL);
		goto bad;
	}

	/* copy back arglist */

	ucp = v.v_uend - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	up->u_ar0[SP] = ap;
#ifdef HOWFAR
	printf("Setting new stack pointer to 0x%x\n", ap);
#endif
	(void) suword((caddr_t)ap, na-ne);
	nc = 0;
	for (;;) {
		ap += NBPW;
		if (na==ne) {
			ap += NBPW;
		}
		if (--na < 0)
			break;
		(void) suword((caddr_t)ap, ucp);
		do {
			if ((nc&BMASK) == 0) {
				if (bp) {
					bp->b_flags |= B_AGE;
					bp->b_flags &= ~B_DELWRI;
					brelse(bp);
				}
				bp = bread(swapdev,
					(daddr_t)(swplo+bno+(nc>>BSHIFT)));
				bp->b_flags |= B_AGE;
				bp->b_flags &= ~B_DELWRI;
				cp = bp->b_un.b_addr;
			}
			(void) subyte((caddr_t)ucp++, (c = *cp++));
			nc++;
		} while (c&0377);
	}
	(void) suword((caddr_t)ap, 0);
	if ((long)ucp & 1)
		(void) subyte((caddr_t)ucp++, 0);
	(void) suword((caddr_t)ucp, 0);
	setregs();
	if (bp) {
		bp->b_flags |= B_AGE;
		bp->b_flags &= ~B_DELWRI;
		brelse(bp);
	}
	iput(ip);
	mfree(swapmap, NCABLK, bno);
	return;
bad:
	if (bp)
		brelse(bp);
	iput(ip);
	for (nc = 0; nc < NCABLK; nc++) {
		bp = getblk(swapdev, (daddr_t)(swplo+bno+nc));
		bp->b_flags |= B_AGE;
		bp->b_flags &= ~B_DELWRI;
		brelse(bp);
	}
	mfree(swapmap, NCABLK, bno);
}

struct inode *
gethead()
{
	register struct exdata *ep;
	register struct inode *ip;
	register unsigned ds, ts;
	register struct user *up;
	struct exdata exdata;

	struct	naout	{
		short	magic;
		short	vstamp;
		long	tsize,
			dsize,
			bsize,
			entry,
			ts,
			ds;
	};

	struct	filhd	{
		unsigned short	magic;
		unsigned short	nscns;
		long		timdat,
				symptr,
				nsyms;
		unsigned short	opthdr,
				flags;
	} ;

	struct	scnhdr	{
		char		s_name[8];
		long		s_paddr,
				s_vaddr,
				s_size,
				s_scnptr,
				s_relptr,
				s_lnnoptr;
		unsigned short	s_nreloc,
				s_nlnno;
		long		s_flags;
	};

	if ((ip = namei(uchar, 0)) == NULL)
		return(NULL);
	up = &u;
	if (access(ip, IEXEC) ||
	   (ip->i_mode & IFMT) != IFREG ||
	   (ip->i_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) {
		up->u_error = EACCES;
		goto bad;
	}
	/*
	 * read in first few bytes of file for segment sizes
	 * ux_mag = 407/410/411
	 *  407 is plain executable
	 *  410 is RO text
	 *  411 is separated ID
	 *  570 Common object
	 *  575 "
	 *  set ux_tstart to start of text portion
	 */
	ep = &exdata;
	up->u_base = (caddr_t)ep;
	up->u_count = sizeof(*ep);
	up->u_offset = 0;
	up->u_segflg = 1;
	readi(ip);
#ifdef notdef
	if (ep->ux_mag == 0570 || ep->ux_mag == 0575) {
		up->u_base = (caddr_t)ep;
		up->u_count = sizeof(*ep);
		up->u_offset = sizeof(struct filhd);
		up->u_segflg = 1;
		readi(ip);
		ep->ux_tstart = sizeof(struct naout) +
			sizeof(struct filhd) + (3 * sizeof(struct scnhdr));
		ep->ux_entloc = ep->ux_ssize;
	} else {
		ep->ux_tstart = sizeof(up->u_exdata);
	}
#endif
	up->u_segflg = 0;
	if (up->u_count!=0)
		ep->ux_mag = 0;
	if (ep->ux_mag == 0407) {
		ds = btoc((long)ep->ux_tsize +
			(long)ep->ux_dsize + (long)ep->ux_bsize);
		ts = 0;
		ep->ux_dsize += ep->ux_tsize;
		ep->ux_tsize = 0;
	} else {
		ts = btoc(ep->ux_tsize);
		ds = btoc(ep->ux_dsize+ep->ux_bsize);
		if ((ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
			register struct file *fp;

			for (fp = file; fp < (struct file *)v.ve_file; fp++)
				if (fp->f_count && fp->f_inode == ip &&
				    (fp->f_flag&FWRITE)) {
					up->u_error = ETXTBSY;
					goto bad;
				}
		}
		if (ep->ux_mag != 0410) {
			up->u_error = ENOEXEC;
			goto bad;
		}
	}
	(void) chksize(ts, ds, (unsigned)(SSIZE+btoc(NCARGS-1)));
	up->u_exdata = exdata;
bad:
	if (up->u_error) {
		iput(ip);
		ip = NULL;
	}
	return(ip);
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(ip, nargc)
register struct inode *ip;
{
	register struct exdata *ep;
	register struct user *up;
	register struct proc *p;
	register unsigned ts, ds, ss;
	register i;

	up = &u;
	p = up->u_procp;
	ep = &up->u_exdata;
#ifdef HOWFAR
	printf("getxfile:reading in program\n");
#endif
	shmexec();
	(void) punlock();
	xfree();
	ts = btoc(ep->ux_tsize);
	ds = btoc(ep->ux_dsize + ep->ux_bsize);
	ss = SSIZE + v.v_usize + btoc(nargc);
	i = v.v_usize+ds+ss;
	expand(i);
	(void) estabur((unsigned)0, ds, ss, 0, RO);
	procclear(p);
	/*
	while (--i >= v.v_usize)
		clearseg((int)(p->p_addr+i));
	*/
	xalloc(ip);
	up->u_prof.pr_scale = 0;

	/*
	 * read in data segment
	 */

	(void) estabur(ts, ds, (unsigned)0, 0, RO);
	if (v.v_doffset)
		up->u_base = (caddr_t)v.v_doffset;
	else
		up->u_base = (caddr_t)(v.v_ustart + ctob(stoc(ctos(ts))));
	up->u_offset = sizeof(up->u_exdata) + ep->ux_tsize;
	up->u_count = ep->ux_dsize;
	readi(ip);
	if (up->u_count!=0)
		up->u_error = EFAULT;

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((p->p_flag&STRC)==0) {
		if (ip->i_mode&ISUID)
			up->u_uid = ip->i_uid;
		if (ip->i_mode&ISGID)
			up->u_gid = ip->i_gid;
		p->p_suid = up->u_uid;
	} else
		psignal(p, SIGTRAP);
	up->u_tsize = ts;
	up->u_dsize = ds;
	up->u_ssize = ss;
	(void) estabur(ts, ds, ss, 0, RO);
}

/*
 * Clear registers on exec
 */
setregs()
{
	register struct user *up;
	register char *cp;
	register int *rp;
	register i;

	up = &u;
	for (rp = &up->u_signal[0]; rp < &up->u_signal[NSIG]; rp++)
		if ((*rp & 1) == 0)
			*rp = 0;
	for (cp = &regloc[0]; cp < &regloc[15]; )
		up->u_ar0[*cp++] = 0;
	up->u_ar0[PC] = up->u_exdata.ux_entloc & ~01;
#ifdef HOWFAR
	printf("New pc = 0x%x\n", up->u_ar0[PC]);
#endif

	for (i=0; i<NOFILE; i++) {
		if ((up->u_pofile[i]&EXCLOSE) && up->u_ofile[i] != NULL) {
			closef(up->u_ofile[i]);
			up->u_ofile[i] = NULL;
		}
	}
	/*
	 * Remember file name for accounting.
	 */
	up->u_acflag &= ~AFORK;
	bcopy((caddr_t)up->u_dent.d_name, (caddr_t)up->u_comm, DIRSIZ);
}

/*
 * clear the data space for a process
 */
#ifdef NONSCATLOAD
procclear(p)
struct proc *p;
{
	register i, a;

	a = p->p_addr;
	i = p->p_size;
	while(--i >= v.v_usize)
		clearseg(a+i);
}
#else
procclear(p)
struct proc *p;
{
	register struct scatter *s;
	register i, a1;

	s = scatmap;
	a1 = p->p_scat;
	for (i=0; i<v.v_usize; i++)
		a1 = s[a1].sc_index;
	while (a1 != SCATEND) {
		clearseg(ixtoc(a1));
		a1 = s[a1].sc_index;
	}
}
#endif

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
{
	register struct user *up;
	register int i;
	register struct proc *p, *q;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	up = &u;
	p = up->u_procp;
	p->p_flag &= ~(STRC);
	p->p_clktim = 0;
	for (i=0; i<NSIG; i++)
		up->u_signal[i] = 1;
	expand(v.v_usize);
	(void) estabur((unsigned)0, (unsigned)0, (unsigned)0, 0, RO);
	if ((p->p_pid == p->p_pgrp)
	 && (up->u_ttyp != NULL)
	 && (*up->u_ttyp == p->p_pgrp)) {
		*up->u_ttyp = 0;
		signal(p->p_pgrp, SIGHUP);
	}
	p->p_pgrp = 0;
	for (i=0; i<NOFILE; i++) {
		if (up->u_ofile[i] != NULL)
			closef(up->u_ofile[i]);
	}
	(void) punlock();
	plock(up->u_cdir);
	iput(up->u_cdir);
	if (up->u_rdir) {
		plock(up->u_rdir);
		iput(up->u_rdir);
	}

#ifdef FLOAT		/* sky floating point board */
	/*
	 *	If this process was using the SKY FFP, restore
	 *	the board to a known state.
	 */
	if (up->u_fpinuse)
		savfp();
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	/*
	 * If there is an MC68881, save the internal state and user
	 * registers so they'll be available in a core image.
	 * Then reset the coprocessor by restoring it to a null state.
	 */
	if (fp881) {
		fpsave();
		fpreset();
	}
#endif mc68881

	/*
	 * call OEM supplied subroutine on process exit
	 */
	oemexit(p);

	xfree();

	semexit();
	shmexit();

	acct(rv);
#ifdef NONSCATLOAD
	mfree(coremap, p->p_size, p->p_addr);
#else
	memfree(p->p_scat);
	p->p_scat = SCATEND;
#endif
	cxrelse(p->p_context);
	p->p_stat = SZOMB;
	((struct xproc *)p)->xp_xstat = rv;
	((struct xproc *)p)->xp_utime = up->u_cutime + up->u_utime;
	((struct xproc *)p)->xp_stime = up->u_cstime + up->u_stime;
	for (q = &proc[1]; q < (struct proc *)v.ve_proc; q++) {
		if (p->p_pid == q->p_ppid) {
			q->p_ppid = 1;
			if (q->p_stat == SZOMB)
				psignal(&proc[1], SIGCLD);
			if (q->p_stat == SSTOP)
				setrun(q);
		} else
		if (p->p_ppid == q->p_pid)
			psignal(q, SIGCLD);
		if (p->p_pid == q->p_pgrp)
			q->p_pgrp = 0;
	}
#ifdef NONSCATLOAD
	resume(proc[0].p_addr, up->u_qsav);
#else
	resume(ixtoc(proc[0].p_scat), up->u_qsav);
#endif
	/* no deposit, no return */
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register struct user *up;
	register f;
	register struct proc *p;

	up = &u;
loop:
	f = 0;
	for (p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
	if (p->p_ppid == up->u_procp->p_pid) {
		f++;
		if (p->p_stat == SZOMB) {
			freeproc(p, 1);
			return;
		}
		if (p->p_stat == SSTOP) {
			if ((p->p_flag&SWTED) == 0) {
				p->p_flag |= SWTED;
				up->u_rval1 = p->p_pid;
				up->u_rval2 = (fsig(p)<<8) | 0177;
				return;
			}
			continue;
		}
	}
	if (f) {
		(void) sleep((caddr_t)up->u_procp, PWAIT);
		goto loop;
	}
	up->u_error = ECHILD;
}

/*
 * Remove zombie children from the process table.
 */
freeproc(p, flag)
register struct proc *p;
{
	register struct user *up;

	up = &u;
	if (flag) {
		register n;

		n = up->u_procp->p_cpu + p->p_cpu;
		if (n > 80)
			n = 80;
		up->u_procp->p_cpu = n;
		up->u_rval1 = p->p_pid;
		up->u_rval2 = ((struct xproc *)p)->xp_xstat;
	}
	up->u_cutime += ((struct xproc *)p)->xp_utime;
	up->u_cstime += ((struct xproc *)p)->xp_stime;
	p->p_stat = NULL;
	p->p_pid = 0;
	p->p_ppid = 0;
	p->p_sig = 0L;
	p->p_flag = 0;
	p->p_wchan = 0;
}

/*
 * fork system call.
 */
fork()
{
	register struct user *up;
	register a, i;
	int sz, xsize[NSCATSWAP], xaddr[NSCATSWAP];

	up = &u;
	sysinfo.sysfork++;
	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot; or
	 *  not su and no space on swap.
	 * Part of check done in newproc().
	 */
	if (up->u_uid && up->u_ruid) {
		if ((a = malloc(swapmap, ctod(maxmem))) == NULL) {
			sz = ctod(maxmem);
			for (i = 0; i < NSCATSWAP; i++)
				if (sz == 0) {
					xsize[i] = 0;
					break;
				} else {
					a = MIN(mallocl(swapmap), sz);
					xsize[i] = a;
					xaddr[i] = malloc(swapmap, a);
					sz -= a;
				}
			for (i = 0; i < NSCATSWAP; i++)
				if (xsize[i] == 0)
					break;
				else
					mfree(swapmap, xsize[i], xaddr[i]);
			if (sz != 0) {
				printf("Not enough swap space to fork\n");
				up->u_error = ENOMEM;
				goto out;
			}
		} else
			mfree(swapmap, ctod(maxmem), a);
	}
	switch( newproc(1) ) {
		case 1: /* child  -- successful newproc */
			up->u_rval1 = up->u_procp->p_ppid;
			up->u_rval2 = 1;  /* child */
			up->u_start = time;
			up->u_ticks = lbolt;
			up->u_mem = v.v_usize + procsize(up->u_procp);
			up->u_ior = 0;
			up->u_iow = 0;
			up->u_ioch = 0;
			up->u_cstime = 0;
			up->u_stime = 0;
			up->u_cutime = 0;
			up->u_utime = 0;
			up->u_acflag = AFORK;
			return;
		case 0: /* parent -- successful newproc */
			/* up->u_rval1 = pid-of-child; */
			break;
		default: /* unsuccessful newproc */
			up->u_error = EAGAIN;
			break;
	}
out:
	up->u_rval2 = 0; /* parent */
	up->u_ar0[PC] += 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
#ifdef NONSCATLOAD
sbreak()
{
	register struct user *up;
	struct a {
		unsigned nsiz;
	};
	register n, d, a;
	int i;

	up = &u;
	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	if (v.v_doffset)
		n = btoc((int)(((struct a *)up->u_ap)->nsiz)) -
			btoc(v.v_doffset);
	else {
		n = btoc((int)(((struct a *)up->u_ap)->nsiz)) -
			btoc(v.v_ustart);
		n -= stoc(ctos(up->u_tsize));
	}
	if (n < 0)
		n = 0;
	d = n - up->u_dsize;
	if (d == 0)
		return;
	n += v.v_usize+up->u_ssize;
	if (chksize(up->u_tsize, up->u_dsize+d, up->u_ssize))
		return;
	up->u_dsize += d;
	(void) estabur(up->u_tsize, up->u_dsize, up->u_ssize, 0, RO);
	if (d > 0)
		goto bigger;
	a = up->u_procp->p_addr + n - up->u_ssize;
	i = n;
	if (d < 0) {
		n = up->u_ssize;
		while (n--) {
			copyseg(a-d, a);
			a++;
		}
	}
	expand(i);
	return;

bigger:
	expand(n);
	a = up->u_procp->p_addr + n;
	n = up->u_ssize;
	while (n--) {
		a--;
		copyseg(a-d, a);
	}
	while (d--)
		clearseg(--a);
}
#else
sbreak()
{
	register struct scatter *s;
	register struct user *up;
	struct a {
		unsigned nsiz;
	};
	register n, d, a1, a2;
	int i;
	short t;

	up = &u;
	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = btoc((int)(((struct a *)up->u_ap)->nsiz)) - btoc(v.v_ustart);
	n -= stoc(ctos(up->u_tsize));
	if (n < 0)
		n = 0;
	d = n - up->u_dsize;
	if (d == 0)
		return;
	n += v.v_usize+up->u_ssize;
	if (chksize(up->u_tsize, up->u_dsize+d, up->u_ssize))
		return;
	s = scatmap;
	up->u_dsize += d;
	if (d > 0)
		goto bigger;
	nscatfree -= d;		/* note: d is negative */
	up->u_procp->p_size = n;
	a1 = up->u_procp->p_scat;
	n = up->u_dsize + v.v_usize;
	for (i=1; i<n; i++)
		a1 = s[a1].sc_index;
	a2 = a1;
	while (d++ < 0)
		a2 = s[a2].sc_index;
	t = scatfreelist.sc_index;
	scatfreelist.sc_index = s[a1].sc_index;
	s[a1].sc_index = s[a2].sc_index;
	s[a2].sc_index = t;
	(void) estabur(up->u_tsize, up->u_dsize, up->u_ssize, 0, RO);
#ifdef OLD
	a = up->u_procp->p_addr + n - up->u_ssize;
	i = n;
	if (d < 0) {
		n = up->u_ssize;
		while (n--) {
			copyseg(a-d, a);
			a++;
		}
	}
	expand(i);
#endif
	up->u_procp->p_flag &= ~SCONTIG;
	return;

bigger:
	expand(n);
	a1 = up->u_procp->p_scat;
	/*
	 * find last index of original data space
	 */
	n = up->u_dsize + v.v_usize - d;
	if (n == 0)
		printf("sbreak:original size is zero\n");
	for (i=1; i<n; i++)
		a1 = s[a1].sc_index;
	/*
	 * move stack if necessary
	 */
	if (up->u_ssize !=0 && (int)(up->u_dsize-d) <= 0)
		printf("sbreak:bigger: unusual condition #1\n");
	if (up->u_ssize == 0) {
		while (d-- > 0)
			clearseg(ixtoc(a1 = s[a1].sc_index));
		if (s[a1].sc_index != SCATEND)
			printf("sbreak:not at end of list\n");
	} else {
		/*
		 * find end of original stack space
		 */
		a2 = a1;
		for (i=0; i<up->u_ssize; i++)
			a2 = s[a2].sc_index;
		t = s[a1].sc_index;
		s[a1].sc_index = s[a2].sc_index;
		s[a2].sc_index = SCATEND;
		while (d-- > 0)
			clearseg(ixtoc(a1 = s[a1].sc_index));
		s[a1].sc_index = t;
	}
	(void) estabur(up->u_tsize, up->u_dsize, up->u_ssize, 0, RO);
	up->u_procp->p_flag &= ~SCONTIG;
#ifdef OLD
	a = up->u_procp->p_addr + n;
	n = up->u_ssize;
	while (n--) {
		a--;
		copyseg(a-d, a);
	}
	while (d--)
		clearseg(--a);
#endif
}
