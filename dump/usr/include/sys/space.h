/* @(#)space.h	1.13 */
#include "sys/acct.h"
struct	acct	acctbuf;
struct	inode	*acctp;

#include "sys/tty.h"
struct	cblock	cfree[NCLIST];

#include "sys/buf.h"
struct	buf	bfreelist;	/* head of the free list of buffers */
struct	pfree	pfreelist;	/* Head of physio header pool */
struct	buf	pbuf[NPBUF];	/* Physical io header pool */
struct	buf	buf[NBUF];	/* buffer headers */
#ifdef BUFPCT
/*
 * Lisa: binit() takes the buffer space from free memory at boot, sized
 * at bufpct percent of it, between nbufmin and NBUF buffers.  Patch
 * bufpct in /unix to change the share.
 */
caddr_t	buffers;
#else
#define	BUFPCT	0
#define	NBUFMIN	0
char	bspace[NBUF*SBUFSIZE+sizeof(int)-1];	/* actual buffer space */
caddr_t	buffers = bspace;
#endif
int	bufpct = BUFPCT;	/* percent of free memory for buffers */
int	nbufmin = NBUFMIN;	/* fewest buffers */

struct	hbuf	hbuf[NHBUF];	/* buffer hash table */

#include "sys/scat.h"
#ifndef NONSCATLOAD
int	nscatfree;			/* number of items in freelist */
struct	scatter scatfreelist;		/* head of free list of scatter map */
struct	scatter scatmap[NSCATLOAD];	/* scatter map */
int	scsortmap[(NSCATLOAD+31)/32];	/* scatter map sort array */
#endif

#include "sys/file.h"
struct	file	file[NFILE];	/* file table */

#include "sys/inode.h"
struct	inode	inode[NINODE];	/* inode table */

#include "sys/proc.h"
struct	proc	proc[NPROC];	/* process table */

#include "sys/text.h"
struct	text text[NTEXT];	/* text table */
struct	svtext svtext[NSVTEXT];	/* save text table */

#include "sys/locking.h"
struct	locklist locklist[NFLOCK];

#include "sys/map.h"
struct map coremap[CMAPSIZ] = { mapdata(CMAPSIZ) };
struct map swapmap[SMAPSIZ] = { mapdata(SMAPSIZ) };

#include "sys/callo.h"
struct callo callout[NCALL];

#include "sys/mount.h"
struct mount mount[NMOUNT];

#include "sys/elog.h"
#include "sys/err.h"
struct	err	err = {
	NESLOT,
};

#include "sys/sysinfo.h"
struct sysinfo sysinfo;
struct syswait syswait;
struct syserr syserr;

#include "sys/opt.h"

#include "sys/var.h"
struct var v = {
	NBUF,
	NCALL,
	NINODE,
#ifdef lint
	&inode[NINODE],
	NFILE,
	&file[NFILE],
	NMOUNT,
	&mount[NMOUNT],
	NPROC,
	&proc[1],
	NTEXT,
	&text[NTEXT],
#else
	(char *)(&inode[NINODE]),
	NFILE,
	(char *)(&file[NFILE]),
	NMOUNT,
	(char *)(&mount[NMOUNT]),
	NPROC,
	(char *)(&proc[1]),
	NTEXT,
	(char *)(&text[NTEXT]),
#endif
	NCLIST,
	NSABUF,
	MAXUP,
	CMAPSIZ,
	SMAPSIZ,
	NHBUF,
	NHBUF-1,
	NFLOCK,
	NPHYS,
	CLSIZE,
#ifdef lint
	0,
#else
	ctob(stoc(1)),
#endif
	BSIZE,
	CXMAPSIZ,
	CLKTICK,
	HZ,
	USIZE,
	PAGESHIFT,
	PAGEMASK,
	SEGSHIFT,
	SEGMASK,
	USTART,
	UEND,
#ifdef lint
	&callout[NCALL],
#else
	(char *)(&callout[NCALL]),
#endif
	STACKGAP,
	CPU_MC68000,
	VER_MC68000,
	MMU_APPLE,
	DOFFSET,
	KVOFFSET,
	NSVTEXT,
#ifdef lint
	&svtext[NSVTEXT],
#else
	(char *)(&svtext[NSVTEXT]),
#endif
	NPBUF,
	NSCATLOAD,
};

#include "sys/init.h"

#ifndef	PRF_0
/* ARGSUSED */
prfintr(a, b) caddr_t a; short b; { }
int	prfstat;
#endif

#ifdef	VP_0
#include "sys/vp.h"
#endif

#ifdef	DISK_0
#define	RM05_0
#define	RP06_0
#define	RM80_0
#define	RP07_0
#ifndef	DISK_1
#define	DISK_1	0
#endif
#ifndef	DISK_2
#define	DISK_2	0
#endif
#ifndef	DISK_3
#define	DISK_3	0
#endif
#include "sys/iobuf.h"
#define	DISKS	(DISK_0+DISK_1+DISK_2+DISK_3)
struct iobuf gdtab[DISKS];
struct iobuf gdutab[DISKS*8];
int	gdindex[DISKS*8];
short	gdtype[DISKS*8];
struct iotime gdstat[DISKS*8];
#endif

#ifdef	TRACE_0
#include "sys/trace.h"
struct trace trace[TRACE_0];
#endif

#ifdef  CSI_0
#include "sys/csi.h"
#include "sys/csihdw.h"
struct csi csi_csi[CSI_0];
int csibnum = CSIBNUM;
struct csibuf *csibpt[CSIBNUM];
#endif

#ifdef	VPM_0
#include "sys/vpmt.h"
struct vpmt vpmt[VPM_0];
struct csibd vpmtbd[VPM_0*(XBQMAX + EBQMAX)];
struct vpminfo vpminfo =
	{XBQMAX, EBQMAX, VPM_0*(XBQMAX + EBQMAX)};
int vpmbsz= VPMBSZ;
#endif

#ifdef	DMK_0
#define MAXDMK 4
#include	"sys/dmk.h"
struct dmksave dmksave[MAXDMK];
#endif

#ifdef	X25_0
#include "sys/x25.h"
struct x25slot x25slot[X25_0];
struct x25tab x25tab[X25_0];
struct x25timer x25timer[X25_0];
struct x25link x25link[X25LINKS];
struct x25timer *x25thead[X25LINKS];
struct x25lntimer x25lntimer[X25LINKS];
struct csibd x25bd[X25BUFS];
struct csibuf x25buf;
struct x25info x25info =
	{X25_0, X25_0, X25LINKS, X25BUFS, X25BYTES};
#endif

#ifdef PCL11B_0
#include "sys/pcl.h"
#endif

#if MESG==1
#include	"sys/ipc.h"
#include	"sys/msg.h"

struct map	msgmap[MSGMAP];
struct msqid_ds	msgque[MSGMNI];
struct msg	msgh[MSGTQL];
char		msgspace[MSGSEG * MSGSSZ];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	MSGSEG
};
#endif

#if SEMA==1
#	ifndef IPC_ALLOC
#	include	"sys/ipc.h"
#	endif
#include	"sys/sem.h"
struct semid_ds	sema[SEMMNI];
struct sem	sem[SEMMNS];
struct map	semmap[SEMMAP];
struct	sem_undo	*sem_undo[NPROC];
#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)
#ifdef lint
struct sem_undo semu[SEMUSZ];
#else
int	semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];
#endif
union {
	short		semvals[SEMMSL];
	struct semid_ds	ds;
	struct sembuf	semops[SEMOPM];
}	semtmp;

struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};
#endif

#if SHMEM==1
#	ifndef	IPC_ALLOC
#	include	"sys/ipc.h"
#	endif
#include	"sys/shm.h"
struct shmid_ds	*shm_shmem[NPROC*SHMSEG];
struct shmid_ds	shmem[SHMMNI];	
struct	shmpt_ds	shm_pte[NPROC*SHMSEG];
struct	shminfo shminfo = {
	SHMMAX,
	SHMMIN,
	SHMMNI,
	SHMSEG,
	SHMBRK,
	SHMALL
};
#endif
#ifdef	NSC_0
#include "sys/nscdev.h"
#endif
#ifdef ST_0
#include "sys/st.h"
struct stbhdr	stihdrb[STIHBUF];
struct stbhdr	stohdrb[STOHBUF];
struct ststat	ststat = {
	STIBSZ,	/* input buffer size */
	STOBSZ,	/* output buffer size */
	STIHBUF,	/* # of buffer headers */
	STOHBUF,	/* # of buffer headers */
	STNPRNT	/* # of printer channels */
};

#endif
