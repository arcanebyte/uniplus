/* @(#)subr.c	1.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/inode.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/mount.h"
#include "sys/var.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in u.u_rablock
 * for use in read-ahead.
 */
daddr_t
bmap(ip, readflg)
register struct inode *ip;
{
	register struct user *up;
	register i;
	register dev;
	daddr_t bn;
	daddr_t nb, *bap;
	int raflag;

	up = &u;
	up->u_rablock = 0;
	raflag = 0;
	{
		register sz, rem, type;

		type = ip->i_mode&IFMT;
		if (type == IFBLK) {
			dev = (dev_t)ip->i_rdev;
			for (i=0; i<v.v_mount; i++)
				if ((mount[i].m_flags==MINUSE) &&
				    (brdev(mount[i].m_dev)==brdev(dev))) {
					dev = mount[i].m_dev;
					break;
				}
		} else
			dev = ip->i_dev;
		up->u_pbdev = dev;
		bn = FsBNO(dev, up->u_offset);
		if (bn < 0) {
			up->u_error = EFBIG;
			return((daddr_t)-1);
		}
		if ((ip->i_lastr + 1) == bn)
			raflag++;
		up->u_pboff = FsBOFF(dev, up->u_offset);
		sz = FsBSIZE(dev) - up->u_pboff;
		if (up->u_count < sz) {
			sz = up->u_count;
			raflag = 0;
		} else
			ip->i_lastr = bn;

		up->u_pbsize = sz;
		if (type == IFBLK) {
			if (raflag)
				up->u_rablock = bn + 1;
			return(bn);
		}
		if (readflg) {
			if (type == IFIFO) {
				raflag = 0;
				rem = ip->i_size;
			} else
				rem = ip->i_size - up->u_offset;
			if (rem < 0)
				rem = 0;
			if (rem < sz)
				sz = rem;
			if ((up->u_pbsize = sz) == 0)
				return((daddr_t)-1);
		} else {
			if (bn >= FsPTOL(dev, up->u_limit)) {
				up->u_error = EFBIG;
				return((daddr_t)-1);
			}
		}
	}
	{
	register struct buf *bp;
	register j, sh;


	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if (bn < NADDR-3) {
		i = bn;
		nb = ip->i_addr[i];
		if (nb == 0) {
			if (readflg || (bp = alloc(dev))==NULL)
				return((daddr_t)-1);
			nb = FsPTOL(dev, bp->b_blkno);
			bdwrite(bp);
			ip->i_addr[i] = nb;
			ip->i_flag |= IUPD|ICHG;
		}
		if ((i < NADDR-4) && raflag)
			up->u_rablock = ip->i_addr[i+1];
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += FsNSHIFT(dev);
		nb <<= FsNSHIFT(dev);
		if (bn < nb)
			break;
		bn -= nb;
	}
	if (j == 0) {
		up->u_error = EFBIG;
		return((daddr_t)-1);
	}

	/*
	 * fetch the address from the inode
	 */
	nb = ip->i_addr[NADDR-j];
	if (nb == 0) {
		if (readflg || (bp = alloc(dev))==NULL)
			return((daddr_t)-1);
		nb = FsPTOL(dev, bp->b_blkno);
		bwrite(bp);
		ip->i_addr[NADDR-j] = nb;
		ip->i_flag |= IUPD|ICHG;
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		bp = bread(dev, nb);
		if (up->u_error) {
			brelse(bp);
			return((daddr_t)-1);
		}
		bap = bp->b_un.b_daddr;
		sh -= FsNSHIFT(dev);
		i = (bn>>sh) & FsNMASK(dev);
		nb = bap[i];
		if (nb == 0) {
			register struct buf *nbp;

			if (readflg || (nbp = alloc(dev))==NULL) {
				brelse(bp);
				return((daddr_t)-1);
			}
			nb = FsPTOL(dev, nbp->b_blkno);
			if (j < 3)
				bwrite(nbp);
			else
				bdwrite(nbp);
			bap[i] = nb;
			bdwrite(bp);
		} else
			brelse(bp);
	}

	/*
	 * calculate read-ahead.
	 */
	if ((i < FsNINDIR(dev)-1) && raflag)
		up->u_rablock = bap[i+1];
	return(nb);
	}
}

/*
 * Pass back  c  to the user at his location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * on the last character of the user's read.
 * u_base is in the user data space.
 */
passc(c)
register c;
{
	register struct user *up;

	up = &u;
	if (subyte(up->u_base, c) < 0) {
		up->u_error = EFAULT;
		return(-1);
	}
	up->u_count--;
	up->u_offset++;
	up->u_base++;
	return(up->u_count == 0? -1: 0);
}

/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.
 * u_base is in the user data space.
 */
cpass()
{
	register struct user *up;
	register c;

	up = &u;
	if (up->u_count == 0)
		return(-1);
	if ((c = fubyte(up->u_base)) < 0) {
		up->u_error = EFAULT;
		return(-1);
	}
	up->u_count--;
	up->u_offset++;
	up->u_base++;
	return(c);
}

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}

/*
 * Max function
 */
max(a, b)
unsigned a, b;
{
	if (a >= b)
		return(a);
	return(b);
}

/*
 * Min function
 */
min(a, b)
unsigned a, b;
{
	if (a <= b)
		return(a);
	return(b);
}
