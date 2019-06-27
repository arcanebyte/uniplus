/* @(#)sys2.c	1.6 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/sysinfo.h"
#ifdef UCB_NET
#include "net/misc.h"
#include "net/socket.h"
#include "net/socketvar.h"
#endif

/*
 * read system call
 */
read()
{
	sysinfo.sysread++;
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	sysinfo.syswrite++;
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi or writei code.
 */
rdwr(mode)
register mode;
{
	register struct user *up;
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;
	register int type;

	up = &u;
	uap = (struct a *)up->u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	if ((fp->f_flag&mode) == 0) {
		up->u_error = EBADF;
		return;
	}
	up->u_base = (caddr_t)uap->cbuf;
	up->u_count = uap->count;
	up->u_segflg = 0;
	up->u_fmode = fp->f_flag;
	ip = fp->f_inode;
	type = ip->i_mode&IFMT;

	/*
	 *	Fix from ROOT:  check for file lock before attempting
	 *	to lock the inode.
	 */
	if (type==IFREG) {
		if ((up->u_fmode&FAPPEND) && (mode == FWRITE))
			fp->f_offset = ip->i_size;
		up->u_offset	= fp->f_offset;
		if (ip->i_locklist &&
		    locked(1, ip, up->u_offset,
		        (off_t)(up->u_offset+up->u_count)))
			return;
	}
#ifdef  UCB_NET
	if (fp->f_flag & FSOCKET) {
		if (mode == FREAD)
			u.u_error = soreceive((struct socket *)fp->f_socket, (struct sockaddr *)0);
		else
			u.u_error = sosend((struct socket *)fp->f_socket, (struct sockaddr *)0);
	} else
#endif
	{
	if (type==IFREG || type==IFDIR) {
		plock(ip);
		if ((up->u_fmode&FAPPEND) && (mode == FWRITE))
			fp->f_offset = ip->i_size;
	} else if (type == IFIFO) {
		plock(ip);
		fp->f_offset = 0;
	}
	up->u_offset = fp->f_offset;
	if (mode == FREAD)
		readi(ip);
	else
		writei(ip);
	if (type==IFREG || type==IFDIR || type==IFIFO)
		prele(ip);
	fp->f_offset += uap->count-up->u_count;
	}
	up->u_rval1 = uap->count-up->u_count;
	up->u_ioch += (unsigned)up->u_rval1;
	if (mode == FREAD)
		sysinfo.readch += (unsigned)up->u_rval1;
	else
		sysinfo.writech += (unsigned)up->u_rval1;
}

/*
 * open system call
 */
open()
{
	register struct a {
		char	*fname;
		int	mode;
		int	crtmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	copen(uap->mode-FOPEN, uap->crtmode);
}

/*
 * creat system call
 */
creat()
{
	struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	copen(FWRITE|FCREAT|FTRUNC, uap->fmode);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
copen(mode, arg)
register mode;
{
	register struct user *up;
	register struct inode *ip;
	register struct file *fp;
	int i;

	up = &u;
	if ((mode&(FREAD|FWRITE)) == 0) {
		up->u_error = EINVAL;
		return;
	}
	if (mode&FCREAT) {
		ip = namei(uchar, 1);
		if (ip == NULL) {
			for (i=0; i<NOFILE; i++)
				if (up->u_ofile[i] == NULL)
					break;
			if (i >= NOFILE) {
				iput(u.u_pdir);
				up->u_error = EMFILE;
			}
			if (up->u_error)
				return;
			ip = maknode(arg&07777&(~ISVTX));
			if (ip == NULL)
				return;
			mode &= ~FTRUNC;
		} else {
			if (ip->i_locklist != NULL &&
			      (ip->i_flag&IFMT) == IFREG &&
			      locked(2, ip, (long)(0L), (long)(1L<<30))) {
				iput(ip);
				return;
			}
			if (mode&FEXCL) {
				up->u_error = EEXIST;
				iput(ip);
				return;
			}
			mode &= ~FCREAT;
		}
#ifndef VIRTUAL451
		xmfree(ip);
#endif
	} else {
		ip = namei(uchar, 0);
		if (ip == NULL)
			return;
	}
	if (!(mode&FCREAT)) {
		if (mode&FREAD)
			(void) access(ip, IREAD);
		if (mode&(FWRITE|FTRUNC)) {
			(void) access(ip, IWRITE);
			if ((ip->i_mode&IFMT) == IFDIR)
				up->u_error = EISDIR;
		}
	}
	if (up->u_error || (fp = falloc(ip, mode&FMASK)) == NULL) {
		iput(ip);
		return;
	}
	if (mode&FTRUNC)
		itrunc(ip);
	prele(ip);
	i = up->u_rval1;
	if (save(up->u_qsav)) {	/* catch half-opens */
		if (up->u_error == 0)
			up->u_error = EINTR;
		up->u_ofile[i] = NULL;
#ifdef UCB_NET
		fp->f_flag |= FISUSER;
#endif
		closef(fp);
	} else {
		openi(ip, mode);
		if (up->u_error == 0)
			return;
		up->u_ofile[i] = NULL;
		if ((--fp->f_count) <= 0) {
			fp->f_next = ffreelist;
			ffreelist = fp;
		}
		iput(ip);
	}
}

/*
 * close system call
 */
close()
{
	register struct file *fp;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	u.u_ofile[uap->fdes] = NULL;
#ifdef UCB_NET		/* so sockets close correctly */
	fp->f_flag |= FISUSER;
#endif
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
#ifdef  UCB_NET
	if (fp->f_flag&FSOCKET) {
		u.u_error = ESPIPE;
		return;
	}
#endif
	ip = fp->f_inode;
	if ((ip->i_mode&IFMT)==IFIFO) {
		up->u_error = ESPIPE;
		return;
	}
	if (uap->sbase == 1)
		uap->off += fp->f_offset;
	else if (uap->sbase == 2)
		uap->off += fp->f_inode->i_size;
	else if (uap->sbase != 0) {
		up->u_error = EINVAL;
		psignal(up->u_procp, SIGSYS);
		return;
	}
	if (uap->off < 0) {
		up->u_error = EINVAL;
		return;
	}
	fp->f_offset = uap->off;
	up->u_roff = uap->off;
}

/*
 * link system call
 */
link()
{
	register struct user *up;
	register struct inode *ip, *xp;
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;

	up = &u;
	uap = (struct a *)up->u_ap;
	ip = namei(uchar, 0);
	if (ip == NULL)
		return;
	if (ip->i_nlink >= MAXLINK) {
		up->u_error = EMLINK;
		goto outn;
	}
	if ((ip->i_mode&IFMT)==IFDIR && !suser())
		goto outn;
	/*
	 * Unlock to avoid possibly hanging the namei.
	 * Sadly, this means races. (Suppose someone
	 * deletes the file in the meantime?)
	 * Nor can it be locked again later
	 * because then there will be deadly
	 * embraces.
	 * Update inode first for robustness.
	 */
	ip->i_nlink++;
	ip->i_flag |= ICHG|ISYN;
	iupdat(ip, &time, &time);
	prele(ip);
	up->u_dirp = (caddr_t)uap->linkname;
	xp = namei(uchar, 1);
	if (xp != NULL) {
		iput(xp);
		up->u_error = EEXIST;
		goto out;
	}
	if (up->u_error)
		goto out;
	if (up->u_pdir->i_dev != ip->i_dev) {
		iput(up->u_pdir);
		up->u_error = EXDEV;
		goto out;
	}
	wdir(ip);
out:
	plock(ip);
	if (up->u_error) {
		ip->i_nlink--;
		ip->i_flag |= ICHG;
	}
outn:
	iput(ip);
	return;
}

/*
 * mknod system call
 */
mknod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
		int	dev;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((uap->fmode&IFMT) != IFIFO && !suser())
		return;
	ip = namei(uchar, 1);
	if (ip != NULL) {
		iput(ip);
		u.u_error = EEXIST;
		return;
	}
	if (u.u_error)
		return;
	ip = maknode(uap->fmode);
	if (ip == NULL)
		return;
	switch(ip->i_mode&IFMT) {
	case IFCHR:
	case IFBLK:
		ip->i_rdev = (dev_t)uap->dev;
		ip->i_flag |= ICHG;
	}

	iput(ip);
}

/*
 * access system call
 */
saccess()
{
	register struct user *up;
	register svuid, svgid;
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	up = &u;
	uap = (struct a *)up->u_ap;
	svuid = up->u_uid;
	svgid = up->u_gid;
	up->u_uid = up->u_ruid;
	up->u_gid = up->u_rgid;
	ip = namei(uchar, 0);
	if (ip != NULL) {
		if (uap->fmode&(IREAD>>6))
			(void) access(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			(void) access(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			(void) access(ip, IEXEC);
		iput(ip);
	}
	up->u_uid = svuid;
	up->u_gid = svgid;
}
