/*	@(#)shm.h UniPlus+ v.0.1.1 */
/* @(#)shm.h	1.2 */
/*
**	IPC Shared Memory Facility.
*/

/*
**	Implementation Constants.
*/

#define	SHMLBA	ctob(1)	/* segment low boundary address multiple */
			/* (SHMLBA must be a power of 2) */

/*
**	Permission Definitions.
*/

#define	SHM_R	0400	/* read permission */
#define	SHM_W	0200	/* write permission */

/*
**	ipc_perm Mode Definitions.
*/

#define	SHM_CLEAR	01000	/* clear segment on next attach */
#define	SHM_DEST	02000	/* destroy segment when # attached = 0 */

/*
**	Message Operation Flags.
*/

#define	SHM_RDONLY	010000	/* attach read-only (else read-write) */
#define	SHM_RND		020000	/* round attach address to SHMLBA */

/*
**	Structure Definitions.
*/

/*
**	There is a shared mem id data structure for each segment in the system.
*/

struct shmid_ds {
	struct ipc_perm	shm_perm;	/* operation permission struct */
	int		shm_segsz;	/* segment size (in bytes) */
#ifdef notdef
	struct pt_entry	*shm_ptbl;	/* ptr to page table */
#endif
	union {
		short	Shm_first;	/* first memory pointer */
#define shm_first	Shm.Shm_first
		short	Shm_scat;	/* scatter load memory pointers */
#define shm_scat	Shm.Shm_scat
	} Shm;
	ushort		shm_lpid;	/* pid of last shmop */
	ushort		shm_cpid;	/* pid of creator */
	ushort		shm_nattch;	/* current # attached */
	ushort		shm_cnattch;	/* in memory # attached */
	time_t		shm_atime;	/* last shmat time */
	time_t		shm_dtime;	/* last shmdt time */
	time_t		shm_ctime;	/* last change time */
};

struct	shmpt_ds {
	int	shm_segbeg;	/* virtual start of shared data (in clicks) */
	short	shm_sflg;	/* R/W permission on segment */
	short	shm_seg;	/* segment number */
};

struct	shminfo {
	int	shmmax,	/* max shared memory segment size */
		shmmin,	/* min shared memory segment size */
		shmmni,	/* # of shared memory identifiers */
		shmseg,	/* max attached shared memory segments per process */
		shmbrk,	/* gap (in clicks) used between data and shared memory */
		shmall;	/* max total shared memory system wide (in clicks) */
};
