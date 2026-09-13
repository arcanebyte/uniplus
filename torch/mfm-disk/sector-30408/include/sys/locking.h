/*
 * File locking
 */

struct locklist {
	/* NOTE link must be first in struct */
	struct	locklist *ll_link;	/* link to next lock region */
	int	ll_flags;		/* misc flags ** sleeping */
	struct	proc *ll_proc;		/* process which owns region */
	off_t	ll_start;		/* starting offset */
	off_t	ll_end;			/* ending offset, zero is eof */
};

#ifndef AUTOCONFIG
extern struct locklist locklist[]; /* The lock table itself */
#else
extern struct locklist *locklist; /* The lock table itself */
#endif AUTOCONFIG
