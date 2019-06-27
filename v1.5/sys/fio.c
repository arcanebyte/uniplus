/* @(#)fio.c	1.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/filsys.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#ifdef UCB_NET
#include "net/misc.h"
#include "net/socketvar.h"
#endif

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.
 * Only task is to check range of the descriptor.
 */
struct file *
getf(f)
register int f;
{
	register struct file *fp;

	if (0 <= f && f < NOFILE) {
		fp = u.u_ofile[f];
		if (fp != NULL)
#ifdef UCB_NET
		/* Mainly for net reset */
		if (fp->f_count == 0) {
		    u.u_error = ENETDOWN;
		    return (NULL);
		}
		else
#endif
			return(fp);
	}
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 * Also make sure the pipe protocol does not constipate.
 *
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 * On the last close switch out to the device handler for
 * special files.  Note that the handler is called
 * on every open but only the last close.
 */
closef(fp)
register struct file *fp;
{
	register struct inode *ip;
	int flag, fmt;
	dev_t dev;
	register int (*cfunc)();

	if (fp == NULL)
		return;
	flag = fp->f_flag;
#ifdef UCB_NET
	if ((flag & FSOCKET) == 0)
#endif
		unlock(fp->f_inode);		/* file locking hook */
	if ((unsigned)fp->f_count > 1) {
#ifdef UCB_NET
		fp->f_flag &= ~FISUSER;	
#endif
		fp->f_count--;
		return;
	}
#ifdef UCB_NET
	if (flag & FSOCKET) {
		int nouser = ((flag & FISUSER) == 0);

		u.u_error = 0;			/* XXX */
		fp->f_flag &= ~FISUSER;	
		soclose((struct socket *)fp->f_socket, nouser);
		if (nouser == 0 && u.u_error)
			return;
		fp->f_socket = 0;
		fp->f_count = 0;
		fp->f_next = ffreelist;
		ffreelist = fp;
		/*
		 * the next line was in the 11 code. Dont quite understand it,
		 * but it cant hurt... (billn)
		 */
		u.u_error = 0;		/* so u.u_ofile always gets 0'd */
		return;
	}
#endif
	ip = fp->f_inode;
	plock(ip);
	dev = (dev_t)ip->i_rdev;
	fmt = ip->i_mode&IFMT;
	fp->f_count = 0;
	fp->f_next = ffreelist;
	ffreelist = fp;
	switch(fmt) {

	case IFCHR:
		cfunc = cdevsw[(short)major(dev)].d_close;
		break;

	case IFBLK:
		cfunc = bdevsw[bmajor(dev)].d_close;
		break;

	case IFIFO:
		closep(ip, flag);

	default:
		iput(ip);
		return;
	}
	for (fp = file; fp < (struct file *)v.ve_file; fp++) {
		register struct inode *tip;

#ifdef  UCB_NET
		if (fp->f_flag & FSOCKET)
			continue;
#endif
		if (fp->f_count) {
			tip = fp->f_inode;
			if (tip->i_rdev == dev &&
			  (tip->i_mode&IFMT) == fmt)
				goto out;
		}
	}
	if (fmt == IFBLK) {
		register struct mount *mp;

		for (mp = mount; mp < (struct mount *)v.ve_mount; mp++)
			if (mp->m_flags == MINUSE && mp->m_dev == dev)
				goto out;
		bflush(dev);
		(*cfunc)(minor(dev), flag);
		binval(dev);
	} else {
		prele(ip);
		(*cfunc)(minor(dev), flag);
	}
out:
	iput(ip);
}

/*
 * openi called to allow handler of special files to initialize and
 * validate before actual IO.
 */
openi(ip, flag)
register struct inode *ip;
{
	dev_t dev;
	register unsigned int maj;

	dev = (dev_t)ip->i_rdev;
	switch(ip->i_mode&IFMT) {

	case IFCHR:
		maj = major(dev);
		if (maj >= cdevcnt)
			goto bad;
		if (u.u_ttyp == NULL)
			u.u_ttyd = dev;
		(*cdevsw[(short)maj].d_open)(minor(dev), flag);
		break;

	case IFBLK:
		maj = bmajor(dev);
		if (maj >= bdevcnt)
			goto bad;
		(*bdevsw[maj].d_open)(minor(dev), flag);
		break;

	case IFIFO:
		openp(ip, flag);
		break;
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the read-only status of the file
 * system is checked. Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select the owner/group/other fields.
 * The super user is granted all permissions.
 */
access(ip, mode)
register struct inode *ip;
{
	register struct user *up;
	register m;

	up = &u;
	m = mode;
	if (m == IWRITE) {
		if (getfs(ip->i_dev)->s_ronly) {
			up->u_error = EROFS;
			return(1);
		}
		if (ip->i_flag&ITEXT)
			xrele(ip);
		if (ip->i_flag & ITEXT) {
			up->u_error = ETXTBSY;
			return(1);
		}
	}
	if (up->u_uid == 0)
		return(0);
	if (up->u_uid != ip->i_uid) {
		m >>= 3;
		if (up->u_gid != ip->i_gid)
			m >>= 3;
	}
	if ((ip->i_mode&m) != 0)
		return(0);

	up->u_error = EACCES;
	return(1);
}

/*
 * Look up a pathname and test if the resultant inode is owned by the
 * current user. If not, try for super-user.
 * If permission is granted, return inode pointer.
 */
struct inode *
owner()
{
	register struct inode *ip;

	ip = namei(uchar, 0);
	if (ip == NULL)
		return(NULL);
	if (u.u_uid == ip->i_uid || suser())
		if (getfs(ip->i_dev)->s_ronly)
			u.u_error = EROFS;
	if (!u.u_error)
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the super user.
 */
suser()
{

	if (u.u_uid == 0) {
		u.u_acflag |= ASU;
		return(1);
	}
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc(i)
register i;
{
	register struct user *up;

	up = &u;
	for(; i<NOFILE; i++)
		if (up->u_ofile[i] == NULL) {
			up->u_rval1 = i;
			up->u_pofile[i] = 0;
			return(i);
		}
	up->u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor and a file structure.
 * Initialize the descriptor to point at the file structure.
 *
 * no file -- if there are no available file structures.
 */
struct file *
falloc(ip, flag)
struct inode *ip;
{
	register struct file *fp;
	register i;

	i = ufalloc(0);
	if (i < 0)
		return(NULL);
	if ((fp=ffreelist) == NULL) {
		printf("no file\n");
		syserr.fileovf++;
		u.u_error = ENFILE;
		return(NULL);
	}
	ffreelist = fp->f_next;
	u.u_ofile[i] = fp;
	fp->f_count++;
	fp->f_inode = ip;
	fp->f_flag = flag;
	fp->f_offset = 0;
	return(fp);
}

struct file *ffreelist;
finit()
{
	register struct file *fp;
	register short i;

	ffreelist = fp = &file[0];
	i = v.v_file - 1 - 1;
	do {
		fp->f_next = fp+1;
		fp++;
	} while (--i != -1);
}
