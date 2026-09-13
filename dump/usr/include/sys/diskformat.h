/*
 * Copyright 1982 UniSoft Corporation
 *
 * diskformat.h - general purpose diskformat control structure
 */

struct diskformat {
	long	d_secsize;	/* sector size (128, 256, 512, 1024, etc.) */
	long	d_dens;		/* density (1=single, 2=double, etc.) */
	long	d_fsec;		/* first sector */
	long	d_lsec;		/* last sector */
	long	d_fhead;	/* first head */
	long	d_lhead;	/* last head */
	long	d_fcyl;		/* first cylinder */
	long	d_lcyl;		/* last cylinder */
	long	d_ileave;	/* interleave factor */
};

#ifndef DISKDEFAULT
#define DISKDEFAULT	-1		/* parameter is defaulted */
#endif
