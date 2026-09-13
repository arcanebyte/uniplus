/* _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.1 */
/* _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/14 */
/* sccsid = @(#)spray.h	UniPlus V.2.1.1 (Sun 2.1) */

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#define SPRAYPROG 100012
#define SPRAYPROC_SPRAY 1
#define SPRAYPROC_GET 2
#define SPRAYPROC_CLEAR 3
#define SPRAYVERS_ORIG 1
#define SPRAYVERS 1

#define SPRAYOVERHEAD 86	/* size of rpc packet when size=0 */
#define SPRAYMAX 8845		/* related to max udp packet of 9000 */

int xdr_sprayarr();
int xdr_spraycumul();

struct spraycumul {
	unsigned counter;
	struct timeval clock;
};

struct sprayarr {
	int *data;
	int lnth;
};
