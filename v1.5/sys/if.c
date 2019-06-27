/*	if.c	4.25	83/02/10	*/
#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/socket.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/if.h"
#include "net/af.h"

int	ifqmaxlen = IFQ_MAXLEN;

/*
 * Network interface utility routines.
 *
 * Routines with if_ifwith* names take sockaddr *'s as
 * parameters.  Other routines take value parameters,
 * e.g. if_ifwithnet takes the network number.
 */

ifinit()
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_init) {
			(*ifp->if_init)(ifp->if_unit);
			if (ifp->if_snd.ifq_maxlen == 0)
				ifp->if_snd.ifq_maxlen = ifqmaxlen;
		}
	if_slowtimo();
}

#if vax
/*
 * Call each interface on a Unibus reset.
 */
ifubareset(uban)
	int uban;
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_reset)
			(*ifp->if_reset)(uban);
}
#endif

/*
 * Attach an interface to the
 * list of "active" interfaces.
 */
if_attach(ifp)
	struct ifnet *ifp;
{
	register struct ifnet **p = &ifnet;

	while (*p)
		p = &((*p)->if_next);
	*p = ifp;
}

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifnet *
if_ifwithaddr(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;

#define	equal(a1, a2) \
	(bcmp((caddr_t)((a1)->sa_data), (caddr_t)((a2)->sa_data), 14) == 0)
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_addr.sa_family != addr->sa_family)
			continue;
		if (equal(&ifp->if_addr, addr))
			break;
		if ((ifp->if_flags & IFF_BROADCAST) &&
		    equal(&ifp->if_broadaddr, addr))
			break;
	}
	return (ifp);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is first found.
 */
struct ifnet *
if_ifwithnet(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register u_int af = addr->sa_family;
	register int (*netmatch)();

	if (af >= AF_MAX)
		return (0);
	netmatch = afswitch[af].af_netmatch;
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (af != ifp->if_addr.sa_family)
			continue;
		if ((*netmatch)(addr, &ifp->if_addr))
			break;
	}
	return (ifp);
}

/*
 * As above, but parameter is network number.
 */
struct ifnet *
if_ifonnetof(net)
	register int net;
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_net == net)
			break;
	return (ifp);
}

/*
 * Find an interface using a specific address family
 */
struct ifnet *
if_ifwithaf(af)
	register int af;
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_addr.sa_family == af)
			break;
	return (ifp);
}

#ifdef notdef
/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
if_down(ifp)
	register struct ifnet *ifp;
{

	ifp->if_flags &= ~IFF_UP;
	pfctlinput(PRC_IFDOWN, (caddr_t)&ifp->if_addr);
}
#endif

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
if_slowtimo()
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_timer == 0 || --ifp->if_timer)
			continue;
		if (ifp->if_watchdog)
			(*ifp->if_watchdog)(ifp->if_unit);
	}
	/* billn -- clock calls us in old...
	timeout(if_slowtimo, (caddr_t)0, hz / IFNET_SLOWHZ);
	*/
}
