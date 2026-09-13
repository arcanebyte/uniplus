/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Standard User Include File                *
 *                                                                            *
 * COPYRIGHT : (c) 1986 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) wstd.h 1.3@(#)" */

#include <stdio.h>
#include <mmi/mmi.h>
#include <mmi/icon.h>
#include <mmi/mouse.h>

/* Parameters for Bold(), Italic() etc.  */

#define	OFF     0	/* turn style off */
#define	ON      1	/* turn style on */
#define	TOGGLE  2	/* toggle */

extern char *WError();
extern FILE *POpen();

#ifndef	ERROR
#define	ERROR	(-1)	/* result denoting an error condition */
#endif	ERROR
#ifndef	SUCCESS
#define	SUCCESS	0	/* result denoting no error condition */
#endif	SUCCESS
