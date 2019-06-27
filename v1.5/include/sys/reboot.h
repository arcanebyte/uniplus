/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* This is the command value for the console ioctl which
 * jumps to the start of unix.  It is only compiled into
 * the installation unix sunix.
 */
#define RESTART	('R'<<8)	/* restart unix */
