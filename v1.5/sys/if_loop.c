/*	if_loop.c	4.13	82/06/20	*/

/*
 * Loopback interface driver for protocol testing and timing.
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/socket.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/if.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "net/route.h"
#include "errno.h"

#define LONET   0x7f000000
#define	LOMTU	(1024+512)

struct	ifnet loif;
int	looutput();

loattach()
{
	register struct ifnet *ifp = &loif;
	register struct sockaddr_in *sin;

	ifp->if_name = "lo";
	ifp->if_mtu = LOMTU;
	ifp->if_net = htonl((u_long)LONET);
	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	/*
	sin->sin_addr = if_makeaddr((u_long)ifp->if_net, (u_long)0);
	*/
	sin->sin_addr = if_makeaddr((u_long)ifp->if_net, (u_long)1);
	ifp->if_flags = IFF_UP;
	ifp->if_output = looutput;
	if_attach(ifp);
	if_rtinit(ifp, RTF_UP);
}

looutput(ifp, m0, dst)
	register struct ifnet *ifp;
	register struct mbuf *m0;
	register struct sockaddr *dst;
{
	register int s = splimp();
	register struct ifqueue *ifq;

	ifp->if_opackets++;
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		ifq = &ipintrq;
		if (IF_QFULL(ifq)) {
			IF_DROP(ifq);
			m_freem(m0);
			splx(s);
			return (ENOBUFS);
		}
		IF_ENQUEUE(ifq, m0);
		schednetisr(NETISR_IP);
		break;
#endif
	default:
		splx(s);
		printf("lo%d: can't handle af%d\n", ifp->if_unit,
			dst->sa_family);
		m_freem(m0);
		return (EAFNOSUPPORT);
	}
	ifp->if_ipackets++;
	splx(s);
	return (0);
}
