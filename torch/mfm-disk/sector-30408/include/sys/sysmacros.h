/* @(#)sysmacros.h	6.1 */
/*
 * Some macros for units conversion
 */

/* Core clicks to segments and vice versa */
#define ctos(x) (((int)(x)+((1<<(SEGSHIFT-PAGESHIFT))-1))>>(SEGSHIFT-PAGESHIFT))
#define ctost(x) (((int)(x))>>(SEGSHIFT-PAGESHIFT))
#define stoc(x) ((int)(x)<<(SEGSHIFT-PAGESHIFT))

/* Core clicks to disk blocks and vice versa */
#define	ctod(x)	((x)<<(PAGESHIFT-DEV_BSHIFT))
#define dtoc(x) ((x)>>(PAGESHIFT-DEV_BSHIFT))

/* Bytes to disk blocks nd vice versa */
#define	btod(x)	(((x)+(DEV_BSIZE-1))>>DEV_BSHIFT)
#define	dtob(x)	((x)<<DEV_BSHIFT)

/* inumber to disk address */
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)

/* inumber to disk offset */
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))

/* clicks to bytes */
#define	ctob(x)	((x)<<PAGESHIFT)

/* bytes to clicks */
#define btoc(x)		((((x)+((1<<PAGESHIFT)-1))>>PAGESHIFT)&ADDRMASK)
#define	btoct(x)	(((x)>>PAGESHIFT)&ADDRMASK)

/* major part of a device */
#define	major(x)	(int)((unsigned)(x)>>8)

/* minor part of a device */
#define	minor(x)	(int)((x)&0377)

/* make a device number */
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))

/* Calculate user priority */
#define calcppri(p)	((p->p_cpu) >> 1) +  p->p_nice + (PUSER - NZERO)
