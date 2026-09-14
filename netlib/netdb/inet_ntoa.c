/*
 * inet_ntoa.c -- as in 4.2BSD; the 4.1c library had no inet_ntoa().
 */

#include <sys/types.h>
#include <net/in.h>
#include <netdb.h>

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
char *
inet_ntoa(in)
	struct in_addr in;
{
	static char b[18];
	register u_long i = in.s_addr;

	sprintf(b, "%d.%d.%d.%d", (int)((i >> 24) & 0xff), (int)((i >> 16) & 0xff),
	    (int)((i >> 8) & 0xff), (int)(i & 0xff));
	return (b);
}
