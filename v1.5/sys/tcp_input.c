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
#include "net/if.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "net/tcp.h"
#include "net/tcp_fsm.h"
#include "net/tcp_seq.h"
#include "net/tcp_timer.h"
#include "net/tcp_var.h"
#include "net/tcpip.h"
#include "net/tcp_debug.h"
#include "errno.h"

int     tcpprintfs = 1; /* 1=print, 2= panic */
short	tcpcksum = 1;
struct	sockaddr_in tcp_in = { AF_INET };
struct	tcpiphdr tcp_saveti;
extern	tcpnodelack;

struct	tcpcb *tcp_newtcpcb();
/*
 * TCP input routine, follows pages 65-76 of the
 * protocol specification dated September, 1981 very closely.
 */
tcp_input(m0)
	register struct mbuf *m0;
{
	register struct tcpiphdr *ti;
	register struct inpcb *inp;
	register struct mbuf *m;
	struct mbuf *om = 0;
	int len, tlen, off;
	register struct tcpcb *tp = 0;
	register int tiflags;
	register struct socket *so;
	register int todrop, acked;
	short ostate;
	struct in_addr laddr;

	/*
	 * Get IP and TCP header together in first mbuf.
	 * Note: IP leaves IP header in first mbuf.
	 */
#ifdef SIGH
	{ extern int intcpinput; }
	if (intcpinput) printf("intcpinput..."); else intcpinput++;
#endif
	m = m0;
	ti = mtod(m, struct tcpiphdr *);
	if (((struct ip *)ti)->ip_hl > (sizeof (struct ip) >> 2))
		ip_stripoptions((struct ip *)ti, (struct mbuf *)0);
	if (m->m_off > MMAXOFF || m->m_len < sizeof (struct tcpiphdr)) {
		if ((m = m_pullup(m, sizeof (struct tcpiphdr))) == 0) {
			tcpstat.tcps_hdrops++;
#ifdef SIGH
	intcpinput--;
#endif
			return;
		}
		ti = mtod(m, struct tcpiphdr *);
	}

	/*
	 * Checksum extended TCP header and data.
	 */
	tlen = ((struct ip *)ti)->ip_len;
	len = sizeof (struct ip) + tlen;
	/*
	if ((tlen > 576) || (len > 700))
		printf("len=%d, tlen=%d !!!\n", len, tlen);
	*/
	if (tcpcksum) {
		register sum;
		ti->ti_next = ti->ti_prev = 0;
		ti->ti_x1 = 0;
		ti->ti_len = (u_short)tlen;
#ifndef WATCHOUT
		ti->ti_len = htons((u_short)ti->ti_len);
#endif
		mbprint(m, "tin before ck");
		nprintf("len%o\n", (unsigned)len);
		/*
		if (len == 40)
{int i; int j; printf("segment:\n");for(j=0;j<4;j++){for(i=0;i<10;i++)printf("%x ",(((char *)ti)[i+10*j])&0xff);printf("\n");}}
		if (ti->ti_sum = in_cksum(m, len)) {
		*/
		if (sum = in_cksum(m, len)) {
			nprintf("badsum%o\n", ti->ti_sum);
			tcpstat.tcps_badsum++;
			if (tcpprintfs)
			    /*
			    printf("tcp cksum %x, length %d, loca %x\n",ti->ti_sum,len,ti);
			    */
			    printf("tcp cksum %x, length %d, loca %x\n",sum,len,ti);
			if (len == 40)
	{int i; int j; printf("segment:\n");for(j=0;j<4;j++){for(i=0;i<10;i++)printf("%x ",(((char *)ti)[i+10*j])&0xff);printf("\n");}}
			if (tcpprintfs==2) panic("tcp ck\n");
			goto drop;
		}
		ti->ti_sum = sum;
	}

	/*
	 * Check that TCP offset makes sense,
	 * pull out TCP options and adjust length.
	 */
	off = ti->ti_off << 2;
	if (off < sizeof (struct tcphdr) || off > tlen) {
		tcpstat.tcps_badoff++;
		goto drop;
	}
	tlen -= off;
	ti->ti_len = tlen;
	if (off > sizeof (struct tcphdr)) {
		if ((m = m_pullup(m, sizeof (struct ip) + off)) == 0) {
			tcpstat.tcps_hdrops++;
#ifdef SIGH
	intcpinput--;
#endif
			return;
		}
		ti = mtod(m, struct tcpiphdr *);
		om = m_get(M_DONTWAIT);
		if (om == 0)
			goto drop;
		om->m_off = MMINOFF;
		om->m_len = off - sizeof (struct tcphdr);
		{ caddr_t op = mtod(m, caddr_t) + sizeof (struct tcpiphdr);
		  MBCOPY(m, sizeof(struct tcpiphdr), om, 0, om->m_len);
		  m->m_len -= om->m_len;
		  bcopy(op+om->m_len, op,
		   (int)(m->m_len-sizeof (struct tcpiphdr)));
		}
	}
	tiflags = ti->ti_flags;

	/*
	 * Drop TCP and IP headers.
	 */
	m->m_off += sizeof(struct tcpiphdr);
	m->m_len -= sizeof(struct tcpiphdr);

#ifndef WATCHOUT
	/*
	 * Convert TCP protocol specific fields to host format.
	 */
	ti->ti_seq = ntohl(ti->ti_seq);
	ti->ti_ack = ntohl(ti->ti_ack);
	ti->ti_win = ntohs(ti->ti_win);
	ti->ti_urp = ntohs(ti->ti_urp);
#endif

	/*
	 * Locate pcb for segment.  On match, update the local
	 * address stored in the block to reflect anchoring.
	 */
	inp = in_pcblookup
		(&tcb, ti->ti_src, ti->ti_sport, ti->ti_dst, ti->ti_dport,
		INPLOOKUP_WILDCARD);

	/*
	 * If the state is CLOSED (i.e., TCB does not exist) then
	 * all data in the incoming segment is discarded.
	 */
	if (inp == 0)
		goto dropwithreset;
	tp = intotcpcb(inp);
	if (tp == 0)
		goto dropwithreset;
	so = inp->inp_socket;
	if (so->so_options & SO_DEBUG) {
		ostate = tp->t_state;
		tcp_saveti = *ti;
	}

	/*
	 * Segment received on connection.
	 * Reset idle time and keep-alive timer.
	 */
	tp->t_idle = 0;
	tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;

	/*
	 * Process options.
	 */
	if (om) {
		tcp_dooptions(tp, om);
		om = 0;
	}

	/*
	 * Calculate amount of space in receive window,
	 * and then do TCP input processing.
	 */
	tp->rcv_wnd = sbspace(&so->so_rcv);
	if (tp->rcv_wnd < 0)
		tp->rcv_wnd = 0;

	switch (tp->t_state) {

	/*
	 * If the state is LISTEN then ignore segment if it contains an RST.
	 * If the segment contains an ACK then it is bad and send a RST.
	 * If it does not contain a SYN then it is not interesting; drop it.
	 * Otherwise initialize tp->rcv_nxt, and tp->irs, select an initial
	 * tp->iss, and send a segment:
	 *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
	 * Also initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to tp->iss.
	 * Fill in remote peer address fields if not previously specified.
	 * Enter SYN_RECEIVED state, and process any other fields of this
	 * segment in this state.
	 */
	case TCPS_LISTEN:
		if (tiflags & TH_RST)
			goto drop;
		if (tiflags & TH_ACK)
			goto dropwithreset;
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		tcp_in.sin_addr = ti->ti_src;
		tcp_in.sin_port = ti->ti_sport;
		laddr = inp->inp_laddr;
		if (inp->inp_laddr.s_addr == 0)
			inp->inp_laddr = ti->ti_dst;
		if (in_pcbconnect(inp, (struct sockaddr_in *)&tcp_in)) {
			inp->inp_laddr = laddr;
			goto drop;
		}
		tp->t_template = tcp_template(tp);
		if (tp->t_template == 0) {
			in_pcbdisconnect(inp);
			inp->inp_laddr = laddr;
			tp = 0;
			goto drop;
		}
		tp->iss = tcp_iss; tcp_iss += TCP_ISSINCR/2;
		tp->irs = ti->ti_seq;
		tcp_sendseqinit(tp);
		tcp_rcvseqinit(tp);
		tp->t_state = TCPS_SYN_RECEIVED;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;
		goto trimthenstep6;

	/*
	 * If the state is SYN_SENT:
	 *	if seg contains an ACK, but not for our SYN, drop the input.
	 *	if seg contains a RST, then drop the connection.
	 *	if seg does not contain SYN, then drop it.
	 * Otherwise this is an acceptable SYN segment
	 *	initialize tp->rcv_nxt and tp->irs
	 *	if seg contains ack then advance tp->snd_una
	 *	if SYN has been acked change to ESTABLISHED else SYN_RCVD state
	 *	arrange for segment to be acked (eventually)
	 *	continue processing rest of data/controls, beginning with URG
	 */
	case TCPS_SYN_SENT:
		if ((tiflags & TH_ACK) &&
/* this should be SEQ_LT; is SEQ_LEQ for BBN vax TCP only */
		    (SEQ_LT(ti->ti_ack, tp->iss) ||
		     SEQ_GT(ti->ti_ack, tp->snd_max)))
			goto dropwithreset;
		if (tiflags & TH_RST) {
			if (tiflags & TH_ACK) {
				tcp_drop(tp, ECONNREFUSED);
				tp = 0;
			}
			goto drop;
		}
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		tp->snd_una = ti->ti_ack;
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;
		tp->t_timer[TCPT_REXMT] = 0;
		tp->irs = ti->ti_seq;
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
		if (SEQ_GT(tp->snd_una, tp->iss)) {
			if (so->so_options & SO_ACCEPTCONN)
				so->so_state |= SS_CONNAWAITING;
			soisconnected(so);
			tp->t_state = TCPS_ESTABLISHED;
			(void) tcp_reass(tp, (struct tcpiphdr *)0);
		} else
			tp->t_state = TCPS_SYN_RECEIVED;
		goto trimthenstep6;

trimthenstep6:
		/*
		 * Advance ti->ti_seq to correspond to first data byte.
		 * If data, trim to stay within window,
		 * dropping FIN if necessary.
		 */
		ti->ti_seq++;
		if (ti->ti_len > tp->rcv_wnd) {
			todrop = ti->ti_len - tp->rcv_wnd;
			m_adj(m, -todrop);
			ti->ti_len = tp->rcv_wnd;
			ti->ti_flags &= ~TH_FIN;
		}
		tp->snd_wl1 = ti->ti_seq - 1;
		goto step6;
	}

	/*
	 * States other than LISTEN or SYN_SENT.
	 * First check that at least some bytes of segment are within 
	 * receive window.
	 */
	if (tp->rcv_wnd == 0) {
		/*
		 * If window is closed can only take segments at
		 * window edge, and have to drop data and PUSH from
		 * incoming segments.
		 */
		if (tp->rcv_nxt != ti->ti_seq)
			goto dropafterack;
		if (ti->ti_len > 0) {
			m_adj(m, ti->ti_len);
			ti->ti_len = 0;
			ti->ti_flags &= ~(TH_PUSH|TH_FIN);
		}
	} else {
		/*
		 * If segment begins before rcv_nxt, drop leading
		 * data (and SYN); if nothing left, just ack.
		 */
		todrop = tp->rcv_nxt - ti->ti_seq;
		if (todrop > 0) {
			if (tiflags & TH_SYN) {
				tiflags &= ~TH_SYN;
				ti->ti_flags &= ~TH_SYN;
				ti->ti_seq++;
				if (ti->ti_urp > 1) 
					ti->ti_urp--;
				else
					tiflags &= ~TH_URG;
				todrop--;
			}
			if (todrop > ti->ti_len ||
			    todrop == ti->ti_len && (tiflags&TH_FIN) == 0)
				goto dropafterack;
			m_adj(m, todrop);
			ti->ti_seq += todrop;
			ti->ti_len -= todrop;
			if (ti->ti_urp > todrop)
				ti->ti_urp -= todrop;
			else {
				tiflags &= ~TH_URG;
				ti->ti_flags &= ~TH_URG;
				ti->ti_urp = 0;
			}
		}
		/*
		 * If segment ends after window, drop trailing data
		 * (and PUSH and FIN); if nothing left, just ACK.
		 */
		todrop = (ti->ti_seq+ti->ti_len) - (tp->rcv_nxt+tp->rcv_wnd);
		if (todrop > 0) {
			if (todrop >= ti->ti_len)
				goto dropafterack;
			m_adj(m, -todrop);
			ti->ti_len -= todrop;
			ti->ti_flags &= ~(TH_PUSH|TH_FIN);
		}
	}

	/*
	 * If a segment is received on a connection after the
	 * user processes are gone, then RST the other end.
	 */
	if ((so->so_state & SS_USERGONE) && tp->t_state > TCPS_CLOSE_WAIT &&
	    ti->ti_len) {
		tcp_close(tp);
		tp = 0;
		goto dropwithreset;
	}

	/*
	 * If the RST bit is set examine the state:
	 *    SYN_RECEIVED STATE:
	 *	If passive open, return to LISTEN state.
	 *	If active open, inform user that connection was refused.
	 *    ESTABLISHED, FIN_WAIT_1, FIN_WAIT2, CLOSE_WAIT STATES:
	 *	Inform user that connection was reset, and close tcb.
	 *    CLOSING, LAST_ACK, TIME_WAIT STATES
	 *	Close the tcb.
	 */
	if (tiflags&TH_RST) switch (tp->t_state) {

	case TCPS_SYN_RECEIVED:
		if (inp->inp_socket->so_options & SO_ACCEPTCONN) {
			/* a miniature tcp_close, but invisible to user */
			if (tp->t_template) MSFREE(tp->t_template);
			MSFREE(tp);
			inp->inp_ppcb = 0;
			tp = tcp_newtcpcb(inp);
			if (tp == 0) {  /* unlikely but just in case */
				tcp_drop(tp, ENOBUFS);
				tp = 0;
				goto drop;
			}
			tp->t_state = TCPS_LISTEN;
			inp->inp_faddr.s_addr = 0;
			inp->inp_fport = 0;
			inp->inp_laddr.s_addr = 0;	/* not quite right */
			tp = 0;
			goto drop;
		}
		tcp_drop(tp, ECONNREFUSED);
		tp = 0;
		goto drop;

	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
		tcp_drop(tp, ECONNRESET);
		tp = 0;
		goto drop;

	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
		tcp_close(tp);
		tp = 0;
		goto drop;
	}

	/*
	 * If a SYN is in the window, then this is an
	 * error and we send an RST and drop the connection.
	 */
	if (tiflags & TH_SYN) {
		tcp_drop(tp, ECONNRESET);
		tp = 0;
		goto dropwithreset;
	}

	/*
	 * If the ACK bit is off we drop the segment and return.
	 */
	if ((tiflags & TH_ACK) == 0)
		goto drop;
	
	/*
	 * Ack processing.
	 */
	switch (tp->t_state) {

	/*
	 * In SYN_RECEIVED state if the ack ACKs our SYN then enter
	 * ESTABLISHED state and continue processing, othewise
	 * send an RST.
	 */
	case TCPS_SYN_RECEIVED:
		if (SEQ_GT(tp->snd_una, ti->ti_ack) ||
		    SEQ_GT(ti->ti_ack, tp->snd_max))
			goto dropwithreset;
		tp->snd_una++;			/* SYN acked */
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;
		tp->t_timer[TCPT_REXMT] = 0;
		if (so->so_options & SO_ACCEPTCONN)
			so->so_state |= SS_CONNAWAITING;
		soisconnected(so);
		tp->t_state = TCPS_ESTABLISHED;
		(void) tcp_reass(tp, (struct tcpiphdr *)0);
		tp->snd_wl1 = ti->ti_seq - 1;
		/* fall into ... */

	/*
	 * In ESTABLISHED state: drop duplicate ACKs; ACK out of range
	 * ACKs.  If the ack is in the range
	 *	tp->snd_una < ti->ti_ack <= tp->snd_max
	 * then advance tp->snd_una to ti->ti_ack and drop
	 * data from the retransmission queue.  If this ACK reflects
	 * more up to date window information we update our window information.
	 */
	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
#define	ourfinisacked	(acked > 0)

		if (SEQ_LEQ(ti->ti_ack, tp->snd_una))
			break;
		if (SEQ_GT(ti->ti_ack, tp->snd_max))
			goto dropafterack;
		acked = ti->ti_ack - tp->snd_una;

		/*
		 * If transmit timer is running and timed sequence
		 * number was acked, update smoothed round trip time.
		 */
		if (tp->t_rtt && SEQ_GT(ti->ti_ack, tp->t_rtseq)) {
			if (tp->t_srtt == 0)
				tp->t_srtt = tp->t_rtt * 10;
			else
				tp->t_srtt =
				    (tcp_alpha * tp->t_srtt) / 10 +
				    (10 - tcp_alpha) * tp->t_rtt;
/* printf("rtt %d srtt(*10) now %d\n", tp->t_rtt, tp->t_srtt); */
			tp->t_rtt = 0;
		}

		if (ti->ti_ack == tp->snd_max)
			tp->t_timer[TCPT_REXMT] = 0;
		else {
			TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
			    (tcp_beta * tp->t_srtt)/100, TCPTV_MIN, TCPTV_MAX);
			tp->t_rtt = 1;
			tp->t_rxtshift = 0;
		}
		if (acked > so->so_snd.sb_cc) {
			sbdrop(&so->so_snd, so->so_snd.sb_cc);
			tp->snd_wnd -= so->so_snd.sb_cc;
		} else {
			sbdrop(&so->so_snd, acked);
			tp->snd_wnd -= acked;
			acked = 0;
		}
		if ((so->so_snd.sb_flags & SB_WAIT) || so->so_snd.sb_sel)
			sowwakeup(so);
		tp->snd_una = ti->ti_ack;
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;

		switch (tp->t_state) {

		/*
		 * In FIN_WAIT_1 STATE in addition to the processing
		 * for the ESTABLISHED state if our FIN is now acknowledged
		 * then enter FIN_WAIT_2.
		 */
		case TCPS_FIN_WAIT_1:
			if (ourfinisacked) {
				/*
				 * If we can't receive any more
				 * data, then closing user can proceed.
				 */
				if (so->so_state & SS_CANTRCVMORE)
					soisdisconnected(so);
				tp->t_state = TCPS_FIN_WAIT_2;
			}
			break;

	 	/*
		 * In CLOSING STATE in addition to the processing for
		 * the ESTABLISHED state if the ACK acknowledges our FIN
		 * then enter the TIME-WAIT state, otherwise ignore
		 * the segment.
		 */
		case TCPS_CLOSING:
			if (ourfinisacked) {
				tp->t_state = TCPS_TIME_WAIT;
				tcp_canceltimers(tp);
				tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
				soisdisconnected(so);
			}
			break;

		/*
		 * The only thing that can arrive in  LAST_ACK state
		 * is an acknowledgment of our FIN.  If our FIN is now
		 * acknowledged, delete the TCB, enter the closed state
		 * and return.
		 */
		case TCPS_LAST_ACK:
			if (ourfinisacked) {
				tcp_close(tp);
				tp = 0;
			}
			goto drop;

		/*
		 * In TIME_WAIT state the only thing that should arrive
		 * is a retransmission of the remote FIN.  Acknowledge
		 * it and restart the finack timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			goto dropafterack;
		}
#undef ourfinisacked
	}

step6:
	/*
	 * Update window information.
	 */
	if (SEQ_LT(tp->snd_wl1, ti->ti_seq) || tp->snd_wl1 == ti->ti_seq &&
	    (SEQ_LT(tp->snd_wl2, ti->ti_ack) ||
	     tp->snd_wl2 == ti->ti_ack && ti->ti_win > tp->snd_wnd)) {
		tp->snd_wnd = ti->ti_win;
		tp->snd_wl1 = ti->ti_seq;
		tp->snd_wl2 = ti->ti_ack;
		/*
		if (tp->snd_wnd > 0)
		*/
		if (tp->snd_wnd != 0)
			tp->t_timer[TCPT_PERSIST] = 0;
	}

	/*
	 * Process segments with URG.
	 */
	if ((tiflags & TH_URG) && ti->ti_urp &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		/*
		 * If this segment advances the known urgent pointer,
		 * then mark the data stream.  This should not happen
		 * in CLOSE_WAIT, CLOSING, LAST_ACK or TIME_WAIT STATES since
		 * a FIN has been received from the remote side. 
		 * In these states we ignore the URG.
		 */
		if (SEQ_GT(ti->ti_seq+ti->ti_urp, tp->rcv_up)) {
			tp->rcv_up = ti->ti_seq + ti->ti_urp;
			so->so_oobmark = so->so_rcv.sb_cc +
			    (tp->rcv_up - tp->rcv_nxt) - 1;
			if (so->so_oobmark == 0)
				so->so_state |= SS_RCVATMARK;
#ifdef TCPTRUEOOB
			if ((tp->t_flags & TF_DOOOB) == 0)
#endif
				sohasoutofband(so);
			tp->t_oobflags &= ~TCPOOB_HAVEDATA;
		}
		/*
		 * Remove out of band data so doesn't get presented to user.
		 * This can happen independent of advancing the URG pointer,
		 * but if two URG's are pending at once, some out-of-band
		 * data may creep in... ick.
		 */
		if (ti->ti_urp <= ti->ti_len) {
			tcp_pulloutofband(so, ti);
		}
	}

	/*
	 * Process the segment text, merging it into the TCP sequencing queue,
	 * and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting tp->rcv_wnd as data
	 * is presented to the user (this happens in tcp_usrreq.c,
	 * case PRU_RCVD).  If a FIN has already been received on this
	 * connection then we just ignore the text.
	 */
	if ((ti->ti_len || (tiflags&TH_FIN)) &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		tiflags = tcp_reass(tp, ti);
		if (tcpnodelack == 0)
			tp->t_flags |= TF_DELACK;
		else
			tp->t_flags |= TF_ACKNOW;
	} else {
		m_freem(m);
		tiflags &= ~TH_FIN;
	}

	/*
	 * If FIN is received ACK the FIN and let the user know
	 * that the connection is closing.
	 */
	if (tiflags & TH_FIN) {
		if (TCPS_HAVERCVDFIN(tp->t_state) == 0) {
			socantrcvmore(so);
			tp->t_flags |= TF_ACKNOW;
			tp->rcv_nxt++;
		}
		switch (tp->t_state) {

	 	/*
		 * In SYN_RECEIVED and ESTABLISHED STATES
		 * enter the CLOSE_WAIT state.
		 */
		case TCPS_SYN_RECEIVED:
		case TCPS_ESTABLISHED:
			tp->t_state = TCPS_CLOSE_WAIT;
			break;

	 	/*
		 * If still in FIN_WAIT_1 STATE FIN has not been acked so
		 * enter the CLOSING state.
		 */
		case TCPS_FIN_WAIT_1:
			tp->t_state = TCPS_CLOSING;
			break;

	 	/*
		 * In FIN_WAIT_2 state enter the TIME_WAIT state,
		 * starting the time-wait timer, turning off the other 
		 * standard timers.
		 */
		case TCPS_FIN_WAIT_2:
			tp->t_state = TCPS_TIME_WAIT;
			tcp_canceltimers(tp);
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			soisdisconnected(so);
			break;

		/*
		 * In TIME_WAIT state restart the 2 MSL time_wait timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;
		}
	}
	if (so->so_options & SO_DEBUG)
		tcp_trace(TA_INPUT, ostate, tp, &tcp_saveti, 0);

	/*
	 * Return any desired output.
	 */
	(void) tcp_output(tp);
#ifdef SIGH
	intcpinput--;
#endif
	return;

dropafterack:
	/*
	 * Generate an ACK dropping incoming segment if it occupies
	 * sequence space, where the ACK reflects our state.
	 */
	if ((tiflags&TH_RST) ||
	    tlen == 0 && (tiflags&(TH_SYN|TH_FIN)) == 0)
		goto drop;
	if (tp->t_inpcb->inp_socket->so_options & SO_DEBUG)
		tcp_trace(TA_RESPOND, ostate, tp, &tcp_saveti, 0);
	tcp_respond(tp, ti, tp->rcv_nxt, tp->snd_nxt, TH_ACK);
#ifdef SIGH
	intcpinput--;
#endif
	return;

dropwithreset:
	if (om)
		(void) m_free(om);
	/*
	 * Generate a RST, dropping incoming segment.
	 * Make ACK acceptable to originator of segment.
	 */
	if (tiflags & TH_RST)
		goto drop;
	if (tiflags & TH_ACK)
		tcp_respond(tp, ti, (tcp_seq)0, ti->ti_ack, TH_RST);
	else {
		if (tiflags & TH_SYN)
			ti->ti_len++;
		tcp_respond(tp, ti, ti->ti_seq+ti->ti_len, (tcp_seq)0,
		    TH_RST|TH_ACK);
	}
#ifdef SIGH
	intcpinput--;
#endif
	return;

drop:
	/*
	 * Drop space held by incoming segment and return.
	 */
	if (tp && (tp->t_inpcb->inp_socket->so_options & SO_DEBUG))
		tcp_trace(TA_DROP, ostate, tp, &tcp_saveti, 0);
	m_freem(m);
#ifdef SIGH
	intcpinput--;
#endif
	return;
}

tcp_dooptions(tp, om)
	register struct tcpcb *tp;
	register struct mbuf *om;
{
	register u_char *cp;
	register int opt, optlen, cnt;

	MAPSAVE();
	cp = mtod(om, u_char *);
	cnt = om->m_len;
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = UCHAR(cp[0]);
		if (opt == TCPOPT_EOL)
			break;
		if (opt == TCPOPT_NOP)
			optlen = 1;
		else
			optlen = UCHAR(cp[1]);
		switch (opt) {

		default:
			break;

		case TCPOPT_MAXSEG:
			if (optlen != 4)
				continue;
			bcopy((caddr_t)(cp+2), (caddr_t)&tp->t_maxseg, 2);
#ifdef SMALLTCP
			tp->t_maxseg = 256;
#endif
#ifndef WATCHOUT
			tp->t_maxseg = ntohs((u_short)tp->t_maxseg);
#endif

			break;
			
#ifdef TCPTRUEOOB
		case TCPOPT_WILLOOB:
			tp->t_flags |= TF_DOOOB;
printf("tp %x dooob\n", tp);
			break;

		case TCPOPT_OOBDATA: {
			int seq;
			register struct socket *so = tp->t_inpcb->inp_socket;
			tcp_seq mark;

			if (optlen != 8)
				continue;
			seq = UCHAR(cp[2]);
			if (seq < UCHAR(tp->t_iobseq))
				seq += 256;
printf("oobdata cp[2] %d iobseq %d seq %d\n", cp[2], tp->t_iobseq, seq);
			if (seq - UCHAR(tp->t_iobseq) > 128) {
printf("bad seq\n");
				tp->t_oobflags |= TCPOOB_OWEACK;
				break;
			}
			tp->t_iobseq = cp[2];
			tp->t_iobc = cp[3];
			bcopy(cp+4, &mark, sizeof mark);
#ifndef WATCHOUT
			mark = ntohl(mark);
#endif
			so->so_oobmark = so->so_rcv.sb_cc + (mark-tp->rcv_nxt);
			if (so->so_oobmark == 0)
				so->so_state |= SS_RCVATMARK;
printf("take oob data %x input iobseq now %x\n", tp->t_iobc, tp->t_iobseq);
			sohasoutofband(so);
			break;
		}

		case TCPOPT_OOBACK: {
			int seq;

			if (optlen != 4)
				continue;
			if (tp->t_oobseq != cp[2]) {
printf("wrong ack\n");
				break;
			}
printf("take oob ack %x and cancel rexmt\n", cp[2]);
			tp->t_oobflags &= ~TCPOOB_NEEDACK;
			tp->t_timer[TCPT_OOBREXMT] = 0;
			break;
		}
#endif TCPTRUEOOB
		}
	}
	(void) m_free(om);
	MAPREST();
}

/*
 * Pull out of band byte out of a segment so
 * it doesn't appear in the user's data queue.
 * It is still reflected in the segment length for
 * sequencing purposes.
 */
tcp_pulloutofband(so, ti)
	register struct socket *so;
	register struct tcpiphdr *ti;
{
	register struct mbuf *m;
	register int cnt = ti->ti_urp - 1;
	
	MAPSAVE();
	m = dtom(ti);
	while (cnt >= 0) {
		if (m->m_len > cnt) {
			char *cp = mtod(m, caddr_t) + cnt;
			struct tcpcb *tp = sototcpcb(so);

			tp->t_iobc = *cp;
			tp->t_oobflags |= TCPOOB_HAVEDATA;
			bcopy(cp+1, cp, (int)(m->m_len - cnt - 1));
			m->m_len--;
			goto out;
		}
		cnt -= m->m_len;
		m = m->m_next;
		if (m == 0)
			break;
	}
	panic("tcp_pulloutofband");
out:
	MAPREST();
}

#ifdef WATCHOUT 	/* this is for the 11 */
#define ti_mbuf ti_sum
#define DTOM(d) ( (struct mbuf *) ((d)->ti_mbuf) )
#define INSQUE(i,p) { \
	struct tcpiphdr *tii; \
	MSGET(tii, struct tcpiphdr, 0);  if (tii == 0) goto drop; \
	insque(tii,p); \
	tii->ti_len = (i)->ti_len;  tii->ti_seq = (i)->ti_seq; \
	tii->ti_flags = (i)->ti_flags;  tii->ti_mbuf = m0;  i = tii; }
#else
#define DTOM(d) dtom(d)
#define INSQUE(i,p) insque(i,p)
#endif

/*
 * Insert segment ti into reassembly queue of tcp with
 * control block tp.  Return TH_FIN if reassembly now includes
 * a segment with FIN.
 */
tcp_reass(tp, ti)
	register struct tcpcb *tp;
	register struct tcpiphdr *ti;
{
	register struct tcpiphdr *q,*qp;
	register struct socket *so = tp->t_inpcb->inp_socket;
	register struct mbuf *m,*m0;
	register int cnt,flags,empty = 0;

	/*
	 * Call with ti==0 after become established to
	 * force pre-ESTABLISHED data up to user socket.
	 */
	if (ti == 0)
		goto present;
	m0 = dtom(ti);

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = tp->seg_next; q != (struct tcpiphdr *)tp;
	    q = (struct tcpiphdr *)q->ti_next)
		if (SEQ_GT(q->ti_seq, ti->ti_seq))
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if ((struct tcpiphdr *)q->ti_prev != (struct tcpiphdr *)tp) {
		register int i;
		q = (struct tcpiphdr *)q->ti_prev;
		/* conversion to int (in i) handles seq wraparound */
		i = q->ti_seq + q->ti_len - ti->ti_seq;
		if (i > 0) {
			if (i >= ti->ti_len)
				goto drop;
			m_adj(m0, i);
			ti->ti_len -= i;
			ti->ti_seq += i;
		}
		q = (struct tcpiphdr *)(q->ti_next);
	}

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct tcpiphdr *)tp) {
		register int i = (ti->ti_seq + ti->ti_len) - q->ti_seq;
		if (i <= 0)
			break;
		if (i < q->ti_len) {
			q->ti_seq += i;
			q->ti_len -= i;
			m_adj(DTOM(q), i);
			break;
		}
		qp = q;
		q = (struct tcpiphdr *)q->ti_next;
		m = DTOM(qp);
		remque(qp);
#ifdef WATCHOUT
		MSFREE(qp);
#endif
		m_freem(m);
	}

	/*
	 * Stick new segment in its place.  Insque stuff can be expensive,
	 * so avoid if possible.
	 */
	if (tp->seg_next == (struct tcpiphdr *)tp)         /* queue was empty */
		empty++;
	else
		INSQUE(ti, q->ti_prev);

present:
	/*
	 * Present data to user, advancing rcv_nxt through
	 * completed sequence space.
	 */
	if (TCPS_HAVERCVDSYN(tp->t_state) == 0)
		goto out;
	if (!empty) ti = tp->seg_next;
	if (ti == (struct tcpiphdr *)tp || ti->ti_seq != tp->rcv_nxt)
		goto out;
	if (tp->t_state == TCPS_SYN_RECEIVED && ti->ti_len)
		goto out;
	if (empty) {
		tp->rcv_nxt += ti->ti_len;
		flags = ti->ti_flags & TH_FIN;
		if (so->so_state & SS_CANTRCVMORE)
			m_freem(m0);
		else
			sbappend(&so->so_rcv, m0);

		sorwakeup(so);
		return (flags);
	}
	do {
		tp->rcv_nxt += ti->ti_len;
		flags = ti->ti_flags & TH_FIN;
		remque(ti);
		m = DTOM(ti);
		qp = ti;
		ti = (struct tcpiphdr *)ti->ti_next;
#ifdef WATCHOUT
		MSFREE(qp);
#endif
		if (so->so_state & SS_CANTRCVMORE)
			m_freem(m);
		else
			sbappend(&so->so_rcv, m);
	} while (ti != (struct tcpiphdr *)tp && ti->ti_seq == tp->rcv_nxt);
	sorwakeup(so);
	return (flags);
drop:
	m_freem(m0);
	return (0);
out:
	if (empty) INSQUE(ti, tp);
	/*
	 * If there are more than 10 tcp segments already queued,
	 * drop the oldest one from the queue.  This sometimes happens
	 * when we get flooded with packets from unbuffered writes and
	 * there are gaps in the sequence numbers.  (Why are gaps so likely
	 * with unbuffered writes?)  A retransmit (covering all the
	 * one byte segments queued) will occur in a moment.
	 */
	cnt = 0;
	for (q = tp->seg_next; q != (struct tcpiphdr *)tp;
	    q = (struct tcpiphdr *)q->ti_next)
		cnt++;
	if (cnt > 10) {
		q = tp->seg_next;
		m = DTOM(q);
		remque(q);
#ifdef WATCHOUT
		MSFREE(q);
#endif
		m_freem(m);
	}
	return (0);
}
