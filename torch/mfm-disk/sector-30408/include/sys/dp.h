/*
 * Copyright 1982 UniSoft Corporation
 *
 * Use of this code is subject to your disclosure agreement with AT&T,
 * Western Electric, and UniSoft Corporation
 */

/*
 * Descriptor pointer allocation structure.
 * One allocated per used descriptor pointer.
 * NMMU descriptors exist with userdp being the
 * first one available for users.
 */
struct dp {
	struct	dp *dp_link;	/* forward link */
	char	dp_num;			/* descriptor number */
};

/*
 * link list of proc's with allocated descriptors
 */
struct dpproc {
	struct dpproc *dpprocf;	/* forward proc struct pointer */
	struct dpproc *dpprocb;	/* backward proc struct pointer */
};
