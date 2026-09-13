/*	inet_netof.c	4.2	82/10/07	*/

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
#if	!pdp11
	register u_long net;

	if ((in.s_addr&IN_CLASSA) == 0)
		return (in.s_addr & IN_CLASSA_NET);
	if ((in.s_addr&IN_CLASSB) == 0)
		return ((int)htons((u_short)(in.s_addr & IN_CLASSB_NET)));
	net = htonl((u_long)(in.s_addr & IN_CLASSC_NET));
	net >>= 8;
	return (net);
#else
	u_long net;

	in.s_addr = htonl(in.s_addr);
	net = IN_NETOF(in);
	net = (long)(net << 16) | (long)((net >> 16) & 0xffff);
	in.s_addr = ntohl(in.s_addr);
	return(net);
#endif
}
