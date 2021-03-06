/*
 * Sunol definitions
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 *	This block is returned when a "get drive parameters" command is
 *	issued.
 */
struct sdparam {
	char fwmsg[31];		/* firmware message */
	unsigned char revision;	/* revision number */
	unsigned char romv;	/* ROM version */
		/* next four bytes are for entire disk */
	unsigned char spt;	/* sectors per track */
	unsigned char tpc;	/* tracks per cylinder */
	unsigned char cpd_lsb;	/* cylinders per drive (least sig byte) */
	unsigned char cpd_msb;	/* cylinders per drive (most sig byte) */
		/* next three bytes are for specific virtual drive */
	unsigned char nblk_lsb;	/* capacity in 512-byte blks (least sig byte) */
	unsigned char nblk_nsb;	/* capacity in 512-byte blks (next sig byte) */
	unsigned char nblk_msb;	/* capacity in 512-byte blks (most sig byte) */
	char fill[88];		/* fill to 128 bytes */
};

/*
 *	This block is used to issue a copy command which copies from one
 *	virtual drive to another.  The tape unit is virtual drive 9, and
 *	the disk can be divided into virtual drives 1-7.  If the command
 *	returns a non-zero status then the amount that was not copied is
 *	specified in 10 bytes corresponding to the bytes from srcvd to
 *	mcount.  These blocks can be transferred using normal reads and
 *	writes (in start/stop mode).
 */
struct copyparam {
	unsigned char command;		/* copy command = 0xE3 */
	unsigned char srcvd;		/* source virtual drive */
	unsigned char lsrcstart;	/* source starting sector */
	unsigned char msrcstart;	/* source starting sector */
	unsigned char fill1;		/* upper byte of starting sector */
	unsigned char destvd;		/* destination virtual drive */
	unsigned char ldeststart;	/* destination starting sector */
	unsigned char mdeststart;	/* destination starting sector */
	unsigned char fill2;		/* upper byte of starting sector */
	unsigned char lcount;		/* sector count for copy */
	unsigned char mcount;		/* sector count for copy */
	unsigned char chksum;		/* all 12 bytes add to zero */
};

#define	TAPEVD	9
