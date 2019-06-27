/*	raw_cb.c	4.16	83/02/10	*/
#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "net/misc.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/mbuf.h"
#include "net/protosw.h"

#include "net/if.h"
#include "net/raw_cb.h"

/*
 * Routines to manage the raw protocol control blocks. 
 *
 * TODO:
 *	hash lookups by protocol family/protocol + address family
 *	take care of unique address problems per AF?
 *	redo address binding to allow wildcards
 */

/*
 * Allocate a control block and a nominal amount
 * of buffer space for the socket.
 */
raw_attach(so)
	register struct socket *so;
{
	struct mbuf *m;
	register struct rawcb *rp;

	/* billn, meld w/ old  note don't gtclr -- will it work?
	m = m_getclr(M_DONTWAIT, MT_PCB);
	*/
	m = m_get(M_DONTWAIT);
	if (m == 0)
		return (ENOBUFS);
	if (sbreserve(&so->so_snd, RAWSNDQ) == 0)
		goto bad;
	if (sbreserve(&so->so_rcv, RAWRCVQ) == 0)
		goto bad2;
	rp = mtod(m, struct rawcb *);
	rp->rcb_socket = so;
	insque(rp, &rawcb);
	so->so_pcb = (caddr_t)rp;
	rp->rcb_pcb = 0;
	return (0);
bad2:
	sbrelease(&so->so_snd);
bad:
	(void) m_free(m);
	return (ENOBUFS);
}

/*
 * Detach the raw connection block and discard
 * socket resources.
 */
raw_detach(rp)
	register struct rawcb *rp;
{
	struct socket *so = rp->rcb_socket;

	so->so_pcb = 0;
	sofree(so);
	remque(rp);
	m_freem(dtom(rp));
}

/*
 * Disconnect and possibly release resources.
 */
raw_disconnect(rp)
	struct rawcb *rp;
{

	rp->rcb_flags &= ~RAW_FADDR;
	/* billn -- meld with old...
	if (rp->rcb_socket->so_state & SS_NOFDREF)
	*/
	if (rp->rcb_socket->so_state & SS_USERGONE)
		raw_detach(rp);
}

#ifdef notdef
raw_bind(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
	struct sockaddr *addr = mtod(nam, struct sockaddr *);
	register struct rawcb *rp;

	if (ifnet == 0)
		return (EADDRNOTAVAIL);
{
/*
#include "../h/domain.h"
*/
#include "net/in.h"
#include "net/in_systm.h"
/* BEGIN DUBIOUS */
	/*
	 * Should we verify address not already in use?
	 * Some say yes, others no.
	 */
	switch (addr->sa_family) {

	case AF_IMPLINK:
	case AF_INET:
		if (((struct sockaddr_in *)addr)->sin_addr.s_addr &&
		    if_ifwithaddr(addr) == 0)
			return (EADDRNOTAVAIL);
		break;

#ifdef PUP
	/*
	 * Curious, we convert PUP address format to internet
	 * to allow us to verify we're asking for an Ethernet
	 * interface.  This is wrong, but things are heavily
	 * oriented towards the internet addressing scheme, and
	 * converting internet to PUP would be very expensive.
	 */
	case AF_PUP: {
#include "../netpup/pup.h"
		struct sockaddr_pup *spup = (struct sockaddr_pup *)addr;
		struct sockaddr_in inpup;

		bzero((caddr_t)&inpup, (unsigned)sizeof(inpup));
		inpup.sin_family = AF_INET;
		inpup.sin_addr.s_net = spup->sp_net;
		inpup.sin_addr.s_impno = spup->sp_host;
		if (inpup.sin_addr.s_addr &&
		    if_ifwithaddr((struct sockaddr *)&inpup) == 0)
			return (EADDRNOTAVAIL);
		break;
	}
#endif

	default:
		return (EAFNOSUPPORT);
	}
}
/* END DUBIOUS */
	rp = sotorawcb(so);
	bcopy((caddr_t)addr, (caddr_t)&rp->rcb_laddr, sizeof (*addr));
	rp->rcb_flags |= RAW_LADDR;
	return (0);
}
#endif

/*
 * Associate a peer's address with a
 * raw connection block.
 */
raw_connaddr(rp, nam)
	struct rawcb *rp;
	struct mbuf *nam;
{
	struct sockaddr *addr = mtod(nam, struct sockaddr *);

	bcopy((caddr_t)addr, (caddr_t)&rp->rcb_faddr, sizeof(*addr));
	rp->rcb_flags |= RAW_FADDR;
}
