/*	inet_netof.c	4.2	82/10/07	*/
/* Lisa: 68000 byte order, with the class masks from net/in.h. */

#include <sys/types.h>
#include <net/in.h>
#include <netdb.h>

/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
u_long
inet_netof(in)
	struct in_addr in;
{
	register u_long i = in.s_addr;

	if ((i & IN_CLASSA) == 0)
		return ((i & IN_CLASSA_NET) >> 24);
	if ((i & IN_CLASSB) == 0)
		return ((i & IN_CLASSB_NET) >> 16);
	return ((i & IN_CLASSC_NET) >> 8);
}
