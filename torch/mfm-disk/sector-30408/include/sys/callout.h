/*	callout.h	6.1	83/07/29	*/

/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on typewriters.
 */

struct	callout {
	int	c_time;		/* incremental time */
	caddr_t	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
	struct	callout *c_next;
};
#ifdef KERNEL
#ifndef AUTOCONFIG
extern struct callout	callout[];
#else
extern struct callout	*callout;
#endif AUTOCONFIG
struct	callout *callfree, calltodo;
#endif
