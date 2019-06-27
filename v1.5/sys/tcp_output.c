/*	%M%	%I%	%E%	*/

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
#include "net/route.h"
#include "net/in_pcb.h"
#include "net/in_systm.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "net/tcp.h"
#define	TCPOUTFLAGS
#include "net/tcp_fsm.h"
#include "net/tcp_seq.h"
#include "net/tcp_timer.h"
#include "net/tcp_var.h"
#include "net/tcpip.h"
#include "net/tcp_debug.h"
#include "errno.h"

char *tcpstates[]; /* XXX */
#define returnerr(e) { error = (e); goto out; }

/*
 * Initial options: indicate max segment length 1/2 of space
 * allocated for receive; if TCPTRUEOOB is defined, indicate
 * willingness to do true out-of-band.
 */
#ifndef TCPTRUEOOB
u_char	tcp_initopt[4] = { TCPOPT_MAXSEG, 4, 0x0, 0x0, };
#else
u_char	tcp_initopt[6] = { TCPOPT_MAXSEG, 4, 0x0, 0x0, TCPOPT_WILLOOB, 2 };
#endif

/*
 * Tcp output routine: figure out what should be sent and send it.
 */
tcp_output(tp)
	register struct tcpcb *tp;
{
	register struct socket *so = tp->t_inpcb->inp_socket;
	register int len;
	register struct mbuf *m0;
	int off, flags, win, error;
	register struct mbuf *m;
	register struct tcpiphdr *ti;
	register u_char *opt = 0;
	register unsigned optlen = 0;
	register int sendalot;


	/*
	 * Determine length of data that should be transmitted,
	 * and flags that will be used.
	 * If there is some data or critical controls (SYN, RST)
	 * to send, then transmit; otherwise, investigate further.
	 */
#ifdef SIGH
	{ extern int intcpoutput; }
	{if (intcpoutput) printf("intcpoutput..."); intcpoutput++;}
#endif
again:
	sendalot = 0;
	off = tp->snd_nxt - tp->snd_una;
	len = MIN(so->so_snd.sb_cc, tp->snd_wnd+tp->t_force) - off;
	if (len < 0) {
		returnerr(0);     /* ??? */       /* past FIN */
	}
	if (len > tp->t_maxseg) {
		len = tp->t_maxseg;
		sendalot = 1;
	}

	flags = tcp_outflags[tp->t_state];
	if ( SEQ_LT((tp->snd_nxt + len), (tp->snd_una + so->so_snd.sb_cc)) )
		flags &= ~TH_FIN;
	if (flags & (TH_SYN|TH_RST|TH_FIN))
		goto send;
	if (SEQ_GT(tp->snd_up, tp->snd_una))
		goto send;

	/*
	 * Sender silly window avoidance.  If can send all data,
	 * a maximum segment, at least 1/4 of window do it,
	 * or are forced, do it; otherwise don't bother.
	 */
	if (len) {
		if (len == tp->t_maxseg || off+len >= so->so_snd.sb_cc)
			goto send;
		if (len * 4 >= tp->snd_wnd)		/* a lot */
			goto send;
		if (tp->t_force)
			goto send;
	}

	/*
	 * Send if we owe peer an ACK.
	 */
	if (tp->t_flags&TF_ACKNOW)
		goto send;

#ifdef TCPTRUEOOB
	/*
	 * Send if an out of band data or ack should be transmitted.
	 */
	if (tp->t_oobflags&(TCPOOB_OWEACK|TCPOOB_NEEDACK)))
		goto send;
#endif

	/*
	 * Calculate available window, and also amount
	 * of window known to peer (as advertised window less
	 * next expected input.)  If this is 35% or more of the
	 * maximum possible window, then want to send a segment to peer.
	 */
	win = sbspace(&so->so_rcv);
	if (win > 0 &&
	    ((100*(win-(tp->rcv_adv-tp->rcv_nxt))/so->so_rcv.sb_hiwat) >= 35))
		goto send;

	/*
	 * TCP window updates are not reliable, rather a polling protocol
	 * using ``persist'' packets is used to insure receipt of window
	 * updates.  The three ``states'' for the output side are:
	 *	idle			not doing retransmits or persists
	 *	persisting		to move a zero window
	 *	(re)transmitting	and thereby not persisting
	 *
	 * tp->t_timer[TCPT_PERSIST]
	 *	is set when we are in persist state.
	 * tp->t_force
	 *	is set when we are called to send a persist packet.
	 * tp->t_timer[TCPT_REXMT]
	 *	is set when we are retransmitting
	 * The output side is idle when both timers are zero.
	 *
	 * If send window is closed, there is data to transmit, and no
	 * retransmit or persist is pending, then go to persist state,
	 * arranging to force out a byte to get more current window information
	 * if nothing happens soon.
	 */
	if (tp->snd_wnd == 0 && so->so_snd.sb_cc &&
	    tp->t_timer[TCPT_REXMT] == 0 && tp->t_timer[TCPT_PERSIST] == 0) {
		tp->t_rxtshift = 0;
		tcp_setpersist(tp);
	}

	/*
	 * No reason to send a segment, just return.
	 */
	returnerr(0);

send:
	/*
	 * Grab a header mbuf, attaching a copy of data to
	 * be transmitted, and initialize the header from
	 * the template for sends on this connection.
	 */
	MGET(m, 0);
	if (m == 0) {
#ifdef SIGH
		printf ("cant get header in tcpoutput\n");
#endif
		returnerr(ENOBUFS);
	}
	m->m_off = MMAXOFF - sizeof (struct tcpiphdr);
	m->m_len = sizeof (struct tcpiphdr);
	if (len) {
		m->m_next = m_copy(so->so_snd.sb_mb, off, len);
		if (m->m_next == 0)
			len = 0;
	}
	ti = mtod(m, struct tcpiphdr *);
	if (tp->t_template == 0)
		panic("tcp_output");
	bcopy((caddr_t)tp->t_template, (caddr_t)ti, sizeof (struct tcpiphdr));

	/*
	 * Fill in fields, remembering maximum advertised
	 * window for use in delaying messages about window sizes.
	 */
	ti->ti_seq = tp->snd_nxt;
	ti->ti_ack = tp->rcv_nxt;
#ifndef WATCHOUT
	ti->ti_seq = htonl(ti->ti_seq);
	ti->ti_ack = htonl(ti->ti_ack);
#endif
	/*
	 * Before ESTABLISHED, force sending of initial options
	 * unless TCP set to not do any options.
	 */
	if (tp->t_state < TCPS_ESTABLISHED) {
		if (tp->t_flags&TF_NOOPT)
			goto noopt;
		opt = tcp_initopt;
		optlen = sizeof (tcp_initopt);
		*(u_short *)(opt + 2) = so->so_rcv.sb_hiwat / 2;
#ifdef SMALLTCP
		*(u_short *)(opt + 2) = 256;
#endif
#ifdef TCPACKMOST
		*(u_short *)(opt + 2) = so->so_rcv.sb_hiwat;
#endif
#ifndef WATCHOUT
		*(u_short *)(opt + 2) = htons(*(u_short *)(opt + 2));
#endif

	} else {
		if (tp->t_tcpopt == 0)
			goto noopt;
		opt = mtod(tp->t_tcpopt, u_char *);
		optlen = tp->t_tcpopt->m_len;
	}
#ifndef TCPTRUEOOB
	if (opt)
#else
	if (opt || (tp->t_oobflags&(TCPOOB_OWEACK|TCPOOB_NEEDACK)))
#endif
	{
		m0 = m->m_next;
		m->m_next = m_get(M_DONTWAIT);
		if (m->m_next == 0) {
			(void) m_free(m);
			m_freem(m0);
			returnerr(ENOBUFS);
		}
		m->m_next->m_next = m0;
		m0 = m->m_next;
		m0->m_off = MMINOFF;
		m0->m_len = optlen;
#ifdef WATCHOUT
		if (opt == tcp_initopt)
			bcopy((caddr_t)opt, mtod(m0, caddr_t), (int)optlen);
		else
			MBCOPY(tp->t_tcpopt, 0, m0, 0, (int)optlen);
#else
		bcopy((caddr_t)opt, mtod(m0, caddr_t), (int)optlen);
#endif
		opt = (u_char *)(mtod(m0, caddr_t) + optlen);
#ifdef TCPTRUEOOB
		if (tp->t_oobflags&TCPOOB_OWEACK) {
printf("tp %x send OOBACK for %x\n", tp->t_iobseq);
			*opt++ = TCPOPT_OOBACK;
			*opt++ = 3;
			*opt++ = tp->t_iobseq;
			m0->m_len += 3;
			tp->t_oobflags &= ~TCPOOB_OWEACK;
			/* sender should rexmt oob to force ack repeat */
		}
		if (tp->t_oobflags&TCPOOB_NEEDACK) {
			tcp_seq oobseq;
printf("tp %x send OOBDATA seq %x data %x\n", tp->t_oobseq, tp->t_oobc);
			*opt++ = TCPOPT_OOBDATA;
			*opt++ = 8;
			*opt++ = tp->t_oobseq;
			*opt++ = tp->t_oobc;
			oobseq = tp->t_oobmark - tp->snd_nxt;
#ifndef WATCHOUT
			oobseq = htonl(oobseq);
#endif
			bcopy((caddr_t)&oobseq, opt, sizeof oobseq);
			m0->m_len += 8;
			TCPT_RANGESET(tp->t_timer[TCPT_OOBREXMT],
			    (tcp_beta*tp->t_srtt)/100, TCPTV_MIN, TCPTV_MAX);
		}
#endif
		while (m0->m_len & 0x3) {
			*opt++ = TCPOPT_EOL;
			m0->m_len++;
		}
		optlen = m0->m_len;
		ti = mtod(m, struct tcpiphdr *);        /* needed for 11 */
		ti->ti_off = (sizeof (struct tcphdr) + optlen) >> 2;
	}
noopt:
	ti->ti_flags = flags;
	win = sbspace(&so->so_rcv);
#ifndef TCPACKMOST
	if (win < so->so_rcv.sb_hiwat / 4)	/* avoid silly window */
#else
	if (win < so->so_rcv.sb_hiwat)		/* avoid silly window */
#endif
		win = 0;
	if (win > 0)
#ifndef WATCHOUT
		ti->ti_win = htons((u_short)win);
#else
		ti->ti_win = win;
#endif
	if (SEQ_GT(tp->snd_up, tp->snd_nxt)) {
		ti->ti_urp = tp->snd_up - tp->snd_nxt;
#ifndef WATCHOUT
		ti->ti_urp = htons(ti->ti_urp);
#endif
		ti->ti_flags |= TH_URG;
	} else
		/*
		 * If no urgent pointer to send, then we pull
		 * the urgent pointer to the left edge of the send window
		 * so that it doesn't drift into the send window on sequence
		 * number wraparound.
		 */
		tp->snd_up = tp->snd_una;		/* drag it along */
	/*
	 * If anything to send and we can send it all, set PUSH.
	 * (This will keep happy those implementations which only
	 * give data to the user when a buffer fills or a PUSH comes in.
	 */
	if (len && off+len == so->so_snd.sb_cc)
		ti->ti_flags |= TH_PUSH;

	/*
	 * Put TCP length in extended header, and then
	 * checksum extended header and data.
	 */
	if (len + optlen) {
		ti->ti_len = sizeof (struct tcphdr) + optlen + len;
#ifndef WATCHOUT
		ti->ti_len = htons((u_short)ti->ti_len);
#endif
	}
	mbprint(m, "before tout ck");

	ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + (int)optlen + len);
	nprintf("tout sum%x len%x\n",ti->ti_sum,sizeof(struct tcpiphdr)+optlen+len);

	/*
	 * In transmit state, time the transmission and arrange for
	 * the retransmit.  In persist state, reset persist time for
	 * next persist.
	 */
	if (tp->t_force == 0) {
		/*
		 * Advance snd_nxt over sequence space of this segment.
		 */
		if (flags & (TH_SYN|TH_FIN))
			tp->snd_nxt++;
		tp->snd_nxt += len;

		/*
		 * Time this transmission if not a retransmission and
		 * not currently timing anything.
		 */
		if (SEQ_GT(tp->snd_nxt, tp->snd_max) && tp->t_rtt == 0) {
			tp->t_rtt = 1;
			tp->t_rtseq = tp->snd_nxt - len;
		}
		if (SEQ_GT(tp->snd_nxt, tp->snd_max))
			tp->snd_max = tp->snd_nxt;

		/*
		 * Set retransmit timer if not currently set.
		 * Initial value for retransmit timer to tcp_beta*tp->t_srtt.
		 * Initialize shift counter which is used for exponential
		 * backoff of retransmit time.
		 */
		if (tp->t_timer[TCPT_REXMT] == 0 &&
		    tp->snd_nxt != tp->snd_una) {
			TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
			    (tcp_beta*tp->t_srtt)/100, TCPTV_MIN, TCPTV_MAX);
			tp->t_rxtshift = 0;
		}
		tp->t_timer[TCPT_PERSIST] = 0;
	} else {
		if (SEQ_GT(tp->snd_una+1, tp->snd_max))
			tp->snd_max = tp->snd_una+1;
	}

	/*
	 * Trace.
	 */
	if (so->so_options & SO_DEBUG)
		tcp_trace(TA_OUTPUT, tp->t_state, tp, ti, 0);

	/*
	 * Fill in IP length and desired time to live and
	 * send to IP level.
	 */
	((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + optlen + len;
	((struct ip *)ti)->ip_ttl = TCP_TTL;
	if (error = ip_output(m, tp->t_ipopt, (so->so_options & SO_DONTROUTE) ?
	    &routetoif : &tp->t_inpcb->inp_route, 0)) {
		returnerr(error);
	}

	/*
	 * Data sent (as far as we can tell).
	 * If this advertises a larger window than any other segment,
	 * then remember the size of the advertised window.
	 * Drop send for purpose of ACK requirements.
	 */
	if (win > 0 && SEQ_GT(tp->rcv_nxt+win, tp->rcv_adv))
		tp->rcv_adv = tp->rcv_nxt + win;
	tp->t_flags &= ~(TF_ACKNOW|TF_DELACK);
	if (sendalot && tp->t_force == 0)
		goto again;
	returnerr(0);

out:
#ifdef SIGH
	intcpoutput--;
#endif
	return (error);
}

tcp_setpersist(tp)
	register struct tcpcb *tp;
{

	if (tp->t_timer[TCPT_REXMT])
		panic("tcp_output REXMT");
	/*
	 * Start/restart persistance timer.
	 */
	TCPT_RANGESET(tp->t_timer[TCPT_PERSIST],
	    ((tcp_beta * tp->t_srtt)/100) << tp->t_rxtshift,
	    TCPTV_PERSMIN, TCPTV_MAX);
	tp->t_rxtshift++;
	if (tp->t_rxtshift >= TCP_MAXRXTSHIFT)
		tp->t_rxtshift = 0;
}
