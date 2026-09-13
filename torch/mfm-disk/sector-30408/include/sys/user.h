/*
 * %Z%ROOT	%M%	%I%	%E%	%U%
 * @(#)UniSoft	user.h	6.1
 */

#include	<sys/resource.h>

/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*click bytes long.
 * It contains the system stack per user; is cross referenced
 * with the proc structure for the same process.
 */

#define	COMMSIZ	14		/* size of comm field (== SVFSDIRSIZ) */
#define	USERSIZ	4		/* size of area reserved for OEM/user use */

 
struct	user {
	label_t	u_rsav;			/* save info when exchanging stacks */
	label_t	u_qsav;			/* for quits and interrupts */
	label_t	u_ssav;			/* for swapping */
	int	*u_nofault;		/* ``nofault'' save area */
	struct	proc *u_procp;		/* pointer to proc structure */

	int	u_arg[10];		/* arguments to current system call */
	int	*u_ap;			/* pointer to arglist */
	union {				/* syscall return values */
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
	} u_r;
#define	u_rval1	u_r.r_reg.r_val1
#define	u_rval2	u_r.r_reg.r_val2
#define	u_roff	u_r.r_off
#define	u_rtime	u_r.r_time
	short	u_errcnt;		/* syscall error count */
	char	u_error;		/* return error code */
	char	u_eosys;		/* special action on end of syscall */

	short	u_fmode;		/* file mode for IO */
	ushort	u_pbsize;		/* bytes in block for IO */
	ushort	u_pboff;		/* offset in block for IO */
	dev_t	u_pbdev;		/* real device for IO */
	daddr_t	u_rablock;		/* read ahead block addr */

	struct	vnode *u_cdir;		/* ptr to vnode of current directory */
	struct	vnode *u_rdir;		/* root directory of current process */
	caddr_t	u_dirp;			/* pathname pointer */
	struct	vnode *u_pdir;		/* vnode of parent directory of dirp */

	struct	file *u_ofile[NOFILE];	/* file structures of open files */
	char	u_pofile[NOFILE];	/* per-process flags of open files */
#define	UF_EXCLOSE 	0x1		/* auto-close on exec */

	unsigned u_tsize;		/* text size (clicks) */
	unsigned u_dsize;		/* data size (clicks) */
	unsigned u_ssize;		/* stack size (clicks) */

	int	(*u_signal[NSIG])();	/* disposition of signals */
	int	u_sigmask[NSIG];        /* signals to be blocked */
	int	u_sigonstack;		/* signals to take on sigstack */
	int	u_oldmask;		/* saved mask from before sigpause */
	struct	sigstack u_sigstack;	/* sp & on stack state variable */
	short	u_traptype;		/* type of last trap for m68k w/ COFF */
	int	(*u_sigcode)();		/* pointer to user sigcode routine
					   for 4.2 style signals */
#define	u_onstack       u_sigstack.ss_onstack
#define	u_sigsp         u_sigstack.ss_sp

	time_t	u_utime;		/* this process user time */
	time_t	u_stime;		/* this process system time */
	time_t	u_cutime;		/* sum of childs' utimes */
	time_t	u_cstime;		/* sum of childs' stimes */
	struct	itimerval u_timer[3];

	int	*u_ar0;			/* address of users saved R0 */

	struct	ucred *u_cred;		/* user credentials (uid, gid, etc) */
#define	u_uid		u_cred->cr_uid
#define	u_gid		u_cred->cr_gid
#define	u_groups	u_cred->cr_groups
#define	u_ruid		u_cred->cr_ruid
#define	u_rgid		u_cred->cr_rgid

	struct {			/* profile arguments */
		short	*pr_base;	/* buffer base */
		unsigned pr_size;	/* buffer size */
		unsigned pr_off;	/* pc offset */
		unsigned pr_scale;	/* pc scaling */
	} u_prof;

	short	*u_ttyp;		/* pointer to pgrp in "tty" struct */
	dev_t	u_ttyd;			/* controlling tty dev */

	struct exdata {			/* header of executable file */
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
#define	ux_tstart	ux_unused
	char	u_comm[COMMSIZ];
	time_t	u_start;
	time_t	u_ticks;

	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	char	u_acflag;

	daddr_t	u_limit;		/* maximum write address */
	struct	rlimit u_rlimit[RLIM_NLIMITS];

	short	u_cmask;		/* mask for file creation */
	short	u_fcode;		/* function code on bus errors */
	long	u_aaddr;		/* access address on bus errors */
	short	u_ireg;			/* instruction register on bus errors*/
	struct	phys {
		int	u_phladdr;	/* phys logical address */
		int	u_phsize;	/* phys size */
		int	u_phpaddr;	/* phys physical address */
		int	u_phfill;	/* fill */
	} u_phys[NPHYS];
	short	u_lock;			/* process/text locking flags */

	unsigned int u_ptsize;		/* text size (clicks) for sureg */
	unsigned int u_pdsize;		/* data size (clicks) for sureg */
	unsigned int u_pssize;		/* stack size (clicks)for sureg */
	int	u_xrw;			/* rw flag for sureg */

#ifdef FLOAT
	char	u_fpsaved;		/* fp unit state saved */
	char	u_fpinuse;		/* fp unit in use */
	struct {
		unsigned short u_comreg; /* sky fp unit command register */
		unsigned long  u_reg[8]; /* sky fp unit state registers */
	} u_fps;
#endif FLOAT
#ifdef mc68881
	char	u_fpsaved;		/* fp unit state saved */
	char	u_fpexc;		/* last exception which caused SIGFPE */
	char	u_fpfill[2];		/* unused */
	long	u_fpsysreg[3];		/* system regs CONTROL/STATUS/IADDR */
#define	FPDSZ	12			/* size of fp data reg FP0-FP7 */
	char	u_fpdreg[8][FPDSZ];	/* data regs FP0,FP1,...,FP7 */
	char	u_fpstate[184];		/* internal state, FSAVE/FRESTORE */
#endif mc68881

	long	u_user[USERSIZ];	/* reserved for OEM/user use */

	int	u_usrtop;		/* top of user area */
	int	u_stack[1];
					/* kernel stack per user
					 * extends from u + USIZE*64
					 * backward not to reach here
					 */
};
extern struct user u;

/* ioflag values: Read/Write, User/Kernel, Ins/Data */
#define	U_WUD	0
#define	U_RUD	1
#define	U_WKD	2
#define	U_RKD	3
#define	U_WUI	4
#define	U_RUI	5

/* u_eosys values */
#define	JUSTRETURN	0
#define	RESTARTSYS	1
#define	SIMULATERTI	2
#define	REALLYRETURN	3

#define	EXCLOSE		01
#define	TRAPNORM	0		/* "normal" trap */
#define	TRAPBUS		1		/* bus error trap */
#define	TRAPADDR	2		/* address error trap */
#define	TRAPLONG	0x8000		/* long buserr frame - 68010 && 68020 */
#define	TRAPSHORT	0x4000		/* short buserr frame - 68020 only */

struct ucred {
	u_short	cr_ref;			/* reference count */
	short   cr_uid;			/* effective user id */
	short   cr_gid;			/* effective group id */
	int     cr_groups[NGROUPS];	/* groups, 0 terminated */
	short   cr_ruid;		/* real user id */
	short   cr_rgid;		/* real group id */
};
#ifdef KERNEL
#define	crhold(cr)	(cr)->cr_ref++
struct ucred *crget();
struct ucred *crcopy();
struct ucred *crdup();
#endif
