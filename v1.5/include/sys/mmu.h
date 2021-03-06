/*
 * Copyright 1982 UniSoft Corporation
 *
 * Use of this code is subject to your disclosure agreement with AT&T,
 * Western Electric, and UniSoft Corporation
 */

#define NUMCONTX	4			/* number of user contexts */
#define MEMEND		(char **)(0x2A8)	/* Addr of logical end of mem */
#define MEMBASE		(long *)(0x2A4)		/* Addr of phys beg of mem */

/*
 * Constants and definitions
 */
#define SPECIO	0xFE0000	/* Special I/O address */
#define STDIO	0xFC0000	/* Standard I/O address */

	/* Special I/O Space Locations */
#define STATUSA	(char *)(STDIO+0xF800)		/* Status Register address */

#define STATUS	*(short *)(STATUSA)			/* Status Register */
#define MEMERR	*(char *)(STDIO+0xF000)		/* Memory Error Address Latch */
#define SETUP_0	*(char *)(STDIO+0xE012)		/* Turn setup bit off */
#define SETUP_1	*(char *)(STDIO+0xE010)		/* Turn setup bit on */
#define SEG1_0	*(char *)(STDIO+0xE008)		/* Turn SEG1 bit off */
#define SEG1_1	*(char *)(STDIO+0xE00A)		/* Turn SEG1 bit on */
#define SEG2_0	*(char *)(STDIO+0xE00C)		/* Turn SEG2 bit off */
#define SEG2_1	*(char *)(STDIO+0xE00E)		/* Turn SEG2 bit on */
#define VRON	*(char *)(STDIO+0xE01A)		/* Turn Video retrace int on */
#define VROFF	*(char *)(STDIO+0xE018)		/* Turn Video retrace int off */
#define VIDADDR	*(char *)(STDIO+0xE800)		/* Video address latch */
#define ADDRMASK 0xFFFFFF			/* relevant address bits */

/*
 * Access Control Bits
 */
#define ASROS	0x400	/* Address space is read only stack */
#define ASRO	0x500	/* Address space is read only */
#define ASRWS	0x600	/* Address space is read write stack */
#define ASRW	0x700	/* Address space is read write */
#define ASIO	0x800	/* Address space is I/O */
#define ASINVAL	0xC00	/* Address space is invalid */
#define ASSPIO	0xF00	/* Address space is special I/O */
#define PROTMASK 0xF00	/* Address space protection mask */

#define UDOTBASE	0xFA0000	/* Logical base of udot area */

	/* Special I/O Bits */
#define BSEGORIG	0x8		/* Segment Origin Register */

/*
 * Definitions for memory management.
 */
#define NPAGEPERSEG	256		/* number mappable pages per segment */
#define SEGMASK		0xFE0000	/* mask for segment number as address */
#define OFFMASK		0x1FFFF		/* Offset mask for virtual address */
#define PAGEMASK	0x1FE00		/* Page mask for virtual address */
#define PAGESIZE	512		/* bytes per page */
#define PAGESHIFT	9		/* page shift for virtual addr  */
#define DISPMASK	0x001FF		/* Disp mask for virtual address */
#define SEGBASE		0xFFF		/* mask for segment base */
#define SEGNMASK	0x3F		/* mask for segment number as number */
#define SEGSHIFT	17		/* shift for segment number */
#define VIRTSHIFT	9		/* seg shift for virtual address */
#define ACCLIM		0x008000	/* Access Limit Register */
#define ACCSEG		0x008008	/* Access Segment Register */

/* Status register bits */
#define S_SMEMERR	0x1	/* Soft Memory Error */
#define S_HMEMERR	0x2	/* Hard Memory Error */
#define S_VR		0x4	/* Vertical Retrace */

#define vtoseg(x)	((int)(x) & SEGMASK)

	/* size of the current process */
#define procsize(p)	((p)->p_size)

	/* macros to eliminate sep I/D */
#define fuiword(x)	fuword(x)
#define suiword(x,y)	suword(x,y)
