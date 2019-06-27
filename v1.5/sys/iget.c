/* @(#)iget.c	1.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/ino.h"
#include "sys/filsys.h"
#include "sys/buf.h"
#include "sys/var.h"

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure), honor the locking protocol.
 * If it is not in core, read it in from the specified device.
 * If the inode is mounted on, perform the indicated indirection.
 * In all cases, a pointer to a locked inode structure is returned.
 *
 * printf warning: no inodes -- if the inode structure is full
 * panic: no imt -- if the mounted filesystem is not in the mount table.
 *	"cannot happen"
 */

#define	NHINO	128	/* must be power of 2 */
#define	ihash(X)	(&hinode[(int)(X) & (NHINO-1)])
struct	hinode {
	struct inode *i_forw;
} hinode[NHINO];
struct inode *ifreelist;

struct inode *
iget(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct inode *ip;
	register struct hinode *hip;
	register struct mount *mp;
	struct inode *iread();

	sysinfo.iget++;
loop:
	hip = ihash(ino);
	for (ip = hip->i_forw; ip; ip = ip->i_forw)
		if (ino == ip->i_number && dev == ip->i_dev)
			goto found;
	if ((ip = ifreelist) == NULL) {
		printf("Inode table overflow\n");
		syserr.inodeovf++;
		u.u_error = ENFILE;
		return(NULL);
	}
	ifreelist = ip->i_forw;
	if (ip->i_forw = hip->i_forw)
		ip->i_forw->i_back = ip;
	ip->i_back = (struct inode *)hip;
	hip->i_forw = ip;
	ip->i_dev = dev;
	ip->i_number = ino;
	ip->i_flag = ILOCK;
	ip->i_count++;
	ip->i_lastr = 0;
	return(iread(ip));
found:
	if((ip->i_flag&ILOCK) != 0) {
		ip->i_flag |= IWANT;
		(void) sleep((caddr_t)ip, PINOD);
		goto loop;
	}
	if((ip->i_flag&IMOUNT) != 0) {
		for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
		if(mp->m_inodp == ip) {
			dev = mp->m_dev;
			ino = ROOTINO;
			if (ip = mp->m_mount)
				goto found;
			else
				goto loop;
		}
		panic("no imt");
	}
	ip->i_count++;
	ip->i_flag |= ILOCK;
	return(ip);
}

inoinit()
{
	register struct inode *ip;
	register short i;

	ifreelist = ip = &inode[0];
	i = v.v_inode - 1 - 1;
	do {
		ip->i_forw = ip+1;
		ip++;
	} while (--i != -1);
#ifdef notdef
	register i = v.v_inode;

	while (--i)
		inode[i-1].i_forw = &inode[i];
	ifreelist = &inode[0];
#endif
}

struct inode *
iread(ip)
register struct inode *ip;
{
	register char *p1, *p2;
	register struct dinode *dp;
	struct buf *bp;
	register short i;

	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	if (u.u_error) {
		brelse(bp);
		iput(ip);
		return(NULL);
	}
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	ip->i_mode = dp->di_mode;
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	p1 = (char *)ip->i_addr;
	p2 = (char *)dp->di_addr;
	i = NADDR - 1;
	do {
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
	} while (--i != -1);
	brelse(bp);
	return(ip);
}

/*
 * Decrement reference count of an inode structure.
 * On the last reference, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
iput(ip)
register struct inode *ip;
{

	if(ip->i_count == 1) {
		ip->i_flag |= ILOCK;
		if(ip->i_nlink <= 0) {
			itrunc(ip);
			ip->i_mode = 0;
			ip->i_flag |= IUPD|ICHG;
			ifree(ip->i_dev, ip->i_number);
		}
		if(ip->i_flag&(IACC|IUPD|ICHG))
			iupdat(ip, &time, &time);
		prele(ip);
		if (ip->i_back->i_forw = ip->i_forw)
			ip->i_forw->i_back = ip->i_back;
		ip->i_forw = ifreelist;
		ifreelist = ip;
		ip->i_flag = 0;
		ip->i_number = 0;
		ip->i_count = 0;
		return;
	}
	ip->i_count--;
	prele(ip);
}

/*
 * Update the inode with the current time.
 */
iupdat(ip, ta, tm)
register struct inode *ip;
time_t *ta, *tm;
{
	register struct buf *bp;
	struct dinode *dp;
	register char *p1;
	char *p2;
	register short i;

	if(getfs(ip->i_dev)->s_ronly) {
		if(ip->i_flag&(IUPD|ICHG))
			u.u_error = EROFS;
		ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
		return;
	}
	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return;
	}
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	p1 = (char *)dp->di_addr;
	p2 = (char *)ip->i_addr;
	if ((ip->i_mode&IFMT)==IFIFO) {
		i = NFADDR - 1;
		do {
			if (*p2++ != 0)
				printf("iaddress > 2^24\n");
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
		} while (--i != -1);
		i = NADDR - NFADDR - 1;
		if (i >= 0) {
			do {
				*p1++ = 0;
				*p1++ = 0;
				*p1++ = 0;
			} while (--i != -1);
		}
	} else {
		i = NADDR - 1;
		do {
			if(*p2++ != 0)
				printf("iaddress > 2^24\n");
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
		} while (--i != -1);
	}
	if(ip->i_flag&IACC)
		dp->di_atime = *ta;
	if(ip->i_flag&IUPD)
		dp->di_mtime = *tm;
	if(ip->i_flag&ICHG)
		dp->di_ctime = time;
	if (ip->i_flag&ISYN)
		bwrite(bp);
	else
		bdwrite(bp);
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
}

/*
 * Free all the disk blocks associated with the specified inode structure.
 * The blocks of the file are removed in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer than FIFO.
 */
itrunc(ip)
register struct inode *ip;
{
	register i;
	dev_t dev;
	daddr_t bn;

	i = ip->i_mode & IFMT;
	if (i!=IFREG && i!=IFDIR)
		return;
	dev = ip->i_dev;
	for(i=NADDR-1; i>=0; i--) {
		bn = ip->i_addr[i];
		if(bn == (daddr_t)0)
			continue;
		ip->i_addr[i] = (daddr_t)0;
		switch(i) {

		default:
			free(dev, bn);
			break;

		case NADDR-3:
			tloop(dev, bn, 0, 0);
			break;

		case NADDR-2:
			tloop(dev, bn, 1, 0);
			break;

		case NADDR-1:
			tloop(dev, bn, 1, 1);
		}
	}
	ip->i_size = 0;
	ip->i_flag |= IUPD|ICHG;
}

tloop(dev, bn, f1, f2)
dev_t dev;
daddr_t bn;
{
	register i;
	register struct buf *bp;
	register daddr_t *bap;
	daddr_t nb;

	bp = NULL;
	for(i=FsNINDIR(dev)-1; i>=0; i--) {
		if(bp == NULL) {
			bp = bread(dev, bn);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return;
			}
			bap = bp->b_un.b_daddr;
		}
		nb = bap[i];
		if(nb == (daddr_t)0)
			continue;
		if(f1) {
			brelse(bp);
			bp = NULL;
			tloop(dev, nb, f2, 0);
		} else
			free(dev, nb);
	}
	if(bp != NULL)
		brelse(bp);
	free(dev, bn);
}

/*
 * Make a new file.
 */
struct inode *
maknode(mode)
register mode;
{
	register struct inode *ip;

	if ((mode&IFMT) == 0)
		mode |= IFREG;
	mode &= ~u.u_cmask;
	ip = ialloc(u.u_pdir->i_dev, mode, 1);
	if (ip == NULL) {
		iput(u.u_pdir);
		return(NULL);
	}
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with parameters left as side effects
 * to a call to namei.
 */
wdir(ip)
struct inode *ip;
{
	register struct user *up;

	up = &u;
	up->u_dent.d_ino = ip->i_number;
	up->u_count = sizeof(struct direct);
	up->u_segflg = 1;
	up->u_base = (caddr_t)&up->u_dent;
	up->u_fmode = FWRITE;
	writei(up->u_pdir);
	iput(up->u_pdir);
}
