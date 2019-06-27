/*	raw_usrreq.c	4.25	83/02/10	*/
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
#include "net/mbuf.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/protosw.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/if.h"
#include "net/af.h"
#include "errno.h"
#include "net/raw_cb.h"

/*
 * Initialize raw connection block q.
 */
raw_init()
{

	rawcb.rcb_next = rawcb.rcb_prev = &rawcb;
	rawintrq.ifq_maxlen = IFQ_MAXLEN;
}

/*
 * Raw protocol interface.
 */
raw_input(m0, proto, src, dst)
	struct mbuf *m0;
	struct sockproto *proto;
	struct sockaddr *src, *dst;
{
	register struct mbuf *m;
	struct raw_header *rh;
	int s;

	/*
	 * Rip off an mbuf for a generic header.
	 */
	/* billn -- meld with old 
	m = m_get(M_DONTWAIT, MT_HEADER);
	*/
	m = m_get(M_DONTWAIT);
	if (m == 0) {
		m_freem(m0);
		return;
	}
	m->m_next = m0;
	m->m_len = sizeof(struct raw_header);
	rh = mtod(m, struct raw_header *);
	rh->raw_dst = *dst;
	rh->raw_src = *src;
	rh->raw_proto = *proto;

	/*
	 * Header now contains enough info to decide
	 * which socket to place packet in (if any).
	 * Queue it up for the raw protocol process
	 * running at software interrupt level.
	 */
	s = splimp();
	if (IF_QFULL(&rawintrq))
		m_freem(m);
	else
		IF_ENQUEUE(&rawintrq, m);
	splx(s);
	schednetisr(NETISR_RAW);
}

/*
 * Raw protocol input routine.  Process packets entered
 * into the queue at interrupt time.  Find the socket
 * associated with the packet(s) and move them over.  If
 * nothing exists for this packet, drop it.
 */
rawintr()
{
	int s;
	struct mbuf *m;
	register struct rawcb *rp;
	register struct protosw *lproto;
	register struct raw_header *rh;
	struct socket *last;

next:
	s = splimp();
	IF_DEQUEUE(&rawintrq, m);
	splx(s);
	if (m == 0)
		return;
	rh = mtod(m, struct raw_header *);
	last = 0;
	for (rp = rawcb.rcb_next; rp != &rawcb; rp = rp->rcb_next) {
		lproto = rp->rcb_socket->so_proto;
		if (lproto->pr_family != rh->raw_proto.sp_family)
			continue;
		if (lproto->pr_protocol &&
		    lproto->pr_protocol != rh->raw_proto.sp_protocol)
			continue;
		/*
		 * We assume the lower level routines have
		 * placed the address in a canonical format
		 * suitable for a structure comparison.
		 */
#define equal(a1, a2) \
	(bcmp((caddr_t)&(a1), (caddr_t)&(a2), sizeof (struct sockaddr)) == 0)
		if ((rp->rcb_flags & RAW_LADDR) &&
		    !equal(rp->rcb_laddr, rh->raw_dst))
			continue;
		if ((rp->rcb_flags & RAW_FADDR) &&
		    !equal(rp->rcb_faddr, rh->raw_src))
			continue;
		if (last) {
			struct mbuf *n;
			if ((n = m_copy(m->m_next, 0, (int)M_COPYALL)) == 0)
				goto nospace;
			if (sbappendaddr(&last->so_rcv, &rh->raw_src, n)==0) {
				/* should notify about lost packet */
				m_freem(n);
				goto nospace;
			}
			sorwakeup(last);
		}
nospace:
		last = rp->rcb_socket;
	}
	if (last) {
		m = m_free(m);		/* header */
		if (sbappendaddr(&last->so_rcv, &rh->raw_src, m) == 0)
			goto drop;
		sorwakeup(last);
		goto next;
	}
drop:
	m_freem(m);
	goto next;
}

/*ARGSUSED*/
raw_ctlinput(cmd, arg)
	int cmd;
	caddr_t arg;
{

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;
	/* INCOMPLETE */
}

/*ARGSUSED*/
raw_usrreq(so, req, m, nam)
	struct socket *so;
	int req;
	struct mbuf *m, *nam;
{
	register struct rawcb *rp = sotorawcb(so);
	int error = 0;

	if (rp == 0 && req != PRU_ATTACH)
		return (EINVAL);

	switch (req) {

	/*
	 * Allocate a raw control block and fill in the
	 * necessary info to allow packets to be routed to
	 * the appropriate raw interface routine.
	 */
	case PRU_ATTACH:
		if ((so->so_state & SS_PRIV) == 0)
			return (EACCES);
		if (rp)
			return (EINVAL);
		error = raw_attach(so);
		break;

	/*
	 * Destroy state just before socket deallocation.
	 * Flush data or not depending on the options.
	 */
	case PRU_DETACH:
		if (rp == 0)
			return (ENOTCONN);
		raw_detach(rp);
		break;

	/*
	 * If a socket isn't bound to a single address,
	 * the raw input routine will hand it anything
	 * within that protocol family (assuming there's
	 * nothing else around it should go to). 
	 */
	case PRU_CONNECT:
		if (rp->rcb_flags & RAW_FADDR)
			return (EISCONN);
		raw_connaddr(rp, nam);
		soisconnected(so);
		break;

	case PRU_DISCONNECT:
		if ((rp->rcb_flags & RAW_FADDR) == 0)
			return (ENOTCONN);
		raw_disconnect(rp);
		soisdisconnected(so);
		break;

	/*
	 * Mark the connection as being incapable of further input.
	 */
	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	/*
	 * Ship a packet out.  The appropriate raw output
	 * routine handles any massaging necessary.
	 */
	case PRU_SEND:
		if (nam) {
			if (rp->rcb_flags & RAW_FADDR)
				return (EISCONN);
			raw_connaddr(rp, nam);
		} else if ((rp->rcb_flags & RAW_FADDR) == 0)
			return (ENOTCONN);
		error = (*so->so_proto->pr_output)(m, so);
		if (nam)
			rp->rcb_flags &= ~RAW_FADDR;
		break;

	case PRU_ABORT:
		raw_disconnect(rp);
		sofree(so);
		soisdisconnected(so);
		break;

	/*
	 * Not supported.
	 */
	case PRU_ACCEPT:
	case PRU_RCVD:
	case PRU_CONTROL:
	case PRU_SENSE:
	case PRU_RCVOOB:
	case PRU_SENDOOB:
		error = EOPNOTSUPP;
		break;

	case PRU_SOCKADDR:
		bcopy((caddr_t)&rp->rcb_laddr, mtod(nam, caddr_t),
		    sizeof (struct sockaddr));
		nam->m_len = sizeof (struct sockaddr);
		break;

	default:
		panic("raw_usrreq");
	}
	return (error);
}



