/*
 *  Configuration information
 */

/* #define	DISK_0 1 */

#define	NBUF	30
#define	NINODE	50
#define	NFILE	60
#define	NMOUNT	8
#define	CMAPSIZ	50	/* also in reinit.c */
#define	SMAPSIZ	50	/* also in reinit.c */
#define	CXMAPSIZ 50
#define	NCALL	15
#define	NPROC	30
#define	NTEXT	20
#define	NSVTEXT	20
#define	NCLIST	100
#define	STACKGAP 8
#define	NSABUF	5
#define	POWER	0
#define	MAXUP	25
#define	NHBUF	64
#define	NPBUF	4
#define NFLOCK  200
#define	X25LINKS	1
#define	X25BUFS	256
#define	X25MAPS	30
#define	X25BYTES	(16*1024)
#define	CSIBNUM	20
#define	VPMBSZ	8192
#define	MESG	1
#define	MSGMAP	100
#define	MSGMAX	8192
#define	MSGMNB	16384
#define	MSGMNI	50
#define	MSGSSZ	8
#define	MSGTQL	40
#define	MSGSEG	1024
#define	SEMA	1
#define	SEMMAP	10
#define	SEMMNI	10
#define	SEMMNS	60
#define	SEMMNU	30
#define	SEMMSL	25
#define	SEMOPM	10
#define	SEMUME	10
#define	SEMVMX	32767
#define	SEMAEM	16384
#define	SHMEM	1
#define	SHMMAX	(128*1024)
#define	SHMMIN	1
#define	SHMMNI	100
#define	SHMBRK	16
#define	SHMALL	512
#define	STIHBUF	(ST_0*4)
#define	STOHBUF	(ST_0*4)
#define	STNPRNT	(ST_0>>2)
#define	STIBSZ	8192
#define	STOBSZ	8192


#include	"sys/param.h"
#include	"sys/config.h"
#include	"sys/mmu.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/conf.h"
#include	"sys/cpuid.h"
#include	"sys/space.h"
#include	"sys/io.h"
#include	"sys/termio.h"
#include	"sys/reg.h"
#include	"sys/scc.h"
#include	"sys/pport.h"
#include	"sys/swapsz.h"

extern nodev(), nulldev();
extern proopen(), proread(), prowrite(), prostrategy(), proprint(), proioctl();
extern snbopen(), sncopen(), snbclose(), sncclose(), snread(), snwrite(), snstrategy(), snprint(), snioctl();
extern cvopen(), cvread(), cvwrite(), cvstrategy(), cvprint();
extern pmopen(), pmread(), pmwrite(), pmstrategy(), pmprint(), pmioctl();
extern coopen(), coclose(), coread(), cowrite(), coioctl();
extern syopen(), syread(), sywrite(), syioctl();		
extern mmread(), mmwrite();			
extern scopen(), scclose(), scread(), scwrite(), scioctl();
extern erropen(), errclose(), errread();			
extern proread(), prowrite(), proioctl();			
extern ejioctl();						
extern msopen(), msclose(), msread(), msioctl();		
extern lpopen(), lpclose(), lpwrite(), lpioctl();		
extern skopen(), skclose(), skwrite();		
extern rtcread(), rtcwrite();		
extern teopen(), teclose(), teread(), tewrite(), teioctl();

#ifdef UCB_NET
extern int ptsopen(), ptsclose(), ptsread(), ptswrite();
extern int ptcopen(), ptcclose(), ptcread(), ptcwrite();
extern int ptsioctl(), ptcioctl();
#endif

struct bdevsw bdevsw[] = {
	proopen,  nulldev,  prostrategy, proprint,	/* 0 */
	snbopen,  snbclose, snstrategy,  snprint,	/* 1 */
	cvopen,   nulldev,  cvstrategy,  cvprint,	/* 2 */
	pmopen,   nulldev,  pmstrategy,  pmprint,	/* 3 */
};

struct cdevsw cdevsw[] = {
	coopen,  coclose,  coread,  cowrite,  coioctl,	0,	/* 0 */
	syopen,  nulldev,  syread,  sywrite,  syioctl,	0,	/* 1 */
	nulldev, nulldev,  mmread,  mmwrite,  nodev,	0,	/* 2 */
	erropen, errclose, errread, nodev,    nodev, 	0,	/* 3 */
	scopen,  scclose,  scread,  scwrite,  scioctl,	0,	/* 4 */
	proopen, nulldev,  proread, prowrite, proioctl,	0,	/* 5 */
	sncopen, sncclose, snread,  snwrite,  snioctl,	0,	/* 6 */
	nulldev, nulldev,  nodev,   nodev,    ejioctl,	0,	/* 7 */
	lpopen,  lpclose,  nodev,   lpwrite,  lpioctl,	0,	/* 8 */
	msopen,  msclose,  msread,  nodev,    msioctl,	0,	/* 9 */
	skopen,  skclose,  nodev,   skwrite,  nodev,	0,	/* 10 */
	cvopen,  nulldev,  cvread,  cvwrite,  nulldev,	0,	/* 11 */
	pmopen,  nulldev,  pmread,  pmwrite,  pmioctl,	0,	/* 12 */
	nulldev, nulldev,  rtcread, rtcwrite, nulldev,	0,	/* 13 */
	teopen,  teclose,  teread,  tewrite,  teioctl,	0,	/* 14 */
#ifdef UCB_NET
	nodev,   nodev,    nodev,   nodev,    nodev,	0,	/* 15 */
	nodev,   nodev,    nodev,   nodev,    nodev,	0,	/* 16 */
	nodev,   nodev,    nodev,   nodev,    nodev,	0,	/* 17 */
	nodev,   nodev,    nodev,   nodev,    nodev,	0,	/* 18 */
	nodev,   nodev,    nodev,   nodev,    nodev,	0,	/* 19 */
	ptcopen, ptcclose, ptcread, ptcwrite, ptcioctl, 0,	/* ptc 20 */
	ptsopen, ptsclose, ptsread, ptswrite, ptsioctl, 0,	/* pts 21 */
#endif
};

int	bdevcnt = sizeof(bdevsw)/sizeof(bdevsw[0]);
int	cdevcnt = sizeof(cdevsw)/sizeof(cdevsw[0]);

#ifdef SUNIX	/* Sony (installation) root filesystem */
dev_t	rootdev = makedev(1, 0);
dev_t	pipedev = makedev(1, 0);
dev_t	dumpdev = makedev(1, 0);
/* nswap and swapdev are set in lisainit in config.c */
dev_t	swapdev = makedev(0, 1);
daddr_t	swplo = 0;
int nswap =	PRNSWAP;

#else SUNIX	/* ProFile root filesystem */
#define ROOTBASE	0	/* (port * 16) for port=0,1,2,4,5,7, or 8 */
dev_t	rootdev = makedev(0, ROOTBASE);
dev_t	pipedev = makedev(0, ROOTBASE);
dev_t	dumpdev = makedev(0, ROOTBASE);
dev_t	swapdev = makedev(0, ROOTBASE + 1);
daddr_t	swplo = 0;
int nswap =	PRNSWAP;
#endif SUNIX

int	(*dump)() = nulldev;
int	dump_addr = 0x0000;

int	(*pwr_clr[])() = {
	(int (*)())0
};

int	(*dev_init[])() = 
{
	(int (*)())0
};

#ifdef SCC_CONSOLE
int	scputchar();
int	(*putchar)() = scputchar;
#else
int	coputchar();
int	(*putchar)() = coputchar;
#endif

#ifdef UCB_NET
#define PTC_DEV 20
int ptc_dev = PTC_DEV;
#endif

int co_cnt = 1;
struct tty co_tty[1];

struct ttyptr co_ttptr[] = {
	1,	&co_tty[0],		/* tt_addr field not used */
	0,
};

int sc_cnt = NSC;
struct tty sc_tty[NSC];
char sc_modem[NSC];

struct ttyptr sc_ttptr[]  = {
	0xFCD240,	&sc_tty[1],
	0xFCD242, 	&sc_tty[0],
	0,
};

struct scline sc_line[] = {
	W9BRESET,	(4000000/16),	/* clock frequency b */
	W9ARESET,	(4000000/16),	/* clock frequency a */
};

#if NTE != 0
int te_cnt = NTE;
struct tty te_tty[NTE];
char te_dparam[NTE];
char te_modem[NTE];

struct ttyptr te_ttptr[NTE+1];	/* +1 for pstat */
#endif

/*
 * pointers to ttyptr structures for terminal monitoring programs
 */
struct ttyptr *tty_stat[] = {
	co_ttptr,
	sc_ttptr,
#if NTE != 0
	te_ttptr,
#endif
	0
};

/*
 * tty output low and high water marks
 */
#define TTHIGH
#ifdef TTLOW
#define M	1
#define N	1
#endif
#ifdef TTHIGH
#define M	3
#define N	1
#endif
int	tthiwat[16] = {
	0*M,	60*M,	60*M,	60*M,	60*M,	60*M,	60*M,	120*M,
	120*M,	180*M,	180*M,	240*M,	240*M,	240*M,	100*M,	100*M,
};
int	ttlowat[16] = {
	0*N,	20*N,	20*N,	20*N,	20*N,	20*N,	20*N,	40*N,
	40*N,	60*N,	60*N,	80*N,	80*N,	80*N,	50*N,	50*N,
};

/*
 * Default terminal characteristics
 */
char	ttcchar[NCC] = {
	CINTR,
	CQUIT,
	CERASE,
	CKILL,
	CEOF,
	0,
	0,
	0
};

#ifdef lint
/* LINTLIBRARY */
forlint()
{
	bminit();
	nmikey();
	l1intr((struct args *)0);
	kbintr();
	scintr((struct args *)0);
	pmintr((struct args *)0);
	ebintr(0);
	netintr();
}
#endif

#ifdef UCB_NET
#include <net/misc.h>
#include <net/ubavar.h>
extern struct uba_driver ebdriver;
struct uba_device ubdinit[] = {
	/* driver,	unit,	addr,	flags*/
	{ &ebdriver,    0,   (caddr_t)5,  0x59002908 },  /* net 89 */
	0
};

int iff_noarp = 0;	/* 0 -> do ARP;  not 0 -> no ARP */
#endif
