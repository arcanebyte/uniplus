/*
 * Copyright 1982 UniSoft Corporation
 */

/*
 * Context structure.
 * One allocated per active page
 * register context. There are
 * NUMCONTX (16) contexts in hardware.
 */
struct context {
	struct	context *cx_forw;	/* forward pointer */
	struct	context *cx_back;	/* back pointer */
	struct	proc *cx_proc;		/* pointer to proc structure */
	int	cx_daddr;		/* starting data segment address */
	short	cx_dsize;		/* data size of allocated segments */
	short	cx_num;			/* context number of this slot 1 to 1 */
	struct cxphys {
		int	cx_phaddr;	/* sysphys address slot */
		int	cx_phsize;	/* sysphys data slot count */
	} cx_phys[NPHYS];
	struct cxshm {
		int	cx_shmaddr;	/* shared memory address slot */
		int	cx_shmsize;	/* shared memory data slot count */
	} cx_shm[SHMSEG];
};

extern struct context context[];

struct context *cxalloc();
struct context *cxunlink();

#ifdef lint
struct dp {
	int	dp_anything;
};
#endif
