/*	@(#)fpioctl.h	UniPlus VVV.2.1.1	*/
/*
 * MC68881 Floating-Point Coprocessor IOCTL
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#define FPIOC	('F'<<8)
#define FPIOCEXC	(FPIOC|0)	/* reason for last SIGFPE exception */
