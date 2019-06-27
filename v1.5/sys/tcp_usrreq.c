/*	tcp_usrreq.c	1.59	82/06/20	*/

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
#include "net/tcp.h"
#include "net/tcp_fsm.h"
#include "net/tcp_seq.h"
#include "net/tcp_timer.h"
#include "net/tcp_var.h"
#include "net/tcpip.h"
#include "net/tcp_debug.h"
#include "errno.h"

/*
 * TCP protocol interface to socket abstraction.
 */
extern	char *tcpstates[];
struct	tcpcb *tcp_newtcpcb();

/*
 * Process a TCP user request for TCP tb.  If this is a send request
 * then m is the mbuf chain of send data.  If this is a timer expiration
 * (called from the software clock routine), then timertype tells which timer.
 */
tcp_usrreq(so, req, m, addr)
	struct socket *so;
	int req;
	struct mbuf *m;
	caddr_t addr;
{
	register struct inpcb *inp = sotoinpcb(so);
	register struct tcpcb *tp;
	int s = splnet();
	int error = 0;
	int ostate = 0;
	extern struct tcpcb * tcp_disconnect();
	extern struct tcpcb * tcp_usrclosed();

	/*
	 * When a TCP is attached to a socket, then there will be
	 * a (struct inpcb) pointed at by the socket, and this
	 * structure will point at a subsidary (struct tcpcb).
	 * The normal sequence of events is:
	 *	PRU_ATTACH		creating these structures
	 *	PRU_CONNECT		connecting to a remote peer
	 *	(PRU_SEND|PRU_RCVD)*	exchanging data
	 *	PRU_DISCONNECT		disconnecting from remote peer
	 *	PRU_DETACH		deleting the structures
	 * With the operations from PRU_CONNECT through PRU_DISCONNECT
	 * possible repeated several times.
	 *
	 * MULTIPLE CONNECTS ARE NOT YET IMPLEMENTED.
	 */
	if (inp == 0 && req != PRU_ATTACH) {
		splx(s);
		return (EINVAL);		/* XXX */
	}
	if (inp) {
		tp = intotcpcb(inp);
#ifdef KPROF
		tcp_acounts[tp->t_state][req]++;
#endif
		ostate = tp->t_state;
	}
	switch (req) {

	/*
	 * TCP attaches to socket via PRU_ATTACH, reserving space,
	 * and internet and TCP control blocks.
	 * If the socket is to receive connections,
	 * then the LISTEN state is entered.
	 */
	case PRU_ATTACH:
		if (inp) {
			error = EISCONN;
			break;
		}
		error = tcp_attach(so, (struct sockaddr *)addr);
		if (error)
			break;
		if ((so->so_options & SO_DONTLINGER) == 0)
			so->so_linger = TCP_LINGERTIME;
		tp = sototcpcb(so);
		break;

	/*
	 * PRU_DETACH detaches the TCP protocol from the socket.
	 * If the protocol state is non-embryonic, then can't
	 * do this directly: have to initiate a PRU_DISCONNECT,
	 * which may finish later; embryonic TCB's can just
	 * be discarded here.
	 */
	case PRU_DETACH:
		tp = tcp_disconnect(tp);
		break;

	/*
	 * Initiate connection to peer.
	 * Create a template for use in transmissions on this connection.
	 * Enter SYN_SENT state, and mark socket as connecting.
	 * Start keep-alive timer, and seed output sequence space.
	 * Send initial segment on connection.
	 */
	case PRU_CONNECT:
		error = in_pcbconnect(inp, (struct sockaddr_in *)addr);
		if (error)
			break;
		tp->t_template = tcp_template(tp);
		if (tp->t_template == 0) {
			in_pcbdisconnect(inp);
			error = ENOBUFS;
			break;
		}
		soisconnecting(so);
		tp->t_state = TCPS_SYN_SENT;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;
		tp->iss = tcp_iss; tcp_iss += TCP_ISSINCR/2;
		tcp_sendseqinit(tp);
		error = tcp_output(tp);
		break;

	/*
	 * Initiate disconnect from peer.
	 * If connection never passed embryonic stage, just drop;
	 * else if don't need to let data drain, then can just drop anyways,
	 * else have to begin TCP shutdown process: mark socket disconnecting,
	 * drain unread data, state switch to reflect user close, and
	 * send segment (e.g. FIN) to peer.  Socket will be really disconnected
	 * when peer sends FIN and acks ours.
	 *
	 * SHOULD IMPLEMENT LATER PRU_CONNECT VIA REALLOC TCPCB.
	 */
	case PRU_DISCONNECT:
		tp = tcp_disconnect(tp);
		break;

	/*
	 * Accept a connection.  Essentially all the work is
	 * done at higher levels; just return the address
	 * of the peer, storing through addr.
	 */
	case PRU_ACCEPT: {
		struct sockaddr_in *sin = (struct sockaddr_in *)addr;

		if (sin) {
			bzero((caddr_t)sin, sizeof (*sin));
			sin->sin_family = AF_INET;
			sin->sin_port = inp->inp_fport;
			sin->sin_addr = inp->inp_faddr;
		}
		}
		break;

	/*
	 * Mark the connection as being incapable of further output.
	 */
	case PRU_SHUTDOWN:
		socantsendmore(so);
		if (tp = tcp_usrclosed(tp))
			error = tcp_output(tp);
		break;

	/*
	 * After a receive, possibly send window update to peer.
	 */
	case PRU_RCVD:
		(void) tcp_output(tp);
		break;

	/*
	 * Do a send by putting data in output queue and updating urgent
	 * marker if URG set.  Possibly send more data.
	 */
	case PRU_SEND:
		sbappend(&so->so_snd, m);
#ifdef notdef
		if (tp->t_flags & TF_PUSH)
			tp->snd_end = tp->snd_una + so->so_snd.sb_cc;
#endif
		error = tcp_output(tp);
		break;

	/*
	 * Abort the TCP.
	 */
	case PRU_ABORT:
		tcp_drop(tp, ECONNABORTED);
		tp = 0;
		break;

/* SOME AS YET UNIMPLEMENTED HOOKS */
	case PRU_CONTROL:
		error = EOPNOTSUPP;
		break;

	case PRU_SENSE:
		error = EOPNOTSUPP;
		break;
/* END UNIMPLEMENTED HOOKS */

	case PRU_RCVOOB:
		if (so->so_oobmark == 0 &&
		    (so->so_state & SS_RCVATMARK) == 0) {
			error = EINVAL;
			break;
		}
		if ((tp->t_oobflags & TCPOOB_HAVEDATA) == 0) {
			error = EWOULDBLOCK;
			break;
		}
		*mtod(m, caddr_t) = tp->t_iobc;
		break;

	case PRU_SENDOOB:
#ifdef TCPTRUEOOB
		if (tp->t_flags & TF_DOOOB) {
			tp->t_oobseq++;
			tp->t_oobc = *mtod(m, caddr_t);
			tp->t_oobmark = tp->snd_una + so->so_snd.sb_cc;
printf("sendoob seq now %x oobc %x\n", tp->t_oobseq, tp->t_oobc);
			tp->t_oobflags |= TCPOOB_NEEDACK;
			/* what to do ...? */
			if (error = tcp_output(tp))
				break;
		}
#endif
		if (sbspace(&so->so_snd) < -512) {
			error = ENOBUFS;
			break;
		}
		tp->snd_up = tp->snd_una + so->so_snd.sb_cc + 1;
		sbappend(&so->so_snd, m);
#ifdef notdef
		if (tp->t_flags & TF_PUSH)
			tp->snd_end = tp->snd_una + so->so_snd.sb_cc;
#endif
		tp->t_force = 1;
		error = tcp_output(tp);
		tp->t_force = 0;
		break;

	case PRU_SOCKADDR:
		in_setsockaddr((struct sockaddr_in *)addr, inp);
		break;

	/*
	 * TCP slow timer went off; going through this
	 * routine for tracing's sake.
	 */
	case PRU_SLOWTIMO:
		tcp_timers(tp, (int)addr);
		req |= (int)addr << 8;		/* for debug's sake */
		break;

	default:
		panic("tcp_usrreq");
	}
	if (so->so_options & SO_DEBUG)
		tcp_trace(TA_USER, ostate, tp, (struct tcpiphdr *)0, req);
	splx(s);
	return (error);
}

int	tcp_sendspace = 1024*2;
int	tcp_recvspace = 1024*2;
/*
 * Attach TCP protocol to socket, allocating
 * internet protocol control block, tcp control block,
 * bufer space, and entering LISTEN state if to accept connections.
 */
tcp_attach(so, sa)
	struct socket *so;
	struct sockaddr *sa;
{
	register struct tcpcb *tp;
	struct inpcb *inp;
	int error;

	error = in_pcbattach(so, &tcb,
	    tcp_sendspace, tcp_recvspace, (struct sockaddr_in *)sa);
	if (error)
		return (error);
	inp = (struct inpcb *)so->so_pcb;
	tp = tcp_newtcpcb(inp);
	if (tp == 0) {
		in_pcbdetach(inp);
		return (ENOBUFS);
	}
	if (so->so_options & SO_ACCEPTCONN)
		tp->t_state = TCPS_LISTEN;
	else
		tp->t_state = TCPS_CLOSED;
	return (0);
}

/*
 * Initiate (or continue) disconnect.
 * If embryonic state, just send reset (once).
 * If not in ``let data drain'' option, just drop.
 * Otherwise (hard), mark socket disconnecting and drop
 * current input data; switch states based on user close, and
 * send segment to peer (with FIN).
 */
struct tcpcb *
tcp_disconnect(tp)
	struct tcpcb *tp;
{
	struct socket *so = tp->t_inpcb->inp_socket;

	if (tp->t_state <= TCPS_LISTEN) {
		tcp_close(tp);
		tp = 0;
	} else if (so->so_linger == 0) {
		tcp_drop(tp, 0);
		tp = 0;
	} else {
		soisdisconnecting(so);
		sbflush(&so->so_rcv);
		if (tp = tcp_usrclosed(tp))
			(void) tcp_output(tp);
	}
	return (tp);
}

/*
 * User issued close, and wish to trail through shutdown states:
 * if never received SYN, just forget it.  If got a SYN from peer,
 * but haven't sent FIN, then go to FIN_WAIT_1 state to send peer a FIN.
 * If already got a FIN from peer, then almost done; go to LAST_ACK
 * state.  In all other cases, have already sent FIN to peer (e.g.
 * after PRU_SHUTDOWN), and just have to play tedious game waiting
 * for peer to send FIN or not respond to keep-alives, etc.
 * We can let the user exit from the close as soon as the FIN is acked.
 */
struct tcpcb *
tcp_usrclosed(tp)
	struct tcpcb *tp;
{

	switch (tp->t_state) {

	case TCPS_LISTEN:
	case TCPS_SYN_SENT:
		tp->t_state = TCPS_CLOSED;
		tcp_close(tp);
		tp = 0;
		break;

	case TCPS_SYN_RECEIVED:
	case TCPS_ESTABLISHED:
		tp->t_state = TCPS_FIN_WAIT_1;
		break;

	case TCPS_CLOSE_WAIT:
		tp->t_state = TCPS_LAST_ACK;
		break;
	}
	if (tp && tp->t_state >= TCPS_FIN_WAIT_2)
		soisdisconnected(tp->t_inpcb->inp_socket);
	return (tp);
}
