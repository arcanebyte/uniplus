/*	@(#)mount.h 1.1 86/02/03 SMI; from UCB 4.4 82/07/19	*/
/*	@(#)mount.h	2.1 86/04/14 NFSSRC */

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	struct vfs	*m_vfsp;	/* vfs structure for this filesystem */
	dev_t		m_dev;		/* device mounted */
	struct buf	*m_bufp;	/* pointer to superblock */
	struct inode	*m_qinod;	/* QUOTA: pointer to quota file */
	u_short		m_qflags;	/* QUOTA: filesystem flags */
	u_long		m_btimelimit;	/* QUOTA: block time limit */
	u_long		m_ftimelimit;	/* QUOTA: file time limit */
};

#ifdef KERNEL
/*
 * Convert vfs ptr to mount ptr. ONLY WORKS IF m_vfs IS FIRST.
 */
#define VFSTOM(VFSP)  ((struct mount *)(VFSP->vfs_data))

/*
 * mount table
 */
#ifndef AUTOCONFIG
extern struct mount	mounttab[];
#else
extern struct mount	*mounttab;
#endif AUTOCONFIG

/*
 * Operations
 */
struct mount *getmp();
#endif
