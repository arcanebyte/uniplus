/* @(#)param.h	1.6 */
/*
 * fundamental variables
 * don't change too often
 */


#define	NOFILE	20		/* max open files per process */
#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define MAXMEM	(btoc(v.v_uend-v.v_ustart))/* Maximum size of user program */
#define	MAXCOUNT (512*125)	/* max byte count possible for phys IO */
#define	SWAPSIZE	64	/* granularity of partial swaps (in clicks) */
#define	SSIZE	1		/* initial stack size (in clicks) */
#define	SINCR	1		/* increment of stack (in clicks) */
#define	USIZE	4		/* size of user block (in clicks) */
#define	NSWB	3		/* size of swap pool */

#define USTART	0x00000		/* logical start of user program */
#define UEND	0xFA0000	/* logical end of user program +1 */
#define DOFFSET	0		/* Data offset */
#define KVOFFSET 0x0		/* Kernel virtual offset from physical 0 mem */
#define NIVEC	95		/* number of interrupt vectors */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	60		/* Ticks/second of the clock */
#define	CLKTICK	16666		/* microseconds in a clock tick */
#define	NCARGS	5120		/* # characters in exec arglist */

/*
 * priorities
 * should not be altered too much
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */

#define	Fs2BLK	0x2000
#define	Fs4BLK	0x4000

#ifndef	FsTYPE
#define	FsTYPE	3
#endif

#if FsTYPE==1
	/* Original 512 byte file system */
#define	BSIZE	512		/* size of file system block (bytes) */
#define	SBUFSIZE	BSIZE	/* system buffer size */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	0777		/* BSIZE-1 */
#define	INOPB	8		/* inodes per block */
#define	INOSHIFT	3	/* LOG2(INOPB) if exact */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	FsBSIZE(dev)	BSIZE
#define	FsBSHIFT(dev)	BSHIFT
#define	FsNINDIR(dev)	NINDIR
#define	FsBMASK(dev)	BMASK
#define	FsINOPB(dev)	INOPB
#define	FsLTOP(dev, b)	b
#define	FsPTOL(dev, b)	b
#define	FsNMASK(dev)	NMASK
#define	FsNSHIFT(dev)	NSHIFT
#define	FsITOD(dev, x)	itod(x)
#define	FsITOO(dev, x)	itoo(x)
#endif

#if FsTYPE==2
	/* New 1024 byte file system */
#define	BSIZE	1024		/* size of file system block (bytes) */
#define	SBUFSIZE	BSIZE	/* system buffer size */
#define	BSHIFT	10		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	01777		/* BSIZE-1 */
#define	INOPB	16		/* inodes per block */
#define	INOSHIFT	4	/* LOG2(INOPB) if exact */
#define	NMASK	0377		/* NINDIR-1 */
#define	NSHIFT	8		/* LOG2(NINDIR) */
#define	FsBSIZE(dev)	BSIZE
#define	FsBSHIFT(dev)	BSHIFT
#define	FsNINDIR(dev)	NINDIR
#define	FsBMASK(dev)	BMASK
#define	FsINOPB(dev)	INOPB
#define	FsLTOP(dev, b)	(b<<1)
#define	FsPTOL(dev, b)	(b>>1)
#define	FsNMASK(dev)	NMASK
#define	FsNSHIFT(dev)	NSHIFT
#define	FsITOD(dev, x)	itod(x)
#define	FsITOO(dev, x)	itoo(x)
#endif

#if FsTYPE==3
	/* 512 and 1024 bytes per block system */
#define	BSIZE	512		/* size of file system block (bytes) */
#define	SBUFSIZE	(BSIZE*2)	/* system buffer size */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	0777		/* BSIZE-1 */
#define	INOPB	8		/* inodes per block */
#define	INOSHIFT	3	/* LOG2(INOPB) if exact */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	FsLRG(dev)	(dev&Fs2BLK)
#define	FsBSIZE(dev)	(FsLRG(dev) ? (BSIZE*2) : BSIZE)
#define	FsBSHIFT(dev)	(FsLRG(dev) ? 10 : 9)
#define	FsNINDIR(dev)	(FsLRG(dev) ? 256 : 128)
#define	FsBMASK(dev)	(FsLRG(dev) ? 01777 : 0777)
#define	FsBOFF(dev, x)	(FsLRG(dev) ? ((x)&01777) : ((x)&0777))
#define	FsBNO(dev, x)	(FsLRG(dev) ? ((x)>>10) : ((x)>>9))
#define	FsINOPB(dev)	(FsLRG(dev) ? 16 : 8)
#define	FsLTOP(dev, b)	(FsLRG(dev) ? b<<1 : b)
#define	FsPTOL(dev, b)	(FsLRG(dev) ? b>>1 : b)
#define	FsNMASK(dev)	(FsLRG(dev) ? 0377 : 0177)
#define	FsNSHIFT(dev)	(FsLRG(dev) ? 8 : 7)
#define	FsITOD(dev, x)	(daddr_t)(FsLRG(dev) ? \
	((unsigned)x+(2*16-1))>>4 : ((unsigned)x+(2*8-1))>>3)
#define	FsITOO(dev, x)	(daddr_t)(FsLRG(dev) ? \
	((unsigned)x+(2*16-1))&017 : ((unsigned)x+(2*8-1))&07)
#define	FsINOS(dev, x)	(FsLRG(dev) ? \
	((x&~017)+1) : ((x&~07)+1))
#endif

#if FsTYPE==4
	/* 512, 1024, and 2048 bytes per block system */
#define	BSIZE	512		/* size of file system block (bytes) */
#define	SBUFSIZE	(BSIZE*4)	/* system buffer size */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	0777		/* BSIZE-1 */
#define	INOPB	8		/* inodes per block */
#define	INOSHIFT	3	/* LOG2(INOPB) if exact */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	FsLRG(dev)	(dev&Fs2BLK)
#define	FsXLRG(dev)	(dev&Fs4BLK)
#define	FsBSIZE(dev)	(FsXLRG(dev) ? (BSIZE*4) : \
	(FsLRG(dev) ? (BSIZE*2) : BSIZE))
#define	FsBSHIFT(dev)	(FsXLRG(dev) ? 11 : ((FsLRG(dev) ? 10 : 9)))
#define	FsNINDIR(dev)	(FsXLRG(dev) ? 512 : ((FsLRG(dev) ? 256 : 128)))
#define	FsBMASK(dev)	(FsXLRG(dev) ? 03777 : ((FsLRG(dev) ? 01777 : 0777)))
#define	FsBOFF(dev, x)	(FsXLRG(dev) ? ((x)&03777) : \
	(FsLRG(dev) ? ((x)&01777) : ((x)&0777)))
#define	FsBNO(dev, x)	(FsXLRG(dev) ? ((x)>>11) : \
	(FsLRG(dev) ? ((x)>>10) : ((x)>>9)))
#define	FsINOPB(dev)	(FsXLRG(dev) ? 32 : (FsLRG(dev) ? 16 : 8))
#define	FsLTOP(dev, b)	(FsXLRG(dev) ? b<<2 : (FsLRG(dev) ? b<<1 : b))
#define	FsPTOL(dev, b)	(FsXLRG(dev) ? b>>2 : (FsLRG(dev) ? b>>1 : b))
#define	FsNMASK(dev)	(FsXLRG(dev) ? 0777 : (FsLRG(dev) ? 0377 : 0177))
#define	FsNSHIFT(dev)	(FsXLRG(dev) ? 9 : (FsLRG(dev) ? 8 : 7))
#define	FsITOD(dev, x)	(daddr_t)(FsXLRG(dev) ? ((unsigned)x+(2*32-1))>>5 : \
	(FsLRG(dev) ? ((unsigned)x+(2*16-1))>>4 : ((unsigned)x+(2*8-1))>>3))
#define	FsITOO(dev, x)	(daddr_t)(FsXLRG(dev) ? ((unsigned)x+(2*32-1))&037 : \
	(FsLRG(dev) ? ((unsigned)x+(2*16-1))&017 : ((unsigned)x+(2*8-1))&07))
#define	FsINOS(dev, x)	(FsXLRG(dev) ? ((x&~037)+1) : \
	(FsLRG(dev) ? ((x&~017)+1) : ((x&~07)+1)))
#endif

#define	NICFREE	50		/* number of superblock free blocks */
#define	NCPS	1		/* Number of clicks per segment */
#define	NBPC	512		/* Number of bytes per click */
#define	NCPD	1		/* Number of clicks per disk block */
#define	BPCSHIFT	9	/* LOG2(NBPC) if exact */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<24)	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	ROOTINO	((ino_t)2)	/* i number of all roots */
#define	SUPERB	((daddr_t)1)	/* physical block number of the super block */
#define	SUPERBOFF	512	/* byte offset of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */
#define	NPHYS	4		/* max simultaneous phys() calls */
#define	SHMSEG	6		/* max simultaneous shmat() calls */
#define	NSCATSWAP 4		/* number of scatter swapped segments */
#define	NSCATLOAD 2048		/* number of scatter load segments */
#define STKMAGIC 0xA55A5AA5	/* stack verification size magic number */

#define	CPUPRI(ps)	((ps) & PS_IPL)		/* Mask for CPU priority bits */
#define	BASEPRI(ps)	(CPUPRI(ps) != 0)	/* CPU base priority */
#define	USERMODE(ps)	(((ps) & PS_SUP)==0)	/* User mode definition */

#define	lobyte(X)	(((unsigned char *)&(X))[1])
#define	hibyte(X)	(((unsigned char *)&(X))[0])
#define	loword(X)	(((ushort *)&(X))[1])
#define	hiword(X)	(((ushort *)&(X))[0])

#define MIN(a,b)	(((a) < (b))?(a):(b))
#define MAX(a,b)	(((a) > (b))?(a):(b))
#define bcopy(a,b,c)	blt(b, a, c)

#define SPL0()		asm("	movw	#0x2000,sr");
#define SPL1()		asm("	movw	#0x2100,sr");
#define SPL2()		asm("	movw	#0x2200,sr");
#define SPL3()		asm("	movw	#0x2300,sr");
#define SPL4()		asm("	movw	#0x2400,sr");
#define SPL5()		asm("	movw	#0x2500,sr");
#define SPL6()		asm("	movw	#0x2600,sr");
#define SPL7()		asm("	movw	#0x2700,sr");
