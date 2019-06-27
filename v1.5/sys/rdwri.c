/* @(#)rdwri.c	1.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/inode.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/systm.h"
#include "sys/tty.h"

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user/user I
 */
readi(ip)
register struct inode *ip;
{
	register struct user *up;
	register struct buf *bp;
	register dev_t dev;
	register daddr_t bn;
	register unsigned on, n;
	register type;
	register struct tty *tp;
	struct cdevsw *cdevp;

	up = &u;
	if (up->u_count == 0)
		return;
	if (up->u_offset < 0) {
		up->u_error = EINVAL;
		return;
	}
	type = ip->i_mode&IFMT;
	switch(type) {
	case IFCHR:
		dev = (dev_t)ip->i_rdev;
		ip->i_flag |= IACC;
		cdevp = &cdevsw[(short)major(dev)];
		if (tp = cdevp->d_ttys) {
			tp += (short)(minor(dev)&077);
			(*linesw[(short)tp->t_line].l_read)(tp);
		} else
			(*cdevp->d_read)(minor(dev));
		break;

	case IFIFO:
		while (ip->i_size == 0) {
			if (ip->i_fwcnt == 0)
				return;
			if (up->u_fmode&FNDELAY)
				return;
			ip->i_fflag |= IFIR;
			prele(ip);
			(void) sleep((caddr_t)&ip->i_frcnt, PPIPE);
			plock(ip);
		}
		up->u_offset = ip->i_frptr;
	case IFBLK:
	case IFREG:
	case IFDIR:
	do {
		bn = bmap(ip, B_READ);
		if (up->u_error)
			break;
		on = up->u_pboff;
		if ((n = up->u_pbsize) == 0)
			break;
		dev = up->u_pbdev;
		if ((long)bn<0) {
			if (type != IFREG)
				break;
			bp = geteblk();
			clrbuf(bp);
		} else if (up->u_rablock)
			bp = breada(dev, bn, up->u_rablock);
		else
			bp = bread(dev, bn);
		if (bp->b_resid) {
			n = 0;
		}
		if (n!=0)
			iomove(bp->b_un.b_addr+on, (int)n, B_READ);
		if (type == IFIFO) {
			ip->i_size -= n;
			if (up->u_offset >= PIPSIZ)
				up->u_offset = 0;
			if ((on+n) == FsBSIZE(dev) && ip->i_size < (PIPSIZ-FsBSIZE(dev)))
				bp->b_flags &= ~B_DELWRI;
		}
		brelse(bp);
		ip->i_flag |= IACC;
	} while (up->u_error==0 && up->u_count!=0 && n!=0);
		if (type == IFIFO) {
			if (ip->i_size)
				ip->i_frptr = up->u_offset;
			else {
				ip->i_frptr = 0;
				ip->i_fwptr = 0;
			}
			if (ip->i_fflag&IFIW) {
				ip->i_fflag &= ~IFIW;
				curpri = PPIPE;
				wakeup((caddr_t)&ip->i_fwcnt);
			}
		}
		break;

	default:
		up->u_error = ENODEV;
	}
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user/user I
 */
writei(ip)
register struct inode *ip;
{
	register struct user *up;
	register struct buf *bp;
	register dev_t dev;
	register daddr_t bn;
	register unsigned n, on;
	register type;
	unsigned int usave;
	register struct tty *tp;
	struct cdevsw *cdevp;

	up = &u;
	if (up->u_offset < 0) {
		up->u_error = EINVAL;
		return;
	}
	type = ip->i_mode&IFMT;
	switch (type) {
	case IFCHR:
		dev = (dev_t)ip->i_rdev;
		ip->i_flag |= IUPD|ICHG;
		cdevp = &cdevsw[(short)major(dev)];
		if (tp = cdevp->d_ttys) {
			tp += (short)(minor(dev)&077);
			(*linesw[(short)tp->t_line].l_write)(tp);
		} else
			(*cdevp->d_write)(minor(dev));
		break;

	case IFIFO:
	floop:
		usave = 0;
		while ((up->u_count+ip->i_size) > PIPSIZ) {
			if (ip->i_frcnt == 0)
				break;
			if ((up->u_count > PIPSIZ) && (ip->i_size < PIPSIZ)) {
				usave = up->u_count;
				up->u_count = PIPSIZ - ip->i_size;
				usave -= up->u_count;
				break;
			}
			if (up->u_fmode&FNDELAY)
				return;
			ip->i_fflag |= IFIW;
			prele(ip);
			(void) sleep((caddr_t)&ip->i_fwcnt, PPIPE);
			plock(ip);
		}
		if (ip->i_frcnt == 0) {
			up->u_error = EPIPE;
			psignal(up->u_procp, SIGPIPE);
			break;
		}
		up->u_offset = ip->i_fwptr;
	case IFBLK:
	case IFREG:
	case IFDIR:
	while (up->u_error==0 && up->u_count!=0) {
		bn = bmap(ip, B_WRITE);
		if (up->u_error)
			break;
		on = up->u_pboff;
		n = up->u_pbsize;
		dev = up->u_pbdev;
		if (n == FsBSIZE(dev)) 
			bp = getblk(dev, bn);
		else if (type==IFIFO && on==0 && ip->i_size < (PIPSIZ-FsBSIZE(dev)))
			bp = getblk(dev, bn);
		else
			bp = bread(dev, bn);

		iomove(bp->b_un.b_addr+on, (int)n, B_WRITE);
		if (up->u_error)
			brelse(bp);
		else if (up->u_fmode&FSYNC)
			bwrite(bp);
		else if (type == IFBLK) {
			/* IFBLK not delayed for tapes */
			bp->b_flags |= B_AGE;
			bawrite(bp);
		} else
			bdwrite(bp);
		if (type == IFREG || type == IFDIR) {
			if (up->u_offset > ip->i_size)
				ip->i_size = up->u_offset;
		} else if (type == IFIFO) {
			ip->i_size += n;
			if (up->u_offset == PIPSIZ)
				up->u_offset = 0;
		}
		ip->i_flag |= IUPD|ICHG;
	}
		if (type == IFIFO) {
			ip->i_fwptr = up->u_offset;
			if (ip->i_fflag&IFIR) {
				ip->i_fflag &= ~IFIR;
				curpri = PPIPE;
				wakeup((caddr_t)&ip->i_frcnt);
			}
			if (up->u_error==0 && usave!=0) {
				up->u_count = usave;
				goto floop;
			}
		}
		break;

	default:
		up->u_error = ENODEV;
	}
}

/*
 * Move n bytes at byte location
 * &bp->b_un.b_addr[o] to/from (flag) the
 * user/kernel (u.segflg) area starting at u.base.
 * Update all the arguments by the number
 * of bytes moved.
 */
iomove(cp, n, flag)
register caddr_t cp;
register n;
{
	register struct user *up;
	register t;

	if (n==0)
		return;
	up = &u;
	if (up->u_segflg != 1)  {
		if (flag==B_WRITE)
			t = copyin(up->u_base, (caddr_t)cp, n);
		else
			t = copyout((caddr_t)cp, up->u_base, n);
		if (t) {
			up->u_error = EFAULT;
			return;
		}
	}
	else
		if (flag == B_WRITE)
			bcopy(up->u_base,(caddr_t)cp,n);
		else
			bcopy((caddr_t)cp,up->u_base,n);
	up->u_base += n;
	up->u_offset += n;
	up->u_count -= n;
	return;
}
