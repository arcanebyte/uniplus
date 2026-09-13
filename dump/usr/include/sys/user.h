/* @(#)user.h	1.6 */
/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*click bytes long.
 * It contains the system stack per user; is cross referenced
 * with the proc structure for the same process.
 */
 
struct	user
{
	label_t	u_rsav;			/* save info when exchanging stacks */
	label_t	u_qsav;			/* label variable for quits and interrupts */
	label_t	u_ssav;			/* label variable for swapping */
	char	u_segflg;		/* IO flag: 0:user D; 1:system; 2:user I */
	char	u_error;		/* return error code */
	ushort	u_uid;			/* effective user id */
	ushort	u_gid;			/* effective group id */
	ushort	u_ruid;			/* real user id */
	ushort	u_rgid;			/* real group id */
	struct proc *u_procp;		/* pointer to proc structure */
	int	*u_ap;			/* pointer to arglist */
	union {				/* syscall return values */
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
	} u_r;
	caddr_t	u_base;			/* base address for IO */
	unsigned u_count;		/* bytes remaining for IO */
	off_t	u_offset;		/* offset in file for IO */
	short	u_fmode;		/* file mode for IO */
	ushort	u_pbsize;		/* bytes in block for IO */
	ushort	u_pboff;		/* offset in block for IO */
	dev_t	u_pbdev;		/* real device for IO */
	daddr_t	u_rablock;		/* read ahead block addr */
	short	u_errcnt;		/* syscall error count */
	struct inode *u_cdir;		/* pointer to inode of current directory */
	struct inode *u_rdir;		/* root directory of current process */
	caddr_t	u_dirp;			/* pathname pointer */
	struct direct u_dent;		/* current directory entry */
	struct inode *u_pdir;		/* inode of parent directory of dirp */
	struct file *u_ofile[NOFILE];	/* pointers to file structures of open files */
	char	u_pofile[NOFILE];	/* per-process flags of open files */
	int	u_arg[10];		/* arguments to current system call */
	unsigned u_tsize;		/* text size (clicks) */
	unsigned u_dsize;		/* data size (clicks) */
	unsigned u_ssize;		/* stack size (clicks) */
	int	u_signal[NSIG];		/* disposition of signals */
	time_t	u_utime;		/* this process user time */
	time_t	u_stime;		/* this process system time */
	time_t	u_cutime;		/* sum of childs' utimes */
	time_t	u_cstime;		/* sum of childs' stimes */
	int	*u_ar0;			/* address of users saved R0 */
	struct {			/* profile arguments */
		short	*pr_base;	/* buffer base */
		unsigned pr_size;	/* buffer size */
		unsigned pr_off;	/* pc offset */
		unsigned pr_scale;	/* pc scaling */
	} u_prof;
	char	u_intflg;		/* catch intr from sys */
	char	u_sep;			/* flag for I and D separation */
	short	*u_ttyp;		/* pointer to pgrp in "tty" struct */
	dev_t	u_ttyd;			/* controlling tty dev */
	struct exdata {			/* header of executable file */
		long	ux_mag;		/* magic number */
		long	ux_tsize;	/* text size */
		long	ux_dsize;	/* data size */
		long	ux_bsize;	/* bss size */
		long	ux_ssize;	/* symbol table size */
		long	ux_rtsize;
		long	ux_rdsize;
		long	ux_entloc;	/* entry location */
	} u_exdata;
#ifdef notdef
	struct {			/* header of executable file */
		short	ux_mag;		/* magic number */
		short	ux_stamp;	/* stamp */
		unsigned ux_tsize;	/* text size */
		unsigned ux_dsize;	/* data size */
		unsigned ux_bsize;	/* bss size */
		unsigned ux_ssize;	/* symbol table size */
		unsigned ux_entloc;	/* entry location */
		unsigned ux_unused;
		unsigned ux_relflg;
	} u_exdata;
#endif
#define	ux_tstart	ux_unused
	char	u_comm[DIRSIZ];
	time_t	u_start;
	time_t	u_ticks;
	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	char	u_acflag;
	short	u_cmask;		/* mask for file creation */
	short	u_fcode;		/* function code on bus errors */
	long	u_aaddr;		/* access address on bus errors */
	short	u_ireg;			/* instruction register on bus errors*/
	unsigned int u_ptsize;		/* text size (clicks) for sureg */
	unsigned int u_pdsize;		/* data size (clicks) for sureg */
	unsigned int u_pssize;		/* stack size (clicks)for sureg */
	int	u_xrw;			/* rw flag for sureg */
	daddr_t	u_limit;		/* maximum write address */
	short	u_lock;			/* process/text locking flags */
	struct phys {
		int	u_phladdr;	/* phys logical address */
		int	u_phsize;	/* phys size */
		int	u_phpaddr;	/* phys physical address */
		int	u_phfill;	/* fill */
	} u_phys[NPHYS];
	int	u_usrtop;		/* top of user area */
#ifdef FLOAT
	char	u_fpinuse;		/* fp unit in use */
	char	u_fpsaved;		/* fp unit state saved */
	struct {
		unsigned short u_comreg; /* sky fp unit command register */
		unsigned long  u_reg[8]; /* sky fp unit state registers */
	} u_fps;
#endif
	int	u_stack[1];
					/* kernel stack per user
					 * extends from u + USIZE*64
					 * backward not to reach here
					 */
};
extern struct user u;

#define	u_rval1	u_r.r_reg.r_val1
#define	u_rval2	u_r.r_reg.r_val2
#define	u_roff	u_r.r_off
#define	u_rtime	u_r.r_time

/* ioflag values: Read/Write, User/Kernel, Ins/Data */
#define	U_WUD	0
#define	U_RUD	1
#define	U_WKD	2
#define	U_RKD	3
#define	U_WUI	4
#define	U_RUI	5

#define	EXCLOSE	01
