/*	tcp_subr.c	4.27	82/06/20	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/protosw.h"
#include "net/in.h"
#include "net/route.h"
#include "net/in_pcb.h"
#include "net/in_systm.h"
#include "net/if.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "net/ip_icmp.h"
#include "net/tcp.h"
#include "net/tcp_fsm.h"
#include "net/tcp_seq.h"
#include "net/tcp_timer.h"
#include "net/tcp_var.h"
#include "net/tcpip.h"
#include "errno.h"

/*
 * Tcp initialization
 */
tcp_init()
{

	tcp_iss = 1;		/* wrong */
	tcb.inp_next = tcb.inp_prev = &tcb;
	tcp_alpha = TCP_ALPHA;
	tcp_beta = TCP_BETA;
}

/*
 * Create template to be used to send tcp packets on a connection.
 * Call after host entry created, allocates an mbuf and fills
 * in a skeletal tcp/ip header, minimizing the amount of work
 * necessary when the connection is used.
 */
struct tcpiphdr *
tcp_template(tp)
	register struct tcpcb *tp;
{
	register struct inpcb *inp = tp->t_inpcb;
	register struct tcpiphdr *n;

	MSGET(n, struct tcpiphdr, 1);
	if (n == 0)
		return (0);
	n->ti_pr = IPPROTO_TCP;
	n->ti_len = htons(sizeof (struct tcpiphdr) - sizeof (struct ip));
	n->ti_src = inp->inp_laddr;
	n->ti_dst = inp->inp_faddr;
	n->ti_sport = inp->inp_lport;
	n->ti_dport = inp->inp_fport;
	n->ti_off = 5;
	return (n);
}

/*
 * Send a single message to the TCP at address specified by
 * the given TCP/IP header.  If flags==0, then we make a copy
 * of the tcpiphdr at ti and send directly to the addressed host.
 * This is used to force keep alive messages out using the TCP
 * template for a connection tp->t_template.  If flags are given
 * then we send a message back to the TCP which originated the
 * segment ti, and discard the mbuf containing it and any other
 * attached mbufs.
 *
 * In any case the ack and sequence number of the transmitted
 * segment are as specified by the parameters.
 */
tcp_respond(tp, ti, ack, seq, flags)
	register struct tcpcb *tp;
	register struct tcpiphdr *ti;
	register tcp_seq ack, seq;
	register int flags;
{
	register struct mbuf *m;
	register int win = 0, tlen;
	register struct route *ro = 0;

	if (tp) {
		win = sbspace(&tp->t_inpcb->inp_socket->so_rcv);
		ro = &tp->t_inpcb->inp_route;
	}
	if (flags == 0) {
		m = m_get(M_DONTWAIT);
		if (m == 0)
			return;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct tcpiphdr) + 1;
		bcopy((caddr_t)ti, mtod(m, caddr_t), sizeof *ti);
		ti = mtod(m, struct tcpiphdr *);
		flags = TH_ACK;
		tlen = 1;
	} else {
		m = dtom(ti);
		m_freem(m->m_next);
		m->m_next = 0;
#ifndef  newmbufs
		m->m_off = (int)ti - (int)m;
#else
		m->m_off = (int)ti - mtobuf(m, int);
#endif
		m->m_len = sizeof (struct tcpiphdr);
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
		xchg(ti->ti_dst.s_addr, ti->ti_src.s_addr, u_long);
		xchg(ti->ti_dport, ti->ti_sport, u_short);
#undef xchg
		tlen = 0;
	}
	ti->ti_next = ti->ti_prev = 0;
	ti->ti_x1 = 0;
	ti->ti_len = sizeof (struct tcphdr) + tlen;
	ti->ti_seq = seq;
	ti->ti_ack = ack;
#ifndef WATCHOUT
	ti->ti_len = htons((u_short)ti->ti_len);
	ti->ti_seq = htonl(ti->ti_seq);
	ti->ti_ack = htonl(ti->ti_ack);
#endif
	ti->ti_x2 = 0;
	ti->ti_off = sizeof (struct tcphdr) >> 2;
	ti->ti_flags = flags;
	ti->ti_win = win;
#ifndef WATCHOUT
	ti->ti_win = htons(ti->ti_win);
#endif
	ti->ti_urp = 0;

	/* billn
	ti->ti_sum = 0;
	*/

	ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + tlen);
	((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + tlen;
	((struct ip *)ti)->ip_ttl = TCP_TTL;
	(void) ip_output(m, (struct mbuf *)0, ro, 0);
}

/*
 * Create a new TCP control block, making an
 * empty reassembly queue and hooking it to the argument
 * protocol control block.
 */
struct tcpcb *
tcp_newtcpcb(inp)
	register struct inpcb *inp;
{
	register struct tcpcb *tp;

	MSGET(tp, struct tcpcb, 1);
	if (tp == 0)
		return (0);
	tp->seg_next = tp->seg_prev = (struct tcpiphdr *)tp;
	/*  Correction from Dan@sri-tsc
	tp->t_maxseg = 576;		/* satisfy the rest of the world */
	tp->t_maxseg = 512;		/* satisfy the rest of the world */
	tp->t_flags = 0;		/* sends options! */
	tp->t_inpcb = inp;
	inp->inp_ppcb = (caddr_t)tp;
	return (tp);
}

/*
 * Drop a TCP connection, reporting
 * the specified error.  If connection is synchronized,
 * then send a RST to peer.
 */
tcp_drop(tp, errno)
	register struct tcpcb *tp;
	register int errno;
{
	struct socket *so = tp->t_inpcb->inp_socket;

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tp->t_state = TCPS_CLOSED;
		(void) tcp_output(tp);
	}
	so->so_error = errno;
	tcp_close(tp);
}

tcp_abort(inp)
	register struct inpcb *inp;
{
	tcp_close((struct tcpcb *)inp->inp_ppcb);
}

#ifdef WATCHOUT
#define ti_mbuf ti_sum
#define DTOM(d) ( (struct mbuf *) ((d)->ti_mbuf) )
#else
#define DTOM(d) dtom(d)
#endif

/*
 * Close a TCP control block:
 *	discard all space held by the tcp
 *	discard internet protocol block
 *	wake up any sleepers
 */
tcp_close(tp)
	register struct tcpcb *tp;
{
	register struct tcpiphdr *t;
#ifdef WATCHOUT
	register struct tcpiphdr *to;
#endif
	register struct inpcb *inp = tp->t_inpcb;
	register struct socket *so = inp->inp_socket;

	for (t = tp->seg_next; t != (struct tcpiphdr *)tp;) {
		m_freem(DTOM(t));
#ifdef WATCHOUT
		to = t;
#endif
		t = (struct tcpiphdr *)t->ti_next;
#ifdef WATCHOUT
		MSFREE(to);
#endif
	}
	if (tp->t_template)
		MSFREE(tp->t_template);
	if (tp->t_tcpopt)
		(void) m_free(tp->t_tcpopt);
	if (tp->t_ipopt)
		(void) m_free(tp->t_ipopt);
	MSFREE(tp);
	inp->inp_ppcb = 0;
	soisdisconnected(so);
	in_pcbdetach(inp);
}

tcp_drain()
{

}

tcp_ctlinput(cmd, arg)
	register int cmd;
	register caddr_t arg;
{
	register struct in_addr *sin;
	extern u_char inetctlerrmap[];

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;
	switch (cmd) {

	case PRC_ROUTEDEAD:
		break;

	case PRC_QUENCH:
		break;

	/* these are handled by ip */
	case PRC_IFDOWN:
	case PRC_HOSTDEAD:
	case PRC_HOSTUNREACH:
		break;

	default:
		sin = &((struct icmp *)arg)->icmp_ip.ip_dst;
		in_pcbnotify(&tcb, sin, inetctlerrmap[cmd], tcp_abort);
	}
}
