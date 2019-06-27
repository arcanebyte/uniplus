/* @(#)sys3.c	1.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/ino.h"
#include "sys/buf.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/stat.h"
#include "sys/ttold.h"
#include "sys/var.h"
#ifdef UCB_NET
#include "sys/termio.h"
#include "net/misc.h"
#include "net/socketvar.h"
#endif

/*
 * the fstat system call.
 */
fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
#ifdef UCB_NET
	if (fp->f_flag & FSOCKET)
		u.u_error = sostat((struct socket *)fp->f_socket, uap->sb);
	else
#endif
	stat1(fp->f_inode, uap->sb);
}

/*
 * the stat system call.
 */
stat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0);
	if(ip == NULL)
		return;
	stat1(ip, uap->sb);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
register struct inode *ip;
struct stat *ub;
{
	register struct dinode *dp;
	register struct buf *bp;
	register struct stat *dsp;
	struct stat ds;

	if(ip->i_flag&(IACC|IUPD|ICHG))
		iupdat(ip, &time, &time);
	/*
	 * first copy from inode table
	 */
	dsp = &ds;
	dsp->st_dev = brdev(ip->i_dev);
	dsp->st_ino = ip->i_number;
	dsp->st_mode = ip->i_mode;
	dsp->st_nlink = ip->i_nlink;
	dsp->st_uid = ip->i_uid;
	dsp->st_gid = ip->i_gid;
	dsp->st_rdev = (dev_t)ip->i_rdev;
	dsp->st_size = ip->i_size;
	/*
	 * next the dates in the disk
	 */
	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	dsp->st_atime = dp->di_atime;
	dsp->st_mtime = dp->di_mtime;
	dsp->st_ctime = dp->di_ctime;
	brelse(bp);
	if (copyout((caddr_t)dsp, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 * the dup system call.
 */
dup()
{
	register struct file *fp;
	int i;
	struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if ((i = ufalloc(0)) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

/*
 * the file control system call.
 */
fcntl()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	register i;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	switch(uap->cmd) {
	case 0:
		i = uap->arg;
		if (i < 0 || i > NOFILE) {
			up->u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		up->u_ofile[i] = fp;
		fp->f_count++;
		break;

	case 1:
		up->u_rval1 = up->u_pofile[uap->fdes];
		break;

	case 2:
		up->u_pofile[uap->fdes] = uap->arg;
		break;

	case 3:
		up->u_rval1 = fp->f_flag+FOPEN;
		break;

	case 4:
		fp->f_flag &= (FREAD|FWRITE);
		fp->f_flag |= (uap->arg-FOPEN) & ~(FREAD|FWRITE);
		break;

	default:
		up->u_error = EINVAL;
	}
}

/*
 * character special i/o control
 */
ioctl()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	register dev_t dev;
#ifdef UCB_NET
	register unsigned fmt;
#endif

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
#ifdef UCB_NET
	if (fp->f_flag & FSOCKET) {
		soioctl((struct socket *)fp->f_socket, uap->cmd, (caddr_t)uap->arg);
		return;
	}
#endif
	ip = fp->f_inode;
#ifdef UCB_NET
	fmt = ip->i_mode & IFMT;
	if (fmt != IFCHR) {
		if (uap->cmd==FIONREAD && (fmt == IFREG || fmt == IFDIR
			|| fmt == IFIFO)) {
			off_t nread;

			if ((ip->i_mode&IFMT)==IFIFO)
				nread = ip->i_size;
			else
				nread = ip->i_size - fp->f_offset;
			if (copyout((caddr_t)&nread, (caddr_t)uap->arg,
				 sizeof(off_t)))
				u.u_error = EFAULT;
		} else if (uap->cmd == FIONBIO /*|| uap->cmd == FIOASYNC*/)
			return;
		else
			u.u_error = ENOTTY;
		return;
	}
#else
	if ((ip->i_mode&IFMT) != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
#endif UCB_NET
	dev = (dev_t)ip->i_rdev;
	(*cdevsw[(short)major(dev)].d_ioctl)(minor(dev),uap->cmd,uap->arg,fp->f_flag);
}

/*
 * old stty and gtty
 */
stty()
{
	register struct a {
		int	fdes;
		int	arg;
		int	narg;
	} *uap;

	uap = (struct a *)u.u_ap;
	uap->narg = uap->arg;
	uap->arg = TIOCSETP;
	ioctl();
}

gtty()
{
	register struct a {
		int	fdes;
		int	arg;
		int	narg;
	} *uap;

	uap = (struct a *)u.u_ap;
	uap->narg = uap->arg;
	uap->arg = TIOCGETP;
	ioctl();
}

/*
 * the mount system call.
 */
smount()
{
	register struct user *up;
	register dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	struct mount *smp;
	register struct filsys *fp;
	struct inode *bip = NULL;
	register struct a {
		char	*fspec;
		char	*freg;
		int	ronly;
	} *uap;

	up = &u;
	uap = (struct a *)up->u_ap;
	if(!suser())
		return;
	ip = namei(uchar, 0);
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT) != IFBLK)
		up->u_error = ENOTBLK;
	dev = (dev_t)ip->i_rdev;
	if(bmajor(dev) >= bdevcnt)
		if (!up->u_error)
			up->u_error = ENXIO;
	if (up->u_error)
		goto out;
	bip = ip;
	up->u_dirp = (caddr_t)uap->freg;
	ip = namei(uchar, 0);
	if(ip == NULL) {
		iput(bip);
		return;
	}
	if ((ip->i_mode&IFMT) != IFDIR) {
		up->u_error = ENOTDIR;
		goto out;
	}
	if (ip->i_count != 1)
		goto out;
	if (ip->i_number == ROOTINO)
		goto out;
	smp = NULL;
	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++) {
		if(mp->m_flags != MFREE) {
			if (brdev(dev) == brdev(mp->m_dev))
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
	mp = smp;
	if(mp == NULL)
		goto out;
	mp->m_flags = MINTER;
	mp->m_dev = brdev(dev);
	(*bdevsw[(short)bmajor(dev)].d_open)(minor(dev),
		uap->ronly ?  (FREAD | FKERNEL) : (FREAD | FWRITE | FKERNEL));
	if(up->u_error)
		goto out1;
	mp->m_bufp = geteblk();
	fp = mp->m_bufp->b_un.b_filsys;
	up->u_offset = SUPERBOFF;
	up->u_count = sizeof(struct filsys);
	up->u_base = (caddr_t)fp;
	up->u_segflg = 1;
	readi(bip);
	if (up->u_error) {
		brelse(mp->m_bufp);
		goto out1;
	}
	mp->m_inodp = ip;
	mp->m_flags = MINUSE;
	if (fp->s_magic != FsMAGIC)
		fp->s_type = Fs1b;	/* assume old file system */
	if (fp->s_type == Fs2b)
		mp->m_dev |= Fs2BLK;
#if FsTYPE == 4
	if (fp->s_type == Fs4b)
		mp->m_dev |= Fs4BLK;
#endif
	if (brdev(pipedev) == brdev(mp->m_dev))
		pipedev = mp->m_dev;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;
	fp->s_ronly = uap->ronly & 1;
	if (mp->m_mount = iget(mp->m_dev, ROOTINO))
		prele(mp->m_mount);
	else {
		brelse(mp->m_bufp);
		goto out1;
	}
	ip->i_flag |= IMOUNT;
	iput(bip);
	prele(ip);
	return;

out1:
	mp->m_flags = MFREE;
out:
	if (bip != NULL)
		iput(bip);
	if (up->u_error == 0)
		up->u_error = EBUSY;
	iput(ip);
}

/*
 * the umount system call.
 */
sumount()
{
	register dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	register struct a {
		char	*fspec;
	};

	if(!suser())
		return;
	dev = getmdev();
	if(u.u_error)
		return;
	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
		if(mp->m_flags == MINUSE && brdev(dev) == brdev(mp->m_dev))
			goto found;
	u.u_error = EINVAL;
	return;

found:
	dev = mp->m_dev;
	(void) xumount(dev);	/* remove unused sticky files from text table */
	update();
	if (mp->m_mount) {
		plock(mp->m_mount);
		iput(mp->m_mount);
		mp->m_mount = NULL;
	}
	for(ip = &inode[0]; ip < (struct inode *)v.ve_inode; ip++)
		if(ip->i_number != 0 && dev == ip->i_dev) {
			u.u_error = EBUSY;
			return;
		}
	(*bdevsw[(short)bmajor(dev)].d_close)(minor(dev), FKERNEL);
	binval(dev);
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	plock(ip);
	iput(ip);
	brelse(mp->m_bufp);
	mp->m_bufp = NULL;
	mp->m_flags = MFREE;
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
dev_t
getmdev()
{
	dev_t dev;
	register struct inode *ip;

	ip = namei(uchar, 0);
	if(ip == NULL)
		return(NODEV);
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	dev = (dev_t)ip->i_rdev;
	if(bmajor(dev) >= bdevcnt)
		u.u_error = ENXIO;
	iput(ip);
	return(dev);
}
