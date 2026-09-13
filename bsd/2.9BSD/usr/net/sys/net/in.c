/*	in.c	4.3	82/06/20	*/

#include "param.h"
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include "../net/in.h"
#include "../net/in_systm.h"
#include "../net/if.h"
#include "../net/route.h"
#include "../net/af.h"

#ifdef INET
inet_hash(sin, hp)
	register struct sockaddr_in *sin;
	struct afhash *hp;
{
#if pdp11
	long l;
	l = in_netof(sin->sin_addr);
	hp->afh_nethash = ((int)l ^ (int)(l>>16)) & 077777;
	l = sin->sin_addr.s_addr;
	hp->afh_hosthash = ((int)l ^ (int)(l>>16)) & 077777;
#else
	hp->afh_nethash = in_netof(sin->sin_addr);
	hp->afh_hosthash = ntohl(sin->sin_addr.s_addr);
	if (hp->afh_hosthash < 0)
		hp->afh_hosthash = -hp->afh_hosthash;
#endif
}

inet_netmatch(sin1, sin2)
	struct sockaddr_in *sin1, *sin2;
{
	return (in_netof(sin1->sin_addr) == in_netof(sin2->sin_addr));
}

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
struct in_addr
if_mkaddr(net, host)
	u_long net,host;
{
	u_long addr;

	addr = htonl(host) | net;
	return (*(struct in_addr *)&addr);
}

/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
u_long
in_netof(in)
	struct in_addr in;
{

	return (IN_NETOF(in));
}

/*
 * Return the local network address portion of an
 * internet address; handles class a/b/c network
 * number formats.
 */
u_long
in_lnaof(in)
	struct in_addr in;
{

	return (IN_LNAOF(in));
}

/*
 * Initialize an interface's routing
 * table entry according to the network.
 * INTERNET SPECIFIC.
 */
if_rtinit(ifp, flags)
	register struct ifnet *ifp;
	int flags;
{
	struct sockaddr_in sin;

	if (ifp->if_flags & IFF_ROUTE)
		return;
	bzero((caddr_t)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = if_makeaddr(ifp->if_net, 0);
	rtinit(&sin, &ifp->if_addr, flags);
}
#endif
