/* @(#)sys4.c	1.5 */
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
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/filsys.h"
#include "sys/proc.h"
#include "sys/var.h"

/*
 * Everything in this file is a routine implementing a system call.
 */

gtime()
{
	u.u_rtime = time;
}

stime()
{
	register struct a {
		time_t	time;
	} *uap;
	struct filsys *fp;

	uap = (struct a *)u.u_ap;
	if (suser()) {
		logtchg(uap->time);
		time = uap->time;
		if (fp = getfs(rootdev))
			fp->s_fmod = 1;
	}
}

setuid()
{
	register unsigned uid;
	register struct a {
		int	uid;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	uid = uap->uid;
	if (uid >= MAXUID) {
		up->u_error = EINVAL;
		return;
	}
	if (uid && (uid == up->u_ruid || uid == up->u_procp->p_suid))
		up->u_uid = uid;
	else if (suser()) {
		up->u_uid = uid;
		up->u_procp->p_uid = uid;
		up->u_procp->p_suid = uid;
		up->u_ruid = uid;
	}
}

getuid()
{
	register struct user *up;

	up = &u;
	up->u_rval1 = up->u_ruid;
	up->u_rval2 = up->u_uid;
}

setgid()
{
	register unsigned gid;
	register struct a {
		int	gid;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	gid = uap->gid;
	if (gid >= MAXUID) {
		up->u_error = EINVAL;
		return;
	}
	if (up->u_rgid == gid || suser()) {
		up->u_gid = gid;
		up->u_rgid = gid;
	}
}

getgid()
{
	register struct user *up;

	up = &u;
	up->u_rval1 = up->u_rgid;
	up->u_rval2 = up->u_gid;
}

getpid()
{
	register struct user *up;

	up = &u;
	up->u_rval1 = up->u_procp->p_pid;
	up->u_rval2 = up->u_procp->p_ppid;
}

setpgrp()
{
	register struct proc *p = u.u_procp;
	register struct a {
		int	flag;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->flag) {
		if (p->p_pgrp != p->p_pid)
			u.u_ttyp = NULL;
		p->p_pgrp = p->p_pid;
	}
	u.u_rval1 = p->p_pgrp;
}

sync()
{

	update();
}

nice()
{
	register n;
	register struct a {
		int	niceness;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	n = uap->niceness;
	if ((n < 0 || n > 2*NZERO) && !suser())
		n = 0;
	n += up->u_procp->p_nice;
	if (n >= 2*NZERO)
		n = 2*NZERO -1;
	if (n < 0)
		n = 0;
	up->u_procp->p_nice = n;
	up->u_rval1 = n - NZERO;
}

/*
 * Unlink system call.
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
unlink()
{
	register struct inode *ip, *pp;
	struct a {
		char	*fname;
	};
	register struct user *up;

	up = &u;
	pp = namei(uchar, 2);
	if (pp == NULL)
		return;
	/*
	 * Check for unlink(".")
	 * to avoid hanging on the iget
	 */
	if (pp->i_number == up->u_dent.d_ino) {
		ip = pp;
		ip->i_count++;
	} else
		ip = iget(pp->i_dev, up->u_dent.d_ino);
	if (ip == NULL)
		goto out1;
	if ((ip->i_mode&IFMT) == IFDIR && !suser())
		goto out;
	/*
	 * Don't unlink a mounted file.
	 */
	if (ip->i_dev != pp->i_dev) {
		up->u_error = EBUSY;
		goto out;
	}
	if (ip->i_flag&ITEXT)
		xrele(ip);	/* try once to free text */
	if (ip->i_flag&ITEXT && ip->i_nlink == 1) {
		up->u_error = ETXTBSY;
		goto out;
	}
	up->u_offset -= sizeof(struct direct);
	up->u_base = (caddr_t)&up->u_dent;
	up->u_count = sizeof(struct direct);
	up->u_dent.d_ino = 0;
	up->u_segflg = 1;
	up->u_fmode = FWRITE|FSYNC;
	writei(pp);
	if (up->u_error)
		goto out;
	ip->i_nlink--;
	ip->i_flag |= ICHG;

out:
	iput(ip);
out1:
	iput(pp);
}

chdir()
{
	chdirec(&u.u_cdir);
}

chroot()
{
	if (!suser())
		return;
	chdirec(&u.u_rdir);
}

chdirec(ipp)
register struct inode **ipp;
{
	register struct inode *ip;
	struct a {
		char	*fname;
	};

	ip = namei(uchar, 0);
	if (ip == NULL)
		return;
	if ((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	if (access(ip, IEXEC))
		goto bad;
	prele(ip);
	if (*ipp) {
		plock(*ipp);
		iput(*ipp);
	}
	*ipp = ip;
	return;

bad:
	iput(ip);
}

chmod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((ip = owner()) == NULL)
		return;
	ip->i_mode &= ~07777;
	if (u.u_uid) {
		uap->fmode &= ~ISVTX;
		if (u.u_gid != ip->i_gid)
			uap->fmode &= ~ISGID;
	}
	ip->i_mode |= uap->fmode&07777;
	ip->i_flag |= ICHG;
	if (ip->i_flag&ITEXT && (ip->i_mode&ISVTX)==0)
		xrele(ip);
	iput(ip);
}

chown()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	uid;
		int	gid;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((ip = owner()) == NULL)
		return;
	ip->i_uid = uap->uid;
	ip->i_gid = uap->gid;
	if (u.u_uid != 0)
		ip->i_mode &= ~(ISUID|ISGID);
	ip->i_flag |= ICHG;
	iput(ip);
}

ssig()
{
	register a;
	register struct proc *p;
	struct a {
		int	signo;
		int	fun;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	a = uap->signo;
	if (a <= 0 || a > NSIG || a == SIGKILL) {
		up->u_error = EINVAL;
		return;
	}
	up->u_rval1 = up->u_signal[a-1];
	up->u_signal[a-1] = uap->fun;
	up->u_procp->p_sig &= ~(1L<<(a-1));
	if (a == SIGCLD) {
		a = up->u_procp->p_pid;
		for (p = &proc[1]; p < (struct proc *)v.ve_proc; p++) {
			if (a == p->p_ppid && p->p_stat == SZOMB)
				psignal(up->u_procp, SIGCLD);
		}
	}
	if (uap->fun&1)
		up->u_procp->p_sigign |= (1<<(uap->signo-1));
	else
		up->u_procp->p_sigign &= ~(1<<(uap->signo-1));
}

kill()
{
	register struct proc *p, *q;
	register arg;
	register struct a {
		int	pid;
		int	signo;
	} *uap;
	int f;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	if (uap->signo < 0 || uap->signo > NSIG) {
		up->u_error = EINVAL;
		return;
	}
	/* Prevent proc 1 (init) from being SIGKILLed */
	if (uap->signo == SIGKILL && uap->pid == 1) {
		up->u_error = EINVAL;
		return;
	}
	f = 0;
	arg = uap->pid;
	if (arg > 0)
		p = &proc[1];
	else
		p = &proc[2];
	q = up->u_procp;
	if (arg == 0 && q->p_pgrp == 0) {
		up->u_error = ESRCH;
		return;
	}
	for(; p < (struct proc *)v.ve_proc; p++) {
		if (p->p_stat == NULL)
			continue;
		if (arg > 0 && p->p_pid != arg)
			continue;
		if (arg == 0 && p->p_pgrp != q->p_pgrp)
			continue;
		if (arg < -1 && p->p_pgrp != -arg)
			continue;
		if (! (up->u_uid == 0 ||
			up->u_uid == p->p_uid ||
			up->u_ruid == p->p_uid ||
			up->u_uid == p->p_suid ||
			up->u_ruid == p->p_suid ))
			if (arg > 0) {
				up->u_error = EPERM;
				return;
			} else
				continue;
		f++;
		if (uap->signo)
			psignal(p, uap->signo);
		if (arg > 0)
			break;
	}
	if (f == 0)
		up->u_error = ESRCH;
}

times()
{
	register struct a {
		time_t	(*times)[4];
	} *uap;
	register struct user *up;
	time_t loctime[4];

	up = &u;
	uap = (struct a *)up->u_ap;
	if (v.v_hz==60) {
		if (copyout((caddr_t)&up->u_utime, (caddr_t)uap->times,
		    sizeof(*uap->times)))
			up->u_error = EFAULT;
		SPL7();
		up->u_rtime = lbolt;
		SPL0();
	} else {
		loctime[0] = up->u_utime * 60 / v.v_hz;
		loctime[1] = up->u_stime * 60 / v.v_hz;
		loctime[2] = up->u_cutime * 60 / v.v_hz;
		loctime[3] = up->u_cstime * 60 / v.v_hz;
		if (copyout((caddr_t)&loctime[0], (caddr_t)uap->times,
		    sizeof(loctime)) < 0)
			up->u_error = EFAULT;
		SPL7();
		up->u_rtime = lbolt*60/v.v_hz;
		SPL0();
	}
}

profil()
{
	register struct a {
		short	*bufbase;
		unsigned bufsize;
		unsigned pcoffset;
		unsigned pcscale;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	up->u_prof.pr_base = uap->bufbase;
	up->u_prof.pr_size = uap->bufsize;
	up->u_prof.pr_off = uap->pcoffset;
	up->u_prof.pr_scale = uap->pcscale;
}

/*
 * alarm clock signal
 */
alarm()
{
	register struct proc *p;
	register c;
	register struct a {
		int	deltat;
	} *uap;

	uap = (struct a *)u.u_ap;
	p = u.u_procp;
	c = p->p_clktim;
	p->p_clktim = uap->deltat;
	u.u_rval1 = c;
}

/*
 * indefinite wait.
 * no one should wakeup(&u)
 */
pause()
{

	for(;;)
		(void) sleep((caddr_t)&u, PSLEP);
}

/*
 * mode mask for creation of files
 */
umask()
{
	register struct a {
		int	mask;
	} *uap;
	register t;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	t = up->u_cmask;
	up->u_cmask = uap->mask & 0777;
	up->u_rval1 = t;
}

/*
 * Set IUPD and IACC times on file.
 */
utime()
{
	register struct a {
		char	*fname;
		time_t	*tptr;
	} *uap;
	register struct inode *ip;
	time_t tv[2];

	uap = (struct a *)u.u_ap;
	if (uap->tptr != NULL) {
		if (copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof(tv))) {
			u.u_error = EFAULT;
			return;
		}
	} else {
		tv[0] = time;
		tv[1] = time;
	}
	ip = namei(uchar, 0);
	if (ip == NULL)
		return;
	if (u.u_uid != ip->i_uid && u.u_uid != 0) {
		if (uap->tptr != NULL)
			u.u_error = EPERM;
		else
			(void) access(ip, IWRITE);
	}
	if (!u.u_error) {
		ip->i_flag |= IACC|IUPD|ICHG;
		iupdat(ip, &tv[0], &tv[1]);
	}
	iput(ip);
}

ulimit()
{
	register struct a {
		int	cmd;
		long	arg;
	} *uap;
	register struct user *up;
	register n, ts;

	up = &u;
	uap = (struct a *)up->u_ap;
	switch(uap->cmd) {
	case 2:
		if (uap->arg > up->u_limit && !suser())
			return;
		up->u_limit = uap->arg;
	case 1:
		up->u_roff = up->u_limit;
		break;

	case 3:
		ts = up->u_tsize;
		n = maxmem - up->u_ssize - v.v_usize;
		if (ts > minmem)
			n = MAX((int)n-ts, (int)minmem-up->u_ssize-v.v_usize);
		n = MIN(n, MAXMEM-stoc(ctos(ts))-stoc(ctos(up->u_ssize)));
		up->u_roff = v.v_ustart + ctob(stoc(ctos(ts)) + n);
		break;

	default:
		up->u_error = EINVAL;
	}
}

/*
 * phys - Set up a physical address in user's address space.
 */
phys()
{
	register struct a {
		unsigned phnum;		/* phys segment number */
		unsigned laddr;		/* logical address */
		unsigned bcount;	/* byte count */
		unsigned phaddr;	/* physical address */
	} *uap;

	if (!suser()) return;
	uap = (struct a *)u.u_ap;
	dophys(uap->phnum, uap->laddr, uap->bcount, uap->phaddr);
}

/*
 * reboot the system
 */
reboot()
{
	register i;

	update();
	for (i = 0; i < 1000000; i++)
		;
	doboot();
}
