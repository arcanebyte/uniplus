/*   Copyright (c) 1984 AT&T	and UniSoft Systems */
/*     All Rights Reserved  	*/

/*   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T and UniSoft Systems */
/*   The copyright notice above does not evidence any   	*/
/*   actual or intended publication of such source code.	*/

#ifndef _IO
#include <sys/ioctl.h>
#endif

#ifndef FMNAMESZ
#define FMNAMESZ	8
#endif FMNAMESZ

/*
 * Read options
 */

#define RNORM 	0			/* read msg norm */
#define RMSGD	1			/* read msg discard */
#define RMSGN	2			/* read msg no discard */

/*
 * Flush options
 */

#define FLUSHR 1			/* flush read queue */
#define FLUSHW 2			/* flush write queue */
#define FLUSHRW 3			/* flush both queues */


/*
 *  Stream Ioctl defines
 */
#define I_NREAD		_IOR(S, 01, int)
#define I_PUSH		_IOW(S, 02, char[FMNAMESZ+1])
#define I_POP		_IO(S, 03)
#define I_LOOK		_IOW(S, 04, char[FMNAMESZ+1])
#define I_FLUSH		_IO(S, 05)
#define I_SRDOPT	_IO(S, 06)
#define I_GRDOPT	_IOR(S, 07, int)
#define I_STR		_IOWR(S, 010, struct strioctl)

#define I_FIND		_IOW(S, 013, char[FMNAMESZ+1])

#define I_MNAME		_IO(S, 0100)


/*
 * User level ioctl format for ioctl that go downstream I_STR 
 */
struct strioctl {
	int 	ic_cmd;			/* command */
	int	ic_timout;		/* timeout value */
	int	ic_len;			/* length of data */
	char	*ic_dp;			/* pointer to data */
};



