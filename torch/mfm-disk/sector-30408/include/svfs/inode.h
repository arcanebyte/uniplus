/*	@(#)inode.h 1.1 85/05/30 SMI; from UCB 4.24 83/07/01	*/
/* @(#)inode.h	2.2 86/05/15 NFSSRC */

/*
 * The I node is the focus of all file activity in UNIX.
 * There is a unique inode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An inode is 'named' by its dev/inumber pair. (iget/iget.c)
 * Data in icommon is read in from permanent inode on volume.
 */

#define	NADDR	13		/* # of addresses in inode */
#define NIADDR	3		/* indirect addresses in inode */
#define NDADDR	(NADDR - NIADDR)/* direct addresses in inode */
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))
#define	NFADDR	10
#define	PIPSIZ	(NFADDR*DEV_BSIZE)

struct inode {
	struct	inode *i_chain[2];	/* must be first */
	struct	vnode i_vnode;	/* vnode associated with this inode */
	struct	vnode *i_devvp;	/* vnode for block I/O */
	u_short	i_flag;
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	int	i_diroff;	/* offset in dir, where we found last entry */
	struct	filsys *i_fs;	/* file sys associated with this inode */
	union {
		daddr_t	if_lastr;	/* last read (read-ahead) */
		struct	socket *is_socket;
	} i_un;
	struct	{
		struct inode  *if_freef;	/* free list forward */
		struct inode **if_freeb;	/* free list back */
	} i_fr;
	struct 	icommon {
		ushort	ic_mode;
		short	ic_nlink;	/* directory entries */
		ushort	ic_uid;		/* owner */
		ushort	ic_gid;		/* group of owner */
		off_t	ic_size;	/* size of file */
		union {
			daddr_t i_a[NADDR];	/* if normal file/directory */
			short	i_f[NSADDR];	/* if fifo's */
#define	ic_addr		i_p.i_a
		} i_p;
		long	ic_gen;		/* generation number */
		time_t	ic_atime;   	/* time last accessed */
		time_t	ic_mtime;   	/* time last modified */
		time_t	ic_ctime;   	/* time created */
	} i_ic;
	struct locklist *i_locklist;		/* locked region list */
	struct proc	*i_select[2];		/* for select on a FIFO */
	char	*i_lockedfile;
	int	i_lockedline;
};

struct dinode
{
	ushort	di_mode;		/* mode and type of file */
	short	di_nlink;    		/* number of links to file */
	ushort	di_uid;      		/* owner's user id */
	ushort	di_gid;      		/* owner's group id */
	off_t	di_size;     		/* number of bytes in file */
	char  	di_addr[40];		/* disk block addresses */
#define	di_gen		di_addr[39]
	time_t	di_atime;   		/* time last accessed */
	time_t	di_mtime;   		/* time last modified */
	time_t	di_ctime;   		/* time created */
};
/*
 * the 40 address bytes:
 *	39 used; 13 addresses
 *	of 3 bytes each.
 */

#define	i_mode		i_ic.ic_mode
#define	i_nlink		i_ic.ic_nlink
#define	i_uid		i_ic.ic_uid
#define	i_gid		i_ic.ic_gid
#define	i_size		i_ic.ic_size
#define	i_addr		i_ic.ic_addr
#define i_gen           i_ic.ic_gen
#define	i_atime		i_ic.ic_atime
#define	i_mtime		i_ic.ic_mtime
#define	i_ctime		i_ic.ic_ctime
#define i_rdev		i_ic.i_p.i_a[0]
#define	i_faddr			i_ic.i_p.i_a
#define	i_frptr			i_ic.i_p.i_f[NSADDR-5]
#define	i_fwptr			i_ic.i_p.i_f[NSADDR-4]
#define	i_frcnt			i_ic.i_p.i_f[NSADDR-3]
#define	i_fwcnt			i_ic.i_p.i_f[NSADDR-2]
#define	i_fflag			i_ic.i_p.i_f[NSADDR-1]

#define	i_lastr		i_un.if_lastr
#define	i_socket	i_un.is_socket
#define	i_forw		i_chain[0]
#define	i_back		i_chain[1]
#define	i_freef		i_fr.if_freef
#define	i_freeb		i_fr.if_freeb

#ifdef vax
#define	di_size		di_ic.ic_size.val[0]
#endif
#ifdef mc68000
#define	di_size		di_ic.ic_size.val[1]
#endif

#ifdef KERNEL
#ifndef AUTOCONFIG
extern struct inode inode[];		/* The inode table itself */
#else
extern struct inode *inode;		/* The inode table itself */
#endif AUTOCONFIG

extern struct vnodeops svfs_vnodeops;	/* vnode operations for ufs */

extern struct inode	*ialloc();
extern struct inode	*iget();
extern ino_t		dirpref();
#endif

/* flags */
#define	ILOCKED		0x1		/* inode is locked */
#define	IUPD		0x2		/* file has been modified */
#define	IACC		0x4		/* inode access time to be updated */
#ifdef notdef
#define	IMOUNT		0x8		/* inode is mounted on */
#endif
#define	IWANT		0x10		/* some process waiting on lock */
#define	ITEXT		0x20		/* inode is pure text prototype */
#define	ICHG		0x40		/* inode has been changed */
#ifdef notdef
#define	ISHLOCK		0x80		/* file has shared lock */
#define	IEXLOCK		0x100		/* file has exclusive lock */
#endif
#define	ILWAIT		0x200		/* someone waiting on file lock */
#define IREF		0x400		/* inode is being referenced */

/* modes */
#define	IFMT		0170000		/* type of file */
#define	IFIFO		0010000		/* fifo special */
#define	IFCHR		0020000		/* character special */
#define	IFMPC		0030000		/* multiplexed char special */
#define	IFDIR		0040000		/* directory */
#define	IFBLK		0060000		/* block special */
#define	IFMPB		0070000		/* multiplexed block special */
#define	IFREG		0100000		/* regular */
#define	IFLNK		0120000		/* symbolic link */
#define	IFSOCK		0140000		/* socket */

#define	ISUID		04000		/* set user id on execution */
#define	ISGID		02000		/* set group id on execution */
#define	ISVTX		01000		/* save swapped text even after use */
#define	IREAD		0400		/* read, write, execute permissions */
#define	IWRITE		0200
#define	IEXEC		0100

#define	IFIR		01
#define	IFIW		02
#define	IFRDCOLL	04		/* Select read collision */
#define	IFWRCOLL	010		/* Select write collision */

#define	IFRDSEL		0		/* Index for i_select[] */
#define	IFWRSEL		1		/* Index for i_select[] */

#ifdef KERNEL
/*
 * Convert between inode pointers and vnode pointers
 */
#define VTOI(VP)	((struct inode *)(VP)->v_data)
#define ITOV(IP)	((struct vnode *)&(IP)->i_vnode)

/*
 * Convert between vnode types and inode formats
 */
extern enum vtype	iftovt_tab[];
extern int		vttoif_tab[];
#define IFTOVT(M)	(iftovt_tab[((M) & IFMT) >> 12])
#define VTTOIF(T)	(vttoif_tab[(int)(T)])

#define MAKEIMODE(T, M)	(VTTOIF(T) | (M))

/*
 * Lock and unlock inodes.
 */
#ifdef ITRACE

#define NITRACE	2048

extern int (*caller())();
extern int itrace();
extern int ilpanic();

#define	ILOCK(ip, file, line) { \
	while ((ip)->i_flag & ILOCKED) { \
		(ip)->i_flag |= IWANT; \
		sleep((caddr_t)(ip), PINOD); \
	} \
	itrace((ip), caller(), 1); \
	timeout(ilpanic, (ip), 30 * v.v_hz); \
	(ip)->i_flag |= ILOCKED; \
	ip->i_lockedfile = file; \
	ip->i_lockedline = line; \
}

#define	IUNLOCK(ip) { \
	(ip)->i_flag &= ~ILOCKED; \
	untimeout(ilpanic, (ip)); \
	itrace((ip), caller(), 0); \
	if ((ip)->i_flag&IWANT) { \
		(ip)->i_flag &= ~IWANT; \
		wakeup((caddr_t)(ip)); \
	} \
}
#else
#define	ILOCK(ip, file, line) { \
	while ((ip)->i_flag & ILOCKED) { \
		(ip)->i_flag |= IWANT; \
		sleep((caddr_t)(ip), PINOD); \
	} \
	(ip)->i_flag |= ILOCKED; \
	ip->i_lockedfile = file; \
	ip->i_lockedline = line; \
}

#define	IUNLOCK(ip) { \
	(ip)->i_flag &= ~ILOCKED; \
	if ((ip)->i_flag&IWANT) { \
		(ip)->i_flag &= ~IWANT; \
		wakeup((caddr_t)(ip)); \
	} \
}
#endif

#define ESAME (-1)		/* trying to rename linked files (special) */

/*
 * Check that file is owned by current user or user is su.
 */
#define OWNER(CR, IP)	(((CR)->cr_uid == (IP)->i_uid)? 0: (suser()? 0: u.u_error))

/*
 * enums
 */
enum de_op	{ DE_CREATE, DE_LINK, DE_RENAME };	/* direnter ops */

#endif
