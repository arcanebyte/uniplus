/*
 * System parameters.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 */
#include	"param.h"
#include	<sys/systm.h>
#include	<sys/buf.h>
#include	<sys/tty.h>
#include	<sys/conf.h>
#include	<sys/proc.h>
#include	<sys/text.h>
#include	<sys/dir.h>
#include	<sys/user.h>
#include	<sys/file.h>
#include	<sys/inode.h>
#include	<sys/filsys.h>
#include	<sys/mount.h>
#include	<sys/callout.h>
#include	<sys/acct.h>
#include	<sys/map.h>
#include	<sys/seg.h>

#define	HZ	60			/* Ticks/second of the clock */
#define	TIMEZONE (8 * 60)		/* Minutes westward from Greenwich */
#define	DSTFLAG	1			/* Daylight Saving Time applies here */

#define	NBUF	(12 + (2 * MAXUSERS))	/* size of buffer cache, must be <=256*/
#define	NMOUNT	7			/* number of mountable file systems */

#ifdef	UCB_CLIST
#   ifdef UNIBUS_MAP
#	define NCLIST	500		/* number of clists, must be <= 512 */
#   else
#	define NCLIST	200		/* number of clists */
#   endif
#else	UCB_CLIST
#	define NCLIST	100		/* number of clists */
#endif	UCB_CLIST

#define	NPROC	(10 + (7 * MAXUSERS))		/* max number of processes */
#define	NTEXT	(20 + ((3*MAXUSERS) / 2))	/* max number of pure texts */
#define	NINODE	(NPROC + 20 + (2 * MAXUSERS))	/* number of in-core inodes */
#define	NFILE	((8 * NINODE/10) + 5)		/* number of file structures */
#define	NCALL	(4 + MAXUSERS)		/* max simultaneous time callouts */
#define	NDISK	3			/* number of disks to monitor */

#ifndef	UNIBUS_MAP
#   define CMAPSIZ NPROC		/* size of core allocation map */
#   define SMAPSIZ (NPROC+(5*NTEXT/10))	/* size of swap allocation map */
#else
#   define CMAPSIZ (NPROC+(8*NTEXT/10)) /* size of core allocation map */
#   define SMAPSIZ (NPROC+(8*NTEXT/10)) /* size of swap allocation map */
#endif

int	maxusers = MAXUSERS;
int	hz	= HZ;
int	timezone = TIMEZONE;
bool_t	dstflag	= DSTFLAG;
int	nmount	= NMOUNT;
int	nfile	= NFILE;
int	ninode	= NINODE;
int	nproc	= NPROC;
int	ntext	= NTEXT;
int	nbuf	= NBUF;
int	nclist	= NCLIST;
int	ncallout = NCALL;
int	ndisk	= NDISK;
int	cmapsiz	= CMAPSIZ;
int	smapsiz	= SMAPSIZ;

struct	mount	mount[NMOUNT];
struct	inode	inode[NINODE];
struct	buf	buf[NBUF];
struct	callout	callout[NCALL + 1];	/* last one used as a delimiter */
struct	buf	bfreelist;
#ifndef	UCB_CLIST
struct	cblock	cfree[NCLIST];
#else
unsigned clstdesc = ((((btoc(NCLIST*sizeof(struct cblock)))-1) << 8) | RW);
#endif
long	dk_time[1 << (NDISK)];
long	dk_numb[NDISK];
long	dk_wds[NDISK];

struct mapent _coremap[CMAPSIZ];
struct map coremap[1] = {
	_coremap,
	&_coremap[CMAPSIZ],
	"coremap"
};

struct mapent _swapmap[SMAPSIZ];
struct map swapmap[1] = {
	_swapmap,
	&_swapmap[SMAPSIZ],
	"swapmap"
};

struct	mount	*mountNMOUNT	= &mount[NMOUNT];
struct	file	*fileNFILE	= &file[NFILE];
struct	inode	*inodeNINODE	= &inode[NINODE];
struct	proc	*procNPROC	= &proc[NPROC];
struct	text	*textNTEXT	= &text[NTEXT];
/* callNCALL points to the last slot, which must be a terminator */
struct	callout	*callNCALL	= &callout[NCALL];

#ifdef	UCB_METER
char	counted[NTEXT];
#endif

int	bsize	= BSIZE + BSLOP;	/* size of buffers */

#ifdef ACCT
struct	acct	acctbuf;
struct	inode	*acctp;
#endif

char msgbuf[MSGBUFS]	= {"\0"};

/*
 *  Declarations of structures loaded last and allowed to
 *  reside in the 0120000-140000 range (where buffers and clists are
 *  mapped).  These structures must be extern everywhere else,
 *  and the asm output of cc is edited to move these structures
 *  from comm to bss (which is last) (see the script :comm-to-bss).
 */
int	remap_area;	/* start of possibly mapped area; must be first */
struct	proc	proc[NPROC];
struct	file	file[NFILE];
struct	text	text[NTEXT];
