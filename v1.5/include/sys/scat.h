#ifndef NONSCATLOAD

/*
 * Scatter memory storage structure
 */

struct scatter {
	short	sc_index;
};

extern int nscatfree;			/* Number of free scatter elements */
extern int scsortmap[];			/* Scatter map sort array */
extern struct scatter scatmap[];	/* Scatter map */
extern struct scatter scatfreelist;	/* Head of freelist pool */

#define SCATEND		(-1)		/* Scatter map end index */

#endif
