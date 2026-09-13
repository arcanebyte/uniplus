/*	inet_maddr.c	4.2	82/10/07	*/

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
	u_long addr;

#if	!pdp11
	if (net < 128)
		addr = (net << 24) | host;
	else if (net < 65536)
		addr = (net << 16) | host;
	else
		addr = (net << 8) | host;
	addr = htonl(addr);
#else
	addr = (long)((net >> 16) & 0xffff) | (long)(net << 16);
	addr = htonl(addr);
	addr |= htonl(host);
#endif
	return (*(struct in_addr *)&addr);
}
