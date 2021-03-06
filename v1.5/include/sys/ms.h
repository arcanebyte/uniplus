/*
 * (C) 1983, 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Driver definitions for the Lisa mouse.
 */

	/* Types of mouse interrupts */
#define M_FLUSH 0		/* timeout, mouse records out of date */
#define M_PLUG	1		/* mouse unplugged or plugged in */
#define M_BUT	2		/* button changed */
#define M_CTL	3		/* apple key changed */
#define M_SFT	4		/* shift key changed */
#define M_VRT	5		/* verticle retrace interrupt (unused) */
#define M_MOVE	6		/* mouse moved */
