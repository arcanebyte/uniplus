/* NFSSRC @(#)proc.h	2.1 86/04/11 */
/* @(#)proc.h	6.1 */
/*
 * One structure allocated per active process. It contains all data needed
 * about the process while the process may be swapped out.
 * Other per process data (user.h) is swapped with the process.
 */

struct	proc {
	short	p_flag;		/* process flag */
	char	p_stat;
	char	p_pri;		/* priority, negative is high */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for cpu usage */
	char	p_time;		/* resident time for scheduling */

	ushort	p_uid;		/* real user id */
	ushort	p_suid;		/* set (effective) user id */
	short	p_pgrp;		/* name of process group leader */
	short	p_pid;		/* unique process id */
	short	p_ppid;		/* process id of parent */

	union {
		caddr_t P_wchan;	/* event process is awaiting */
		struct locklist *P_wlock;/* locking event address */
		int	p_int;
	} P_unw;
#define p_wchan	P_unw.P_wchan
#define p_wlock	P_unw.P_wlock
#define	p_arg	P_unw.p_int	/* used in xque, xsched */

	char	p_cursig;
	long	p_sig;		/* signals pending to this process */
	int	p_sigmask;	/* current signal mask (BSD) */
	int	p_sigignore;	/* signals being ignored (BSD) */
	int	p_sigcatch;	/* signals being caught by user (BSD) */

	struct proc *p_link;	/* linked list of running processes */
	struct proc *p_parent;	/* ptr to parent process */
#define p_pptr	p_parent
	struct proc *p_child;	/* ptr to first child process */
	struct proc *p_sibling;	/* ptr to next sibling */

	union {
		struct {
			int P_ua;	/* address of U area */
			int P_da;	/* address of data area */
			int P_sa;	/* end address of stack area */
		} P_a;
#define p_uaddr P_una.P_a.P_ua
#define p_daddr P_una.P_a.P_da
#define p_saddr P_una.P_a.P_sa
		struct {
			union {
				int P_addr;	/* address of swappable image */
				daddr_t P_dkaddr;/* disk address if swapped */
			} P_adun;
			short P_scat;	/* scatter load index */
		} P_ad;
#define p_addr   P_una.P_ad.P_adun.P_addr
#define p_dkaddr P_una.P_ad.P_adun.P_dkaddr
#ifndef NONSCATLOAD
#define p_scat   P_una.P_ad.P_scat
#endif
	} P_una;
	union {
		struct {
			int P_ds;	/* size of data area */
			int P_ss;	/* size of stack area */
		} P_s;
#define p_dsize P_uns.P_s.P_ds
#define p_ssize P_uns.P_s.P_ss
		struct {
			int P_size;	/* size of swappable image */
			int P_maxm;	/* maxmem (in clicks) for process */
		} P_sz;
#define p_size	 P_uns.P_sz.P_size
#define	p_maxmem P_uns.P_sz.P_maxm
	} P_uns;
	union {
		struct {
			struct dp *P_dpd;	/* ptr to data descriptors */
			struct dp *P_dps;	/* ptr to stack descriptors */
			struct dp *P_dpphys;	/* ptr to phys descriptor */
			struct dp *P_dpshm;	/* ptr to shared mem descrptr */
			struct proc *P_dpf;	/* fwd ptr for dp link list */
			struct proc *P_dpb;	/* bkwd ptr for dp link list */
			char	P_asn;		/* proc address space number */
		} P_dp;
#define p_dpd	P_un.P_dp.P_dpd
#define p_dps	P_un.P_dp.P_dps
#define p_dpphys P_un.P_dp.P_dpphys
#define p_dpshm P_un.P_dp.P_dpshm
#define p_dpf	P_un.P_dp.P_dpf
#define p_dpb	P_un.P_dp.P_dpb
#define p_asn	P_un.P_dp.P_asn
		struct {
			struct context *P_context; /* ptr to context struct */
		} P_cx;
#define p_context P_un.P_cx.P_context
	} P_un;

	struct text *p_textp;	/* pointer to text structure */
	int	p_clktim;	/* time to alarm clock signal */
	int	p_smbeg;	/* beginning click for shared memory */
	int	p_smend;	/* ending click for shared memory */
	daddr_t	p_xaddr[NSCATSWAP];	/* addresses of swappable images */
	short	p_xsize[NSCATSWAP];	/* sizes of swappable images */

	struct itimerval p_realtimer;

	long	p_compatflags;	/* compatibility mode flags */
};

extern struct proc proc[];	/* the proc table itself */

/* stat codes */
#define	SSLEEP	1		/* awaiting an event */
#define	SWAIT	2		/* (abandoned state) */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */
#define	SXBRK	7		/* process being xswapped */
#define	SXSTK	8		/* process being xswapped */
#define	SXTXT	9		/* process being xswapped */

/* flag codes */
#define	SLOAD	0x000001	/* in core */
#define	SSYS	0x000002	/* scheduling process */
#define	SLOCK	0x000004	/* process cannot be swapped */
#define	SSWAP	0x000008	/* process is being swapped out */
#define	STRC	0x000010	/* process is being traced */
#define	SWTED	0x000020	/* another tracing flag */

/* UniSoft additions */
#ifndef NONSCATLOAD
#define	SCONTIG	0x000040	/* process is contiguous loaded */
#endif
#define SNOMMU  0x000080	/* do not set up the mmu registers */
#define	SPGRPL	0x000100	/* login-type process group leader */
#ifndef NONSCATLOAD
#define	SSWAPIT	0x000200	/* swap process out */
#endif
#define	SCOFF	0x000400	/* COFF binary format */
#define	SSPART	0x000800	/* process is partially swapped out */
#define SSEL    0x001000	/* selecting, cleared if rescan needed */
#define STIMO   0x002000	/* timing out during sleep */
#define SPGRP42	0x004000	/* 4.2 style process groups for job control */
#define	SOMASK	0x008000	/* restore old mask after taking signal */

/*
 * parallel proc structure
 * to replace part with times
 * to be passed to parent process
 * in ZOMBIE state.
 */
#ifndef NPROC
struct	xproc {
	short	xp_flag;
	char	xp_stat;
	char	xp_pri;		/* priority, negative is high */
	char	xp_cpu;		/* cpu usage for scheduling */
	char	xp_nice;	/* nice for cpu usage */
	char	xp_time;	/* resident time for scheduling */

	ushort	xp_uid;		/* real user id */
	ushort	xp_suid;	/* set (effective) user id */
	short	xp_pgrp;	/* name of process group leader */
	short	xp_pid;		/* unique process id */
	short	xp_ppid;	/* process id of parent */

	short	xp_addr;

	short	xp_xstat;	/* Exit status for wait */
	time_t	xp_utime;	/* user time, this proc */
	time_t	xp_stime;	/* system time, this proc */
};
#endif
