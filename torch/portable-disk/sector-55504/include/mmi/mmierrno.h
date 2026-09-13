/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - MMI Kernel Error Numbers                                 *
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 * CREATED   : 28th February 1986                                             *
 *                                                                            *
 * PURPOSE   : Provides a set of error codes for errno returned from Eioctl() *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) mmierrno.h 1.1@(#)" */

#define	MMIERRBASE	116		/* so last is 127 to fit in char */

/* MMIERRBASE+0 unused to denote unknown error in mmierrors file */

#define	EMMINOWIN	(MMIERRBASE+1)	/* window structure not open */
#define	EMMIINVHAN	(MMIERRBASE+2)	/* invalid handle number */
#define	EMMIINVTYPE	(MMIERRBASE+3)	/* invalid handle type */
#define	EMMILOCKED	(MMIERRBASE+4)	/* handle locked */
#define	EMMINOTLOCKED	(MMIERRBASE+5)	/* handle locked */
#define	EMMIINUSE	(MMIERRBASE+6)	/* handle in use */
#define	EMMINOSIG	(MMIERRBASE+7)	/* no current event signalled */
#define	EMMINOPSIG	(MMIERRBASE+8)	/* not handling signals */
#define	EMMITOMANY	(MMIERRBASE+9)	/* to many menus or items */
#define	EMMIINITFAIL	(MMIERRBASE+10)	/* failed to initialise window */
#define	EMMINOTIMP	(MMIERRBASE+11)	/* not implemented */

#define	SETERROR(n)	u.u_error = (n)	/* set errno on exit */



