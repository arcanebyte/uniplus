/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - MMI Library Error Codes
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 * CREATED   : 28th February 1986                                             *
 *                                                                            *
 * PURPOSE   :                                                                *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) mmierror.h 1.1@(#)" */

/* the error number is of type int with the following bit fields:	   */
/*	0..7	- errno or the library internal reason for failure	   */
/*	8..15	- type, one of WLIBSYSERR, WLIBERR, RESERR, RESSYSERR etc. */
/*	16..31	- liberrno, the library error number for the given type    */

/* consruction of error code returned in Xerrno				   */

/* e.g. werrno = ERRORCODE(errno,       WLIBSYSERR, WEM_OPFAIL)		   */
/*      werrno = ERRORCODE(WER_CORRUPT, WLIBERR,    WEM_OPFAIL)		   */

#define	ERRORCODE(e, t, l)	(((l)<<16)&0xffff0000|((t)<<8)&0xff00|(e)&0xff)

/* decoding of error code returned in Xerrno				   */

#define	MMIERRNO(e)		((e)&0xff)
#define	MMIERRTYPE(t)		(((t)>>8)&0xff)
#define	MMIERROR(l)		(((l)>>16)&0xffff)

/* type field values in Xerrno - leave 0 for the unknown type of error */

#define WLIBSYSERR	1	/* system call fail - reason from errno */
#define	WLIBERR		2	/* internal failure - reason from library */
#define RESSYSERR	3	/* system call fail - reason from errno */
#define RESERR		4	/* internal failure - reason from library */

/* files containing error messages */

/* TORCH extension to standard errno messages */

#define	MMIERRFILE		"/usr/lib/xxxerrors/mmierrors"

/* library - specific error messages */

#define	WLIBERRFILE		"/usr/lib/xxxerrors/wliberrors"
#define	WLIBREASONFILE		"/usr/lib/xxxerrors/wlibreasons"
#define	RESERRFILE		"/usr/lib/xxxerrors/reserrors"
#define	RESREASONFILE		"/usr/lib/xxxerrors/resreasons"

