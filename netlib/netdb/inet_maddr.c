/*	inet_maddr.c	4.2	82/10/07	*/
/* Lisa: 68000 byte order (network order is host order). */

#include <sys/types.h>
#include <net/in.h>
#include <netdb.h>

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
struct in_addr
inet_makeaddr(net, host)
	u_long net, host;
{
	struct in_addr a;

	if (net < 128)
		a.s_addr = (net << 24) | host;
	else if (net < 65536)
		a.s_addr = (net << 16) | host;
	else
		a.s_addr = (net << 8) | host;
	return (a);
}
