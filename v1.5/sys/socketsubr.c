/*	socketsubr.c	4.23	82/06/14	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/in.h"
#include "net/in_systm.h"

/*
 * Primitive routines for operating on sockets and socket buffers
 */

/*
 * Procedures to manipulate state flags of socket
 * and do appropriate wakeups.  Normal sequence is that
 * soisconnecting() is called during processing of connect() call,
 * resulting in an eventual call to soisconnected() if/when the
 * connection is established.  When the connection is torn down
 * soisdisconnecting() is called during processing of disconnect() call,
 * and soisdisconnected() is called when the connection to the peer
 * is totally severed.  The semantics of these routines are such that
 * connectionless protocols can call soisconnected() and soisdisconnected()
 * only, bypassing the in-progress calls when setting up a ``connection''
 * takes no time.
 *
 * When higher level protocols are implemented in
 * the kernel, the wakeups done here will sometimes
 * be implemented as software-interrupt process scheduling.
 */

soisconnecting(so)
	struct socket *so;
{

	so->so_state &= ~(SS_ISCONNECTED|SS_ISDISCONNECTING);
	so->so_state |= SS_ISCONNECTING;
	wakeup((caddr_t)&so->so_timeo);
}

soisconnected(so)
	struct socket *so;
{

	so->so_state &= ~(SS_ISCONNECTING|SS_ISDISCONNECTING);
	so->so_state |= SS_ISCONNECTED;
	wakeup((caddr_t)&so->so_timeo);
	sorwakeup(so);
	sowwakeup(so);
}

soisdisconnecting(so)
	struct socket *so;
{

	so->so_state &= ~SS_ISCONNECTING;
	so->so_state |= (SS_ISDISCONNECTING|SS_CANTRCVMORE|SS_CANTSENDMORE);
	wakeup((caddr_t)&so->so_timeo);
	sowwakeup(so);
	sorwakeup(so);
}

soisdisconnected(so)
	struct socket *so;
{

	so->so_state &= ~(SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING);
	so->so_state |= (SS_CANTRCVMORE|SS_CANTSENDMORE);
	wakeup((caddr_t)&so->so_timeo);
	sowwakeup(so);
	sorwakeup(so);
}

/*
 * Socantsendmore indicates that no more data will be sent on the
 * socket; it would normally be applied to a socket when the user
 * informs the system that no more data is to be sent, by the protocol
 * code (in case PRU_SHUTDOWN).  Socantrcvmore indicates that no more data
 * will be received, and will normally be applied to the socket by a
 * protocol when it detects that the peer will send no more data.
 * Data queued for reading in the socket may yet be read.
 */

socantsendmore(so)
	struct socket *so;
{

	so->so_state |= SS_CANTSENDMORE;
	sowwakeup(so);
}

socantrcvmore(so)
	struct socket *so;
{

	so->so_state |= SS_CANTRCVMORE;
	sorwakeup(so);
}

/*
 * Socket select/wakeup routines.
 */

/*
 * Interface routine to select() system
 * call for sockets.
 */
soselect(so, rw)
	register struct socket *so;
	int rw;
{
	int s = splnet();

	switch (rw) {

	case FREAD:
		if (soreadable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_rcv);
		break;

	case FWRITE:
		if (sowriteable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_snd);
		break;
	}
	splx(s);
	return (0);
}

/*
 * Queue a process for a select on a socket buffer.
 */
sbselqueue(sb)
	struct sockbuf *sb;
{
	register struct proc *p;

	if ((p = sb->sb_sel) && p->p_wchan == (caddr_t)&selwait)
		sb->sb_flags |= SB_COLL;
	else
		sb->sb_sel = u.u_procp;
}

/*
 * Wait for data to arrive at/drain from a socket buffer.
 */
sbwait(sb)
	struct sockbuf *sb;
{

	sb->sb_flags |= SB_WAIT;
	(void) sleep((caddr_t)&sb->sb_cc, PZERO+1);
}

/*
 * Wakeup processes waiting on a socket buffer.
 */
sbwakeup(sb)
	struct sockbuf *sb;
{

	if (sb->sb_sel) {
		selwakeup(sb->sb_sel, sb->sb_flags & SB_COLL);
		sb->sb_sel = 0;
		sb->sb_flags &= ~SB_COLL;
	}
	if (sb->sb_flags & SB_WAIT) {
		sb->sb_flags &= ~SB_WAIT;
		wakeup((caddr_t)&sb->sb_cc);
	}
}

/*
 * Socket buffer (struct sockbuf) utility routines.
 *
 * Each socket contains two socket buffers: one for sending data and
 * one for receiving data.  Each buffer contains a queue of mbufs,
 * information about the number of mbufs and amount of data in the
 * queue, and other fields allowing select() statements and notification
 * on data availability to be implemented.
 *
 * Before using a new socket structure it is first necessary to reserve
 * buffer space to the socket, by calling sbreserve.  This commits
 * some of the available buffer space in the system buffer pool for the
 * socket.  The space should be released by calling sbrelease when the
 * socket is destroyed.
 *
 * The routine sbappend() is normally called to append new mbufs
 * to a socket buffer, after checking that adequate space is available
 * comparing the function spspace() with the amount of data to be added.
 * Data is normally removed from a socket buffer in a protocol by
 * first calling m_copy on the socket buffer mbuf chain and sending this
 * to a peer, and then removing the data from the socket buffer with
 * sbdrop when the data is acknowledged by the peer (or immediately
 * in the case of unreliable protocols.)
 *
 * Protocols which do not require connections place both source address
 * and data information in socket buffer queues.  The source addresses
 * are stored in single mbufs after each data item, and are easily found
 * as the data items are all marked with end of record markers.  The
 * sbappendaddr() routine stores a datum and associated address in
 * a socket buffer.  Note that, unlike sbappend(), this routine checks
 * for the caller that there will be enough space to store the data.
 * It fails if there is not enough space, or if it cannot find
 * a mbuf to store the address in.
 *
 * The higher-level routines sosend and soreceive (in socket.c)
 * also add data to, and remove data from socket buffers repectively.
 */


#ifdef notdef
/* billn -- add this routine from 4.1c for 4.1c udp */
soreserve(so, sndcc, rcvcc)
	struct socket *so;
	int sndcc, rcvcc;
{

	if (sbreserve(&so->so_snd, sndcc) == 0)
		goto bad;
	if (sbreserve(&so->so_rcv, rcvcc) == 0)
		goto bad2;
	return (0);
bad2:
	sbrelease(&so->so_snd);
bad:
	return (ENOBUFS);
}
#endif

/*
 * Allot mbufs to a sockbuf.
 */
sbreserve(sb, cc)
	struct sockbuf *sb;
{

	/* someday maybe this routine will fail... */
	sb->sb_hiwat = cc;
	sb->sb_mbmax = cc*2;
	return (1);
}

/*
 * Free mbufs held by a socket, and reserved mbuf space.
 */
sbrelease(sb)
	struct sockbuf *sb;
{

	sbflush(sb);
	sb->sb_hiwat = sb->sb_mbmax = 0;
}

/*
 * Routines to add (at the end) and remove (from the beginning)
 * data from a mbuf queue.
 */

/*
 * Append mbuf queue m to sockbuf sb.
 */
sbappend(sb, m)
	register struct mbuf *m;
	register struct sockbuf *sb;
{
	register struct mbuf *n;

	n = sb->sb_mb;
#ifdef SIGH
	mcheck(n, "sbappend");
#endif
	if (n)
		while (n->m_next)
			n = n->m_next;
	while (m) {
		if (m->m_len == 0 && (int)m->m_act == 0) {
			m = m_free(m);
			continue;
		}
		if (n && n->m_off <= MMAXOFF && m->m_off <= MMAXOFF &&
		   (int)n->m_act == 0 && (int)m->m_act == 0 &&
		   (n->m_off + n->m_len + m->m_len) <= MMAXOFF) {
			MBCOPY(m, 0, n, n->m_len, (u_int)m->m_len);
			n->m_len += m->m_len;
			sb->sb_cc += m->m_len;
			m = m_free(m);
			continue;
		}
		sballoc(sb, m);
		if (n == 0) {
			sb->sb_mb = m;
#ifdef SIGH
	mcheck(n, "sbappend2");
#endif
		}
		else
			n->m_next = m;
		n = m;
		m = m->m_next;
		n->m_next = 0;
	}
}

/*
 * Append data and address.
 * Return 0 if no space in sockbuf or if
 * can't get mbuf to stuff address in.
 */
sbappendaddr(sb, asa, m0)
	struct sockbuf *sb;
	struct sockaddr *asa;
	struct mbuf *m0;
{
	struct sockaddr *msa;
	register struct mbuf *m;
	register int len = sizeof (struct sockaddr);
	struct sockaddr sa;

	sa = *asa;
	m = m0;
	if (m == 0)
		panic("sbappendaddr");
	for (;;) {
		len += m->m_len;
		if (m->m_next == 0) {
			m->m_act = (struct mbuf *)1;
			break;
		}
		/* This else clause is an sri bug-fix by JC Stewart */
		else {
			m->m_act = (struct mbuf *) 0;
		}
		/* */
		m = m->m_next;
	}
	if (len > sbspace(sb))
		return (0);
	m = m_get(M_DONTWAIT);
	if (m == 0)
		return (0);
	m->m_off = MMINOFF;
	m->m_len = sizeof (struct sockaddr);
	MAPSAVE();
	msa = mtod(m, struct sockaddr *);
	*msa = sa;
	MAPREST();
	m->m_act = (struct mbuf *)1;
	sbappend(sb, m);
	sbappend(sb, m0);
	return (1);
}

/*
 * Free all mbufs on a sockbuf mbuf chain.
 * Check that resource allocations return to 0.
 */
sbflush(sb)
	struct sockbuf *sb;
{

	if (sb->sb_flags & SB_LOCK)
		panic("sbflush");
	if (sb->sb_cc)
		sbdrop(sb, sb->sb_cc);
#ifdef SIGH
	/*
	else
		return;
	*/
#endif
	if (sb->sb_cc || sb->sb_mbcnt || sb->sb_mb) {
#ifdef SIGH
		extern struct mbuf *mfreep;

		printf("sbflush: sb_cc=%x, sb_mbcnt=%x, sb_mb=%x, mfreep=%x\n",
		sb->sb_cc,sb->sb_mbcnt, sb->sb_mb, mfreep);
#endif
		panic("sbflush 2");
	}
}

/*
 * Drop data from (the front of) a sockbuf chain.
 */
sbdrop(sb, len)
	register struct sockbuf *sb;
	register int len;
{
	register struct mbuf *m = sb->sb_mb, *mn;

#ifdef SIGH
	mcheck (m, "sbdrop1");
#endif
	while (len > 0) {
		if (m == 0)
			panic("sbdrop");
		if (m->m_len > len) {
			m->m_len -= len;
			m->m_off += len;
			sb->sb_cc -= len;
			break;
		}
		len -= m->m_len;
		sbfree(sb, m);
		MFREE(m, mn);
		m = mn;
	}
	sb->sb_mb = m;
#ifdef SIGH
	mcheck (m, "sbdrop2");
#endif
}
