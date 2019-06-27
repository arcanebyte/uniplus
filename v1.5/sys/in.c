/*	in.c	4.3	82/06/20	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/if.h"
#include "net/route.h"
#include "net/af.h"

#ifdef INET
inet_hash(sin, hp)
	register struct sockaddr_in *sin;
	struct afhash *hp;
{
#ifdef ELEVEN
	long l;
	l = in_netof(sin->sin_addr);
	hp->afh_nethash = ((int)l ^ (int)(l>>16)) & 077777;
	l = sin->sin_addr.s_addr;
	hp->afh_hosthash = ((int)l ^ (int)(l>>16)) & 077777;
	if (hp->afh_hosthash < 0)
		hp->afh_hosthash = -hp->afh_hosthash;
#endif
	hp->afh_nethash = in_netof(sin->sin_addr);
	hp->afh_hosthash = ntohl(sin->sin_addr.s_addr);

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
/* billn -- for transition */
#undef if_makeaddr(x,y)
struct in_addr 
if_makeaddr(n, h)
	u_long n, h;
{
	return (if_mkaddr(n, h));
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
	sin.sin_addr = if_makeaddr((u_long)ifp->if_net, (u_long)0);
	rtinit((struct sockaddr *)&sin, &ifp->if_addr, flags);
}
#endif
