/*	inet_lnaof.c	4.2	82/10/07	*/
/* Lisa: 68000 byte order, with the class masks from net/in.h. */

#include <sys/types.h>
#include <net/in.h>
#include <netdb.h>

/*
 * Return the local network address portion of an
 * internet address; handles class a/b/c network
 * number formats.
 */
u_long
inet_lnaof(in)
	struct in_addr in;
{
	register u_long i = in.s_addr;

	if ((i & IN_CLASSA) == 0)
		return (i & IN_CLASSA_LNA);
	if ((i & IN_CLASSB) == 0)
		return (i & IN_CLASSB_LNA);
	return (i & IN_CLASSC_LNA);
}
