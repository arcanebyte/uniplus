#define	NDZ	%NDZ%
#define	DZ_IOCTL
#define	DZ_PDMA				/* pseudo-DMA (more efficient) output */
/*
 *  If DZ_SOFTCAR is defined, carrier will be ignored for devices
 *  with the 0200 bit set in their minor device numbers.
 */
#define	DZ_SOFTCAR
/* #define DZ_SILO			/* use silo alarm */
#define	SILOSCANRATE	(hz / 10)	/* frequency of emptying the silo */
