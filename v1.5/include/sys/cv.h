/*
 * Corvus definitions
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* block number to virtual drive */
#define cvbtovd(b)	(((b)>>16 & 0xf)+1)
/* virtual drive to block number */
#define cvvdtob(vd)	((((vd)-1) & 0xf) << 16)

	/* cv_stat bits */
#define ST_BUSY		0x02
#define ST_HTOC		0x01		/* host to controller direction */

	/* normal mode commands */
#define N_READ		0x02
#define N_WRITE		0x02
#define N_PARAM		0x10
#define N_DIAGN		0x11
#define N_R128		0x12
#define N_R256		0x22
#define N_R512		0x32
#define N_W128		0x13
#define N_W256		0x23
#define N_W512		0x33
#define N_BOOT		0x14

	/* Sunol only - copy from one virtual drive to another
	 *		(such as tape to disk or disk to tape)
	 */
#define N_COPY		0xE3

	/* diagnostic mode commands */
#define D_RESET		0x00
#define D_FORMAT	0x01
#define D_VERIFY	0x07
#define D_RFIRMWARE	0x32
#define D_WFIRMWARE	0x33

	/* semaphore commands */
#define	S_INIT0		0x1a
#define S_INIT1		0x10
#define S_LOCK0		0x0b
#define S_LOCK1		0x01
#define S_ULOCK0	0x0b
#define S_ULOCK1	0x11
#define S_STAT0		0x1a
#define S_STAT1		0x41
#define S_STAT2		0x03

	/* normal mode return status */
#define NS_FATAL	0x80		/* mask */
#define NS_VERIFY	0x40		/* mask */
#define NS_RECOVER	0x20		/* mask */
#define NS_HFAULT	0x00
#define NS_STIME	0x01
#define NS_SFAULT	0x02
#define NS_SERROR	0x03
#define NS_HCRC		0x04
#define NS_ZFAULT	0x05
#define NS_ZTIME	0x06
#define NS_OFFLINE	0x07
#define NS_WFAULT	0x08
#define NS_		0x09
#define NS_RFAULT	0x0a
#define NS_DATACRC	0x0b
