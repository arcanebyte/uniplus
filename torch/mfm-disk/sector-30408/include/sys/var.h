/* @(#)var.h	6.1 */
struct var {
	int	v_buf;		/* NBUF */
	int	v_call;		/* NCALL */
	int	v_inode;	/* NINODE */
#ifdef lint
	struct inode *ve_inode;	/* &inode[NINODE] */
	int	v_file;		/* NFILE */
	struct file *ve_file;	/* &file[NFILE] */
	int	v_mount;	/* NMOUNT */
	struct mount *ve_mount;	/* &mount[NMOUNT] */
	int	v_proc;		/* NPROC */
	struct proc *ve_proc;	/* &proc[1] */
	int	v_text;		/* NTEXT */
	struct text *ve_text;	/* &text[NTEXT] */
#else
	char	*ve_inode;	/* (char *)(&inode[NINODE]) */
	int	v_file;		/* NFILE */
	char	*ve_file;	/* (char *)(&file[NFILE]) */
	int	v_mount;	/* NMOUNT */
	char	*ve_mount;	/* (char *)(&mount[NMOUNT]) */
	int	v_proc;		/* NPROC */
	char	*ve_proc;	/* (char *)(&proc[1]) */
	int	v_text;		/* NTEXT */
	char	*ve_text;	/* (char *)(&text[NTEXT]) */
#endif
	int	v_clist;	/* NCLIST */
	int	v_sabuf;	/* NSABUF */
	int	v_maxup;	/* MAXUP */
	int	v_cmap;		/* CMAPSIZ */
	int	v_smap;		/* SMAPSIZ */
	int	v_hbuf;		/* NHBUF */
	int	v_hmask;	/* NHBUF-1 */
	int	v_flock;	/* NFLOCK */
	int	v_phys;		/* NPHYS */
	int	v_clsize;	/* CLSIZE */
	int	v_txtrnd;	/* ctos(1) */
	int	v_bsize;	/* DEV_BSIZE */
	int	v_cxmap;	/* CXMAPSIZ for sun MMU for 68451 */
	int	v_clktick;	/* CLKTICK */
	int	v_hz;		/* HZ */
	int	v_usize;	/* USIZE */
	int	v_pageshift;	/* PAGESHIFT */
	int	v_pagemask;	/* PAGEMASK */
	int	v_segshift;	/* SEGSHIFT */
	int	v_segmask;	/* SEGMASK */
	int	v_ustart;	/* USTART */
	int	v_uend;		/* UEND */
#ifdef lint
	struct callout *ve_call;/* &callout[NCALL] */
#else
	char	*ve_call;	/* (char *)(&callout[NCALL]) */
#endif
	int	v_stkgap;	/* STACKGAP */
	int	v_cputype;	/* CPU_MC68000 */
	int	v_cpuver;	/* VER_MC68000 */
	int	v_mmutype;	/* MMU_SINGLE */
	int	v_doffset;	/* DOFFSET */
	int	v_kvoffset;	/* KVOFFSET */
	int	v_svtext;	/* NSVTEXT */
#ifdef lint
	struct svtext *ve_svtext;/* &svtext[NSVTEXT] */
#else
	char	*ve_svtext;	/* (char *)(&svtext[NSVTEXT]) */
#endif
	int	v_pbuf;		/* NPBUF */
	int	v_nscatload;	/* NSCATLOAD */
	struct user *v_udot;	/* address of udot */
	int	v_nmbufs;	/* NMBUFS */
	int	v_npty;		/* NPTY */
	int	v_maxcore;	/* MAXCORE (.../GEN/sys/heap_kmem.c) */
	int	v_maxheader;	/* MAXHEADER (.../GEN/sys/heap_kmem.c) */
	int	v_nstream;	/* NSTREAM */
	int	v_nqueue;	/* NQUEUE */
	int	v_nblk4096;	/* NBLK4096 */
	int	v_nblk2048;	/* NBLK2048 */
	int	v_nblk1024;	/* NBLK1024 */
	int	v_nblk512;	/* NBLK512 */
	int	v_nblk256;	/* NBLK256 */
	int	v_nblk128;	/* NBLK128 */
	int	v_nblk64;	/* NBLK64 */
	int	v_nblk16;	/* NBLK16 */
	int	v_nblk4;	/* NBLK4 */
#ifdef	lint
	struct 	proc *ve_proctab;	/* &proc[0]	*/
#else
	char	 *ve_proctab;	/* &proc[0]	*/
#endif	lint
	int	v_floattype;	/* FLOAT_NONE/FLOAT_SKY/FLOAT_68881... */
	/* padding make all binaries compatible, even if we add afew fields */
	int	v_fill[128-63];	/* sized to make var 512 bytes long */
};
extern struct var v;
