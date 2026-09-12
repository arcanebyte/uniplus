/*
 * Copyright 1982 UniSoft Corporation
 */

/* structure for tuning floppy disk drivers */
struct disktune {
	int	dt_srt;		/* step rate time */
	int	dt_hlt;		/* head load time */
	int	dt_hut;		/* head unload time */
};

#ifndef DISKDEFAULT
#define DISKDEFAULT	-1	/* parameter is defaulted */
#endif
