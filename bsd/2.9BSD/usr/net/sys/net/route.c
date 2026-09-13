/*	route.c	4.11	82/06/20	*/

#include "param.h"
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "../net/in.h"
#include "../net/in_systm.h"
#include "../net/if.h"
#include "../net/af.h"
#include "../net/route.h"
#include <errno.h>

int	rttrash;		/* routes not in table but not freed */
/*
 * Packet routing routines.
 */
rtalloc(ro)
	register struct route *ro;
{
	register struct rtentry *rt, *rtmin;
	register u_int hash, (*match)();
	struct afhash h;
	struct sockaddr *dst = &ro->ro_dst;
 	struct sockaddr zeros;		/* "Wildcard" destination BRL*/
	int af = dst->sa_family;

	if (ro->ro_rt && ro->ro_rt->rt_ifp)			/* XXX */
		return;
	if (af >= AF_MAX)
		return;
top:								/* BRL */
	(*afswitch[af].af_hash)(dst, &h);
	rtmin = 0, hash = h.afh_hosthash;
	for (rt = rthost[hash % RTHASHSIZ]; rt; rt = rt->rt_next) {
		if (rt->rt_hash != hash)
			continue;
		if ((rt->rt_flags & RTF_UP) == 0 ||
		    (rt->rt_ifp->if_flags & IFF_UP) == 0)
			continue;
		if (bcmp((caddr_t)&rt->rt_dst, (caddr_t)dst, sizeof (*dst)))
			continue;
		if (rtmin == 0 || rt->rt_use < rtmin->rt_use)
			rtmin = rt;
	}
	if (rtmin) 
		goto found;

	hash = h.afh_nethash;
	match = afswitch[af].af_netmatch;
	for (rt = rtnet[hash % RTHASHSIZ]; rt; rt = rt->rt_next) {
		if (rt->rt_hash != hash)
			continue;
		if ((rt->rt_flags & RTF_UP) == 0 ||
		    (rt->rt_ifp->if_flags & IFF_UP) == 0)
			continue;
		if (rt->rt_dst.sa_family != af || !(*match)(&rt->rt_dst, dst))
			continue;
		if (rtmin == 0 || rt->rt_use < rtmin->rt_use)
			rtmin = rt;
	}
found:
	ro->ro_rt = rtmin;
	if (rtmin)
		rtmin->rt_refcnt++;	/* Success! */
	else if (dst != &zeros) {
		/* Look again for "wildcard" route (net 0)  BRL */
		bzero ((caddr_t) &zeros, sizeof(zeros));
		zeros.sa_family = dst->sa_family;
		dst = &zeros;
		goto top;
	}
}

rtfree(rt)
	register struct rtentry *rt;
{
	if (rt == 0)
		panic("freeroute");
	rt->rt_refcnt--;
	if (rt->rt_refcnt == 0 && (rt->rt_flags&RTF_UP) == 0) {
		rttrash--;
		MSFREE(rt);
	}
}

/*
 * Carry out a request to change the routing table.  Called by
 * interfaces at boot time to make their ``local routes'' known
 * and for ioctl's.
 */
rtrequest(req, entry)
	int req;
	register struct rtentry *entry;
{
	register struct rtentry *rt,**rtprev;
	struct afhash h;
	u_int af, s, error = 0, hash, (*match)();
	struct ifnet *ifp;

	af = entry->rt_dst.sa_family;
	if (af >= AF_MAX)
		return (EAFNOSUPPORT);
	(*afswitch[af].af_hash)(&entry->rt_dst, &h);
	if (entry->rt_flags & RTF_HOST) {
		hash = h.afh_hosthash;
		rtprev = &rthost[hash % RTHASHSIZ];
	} else {
		hash = h.afh_nethash;
		rtprev = &rtnet[hash % RTHASHSIZ];
	}
	match = afswitch[af].af_netmatch;
	s = splimp();
	for (; rt = *rtprev; rtprev = &rt->rt_next) {
		if (rt->rt_hash != hash)
			continue;
		if (entry->rt_flags & RTF_HOST) {
#define	equal(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), sizeof (struct sockaddr)) == 0)
			if (!equal(&rt->rt_dst, &entry->rt_dst))
				continue;
		} else {
			if (rt->rt_dst.sa_family != entry->rt_dst.sa_family ||
			    (*match)(&rt->rt_dst, &entry->rt_dst) == 0)
				continue;
		}
		if (equal(&rt->rt_gateway, &entry->rt_gateway))
			break;
	}
	switch (req) {

	case SIOCDELRT:
		if (rt == 0) {
			error = ESRCH;
			goto bad;
		}
		*rtprev = rt->rt_next;
		if (rt->rt_refcnt > 0) {
			rt->rt_flags &= ~RTF_UP;
			rttrash++;
			rt->rt_next = 0;
		} else
			MSFREE(rt);
		break;

	case SIOCADDRT:
		if (rt) {
			error = EEXIST;
			goto bad;
		}
		ifp = if_ifwithaddr(&entry->rt_gateway);
		if (ifp == 0) {
			ifp = if_ifwithnet(&entry->rt_gateway);
			if (ifp == 0) {
				error = ENETUNREACH;
				goto bad;
			}
		}
		MSGET(rt, struct rtentry, 1);
		if (rt == 0) {
			error = ENOBUFS;
			goto bad;
		}
		*rtprev = rt;
		rt->rt_hash = hash;
		rt->rt_dst = entry->rt_dst;
		rt->rt_gateway = entry->rt_gateway;
		rt->rt_flags =
		    RTF_UP | (entry->rt_flags & (RTF_HOST|RTF_GATEWAY));
		rt->rt_ifp = ifp;
		break;
	}
bad:
	splx(s);
	return (error);
}

/*
 * Set up a routing table entry, normally
 * for an interface.
 */
rtinit(dst, gateway, flags)
	struct sockaddr *dst, *gateway;
	int flags;
{
	struct rtentry route;
	struct route ro;

	bzero((caddr_t)&route, sizeof (route));
	route.rt_dst = *dst;
	route.rt_gateway = *gateway;
	route.rt_flags = flags;
	(void) rtrequest(SIOCADDRT, &route);
}
