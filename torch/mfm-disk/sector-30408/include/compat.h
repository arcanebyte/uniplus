/*	@(#)compat.h	1.4 - 12/6/86 */

#define	COMPAT_BSDPROT		0x00000001	/* BSD style protection */
#define	COMPAT_BSDNBIO		0x00000002	/* Non blocking I/O */
#define	COMPAT_BSDSIGNALS	0x00000004	/* use reliable signals */
#define	COMPAT_BSDTTY		0x00000008	/* Stop on TTY activity */
#define	COMPAT_SYSCALLS		0x00000010	/* Uninterruptable calls */
#define COMPAT_CLRPGROUP	0x00000020	/* Clear 4.2 proc group */
#define COMPAT_SVID		0x00000040	/* SVID compatible */
