/*	tcp_debug.c	4.4	82/06/26	*/

#include "param.h"
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#define PRUREQUESTS
#include <sys/protosw.h>
#include "../net/in.h"
#include "../net/route.h"
#include "../net/in_pcb.h"
#include "../net/in_systm.h"
#include "../net/if.h"
#include "../net/ip.h"
#include "../net/ip_var.h"
#include "../net/tcp.h"
#define TCPSTATES
#include "../net/tcp_fsm.h"
#include "../net/tcp_seq.h"
#define	TCPTIMERS
#include "../net/tcp_timer.h"
#include "../net/tcp_var.h"
#include "../net/tcpip.h"
#define	TANAMES
#include "../net/tcp_debug.h"
#include <errno.h>

int     tcpconsdebug = 0;
/*
 * Tcp debug routines
 */
tcp_trace(act, ostate, tp, ti, req)
	short act, ostate;
	struct tcpcb *tp;
	struct tcpiphdr *ti;
	int req;
{
	tcp_seq seq, ack;
	int len, flags;
	struct tcp_debug *td = &tcp_debug[tcp_debx++];

	if (tcp_debx == TCP_NDEBUG)
		tcp_debx = 0;
	td->td_time = iptime();
	td->td_act = act;
	td->td_ostate = ostate;
	td->td_tcb = (caddr_t)tp;
	if (tp)
		td->td_cb = *tp;
	else
		bzero((caddr_t)&td->td_cb, sizeof (*tp));
	if (ti)
		td->td_ti = *ti;
	else
		bzero((caddr_t)&td->td_ti, sizeof (*ti));
	td->td_req = req;
	if (tcpconsdebug == 0)
		return;
	if (tp)
		printf("%x %s:", tp, tcpstates[ostate]);
	else
		printf("???????? ");
	printf("%s ", tanames[act]);
	switch (act) {

	case TA_INPUT:
	case TA_OUTPUT:
		seq = ti->ti_seq;
		ack = ti->ti_ack;
		len = ti->ti_len;
#if vax || pdp11
		if (act == TA_OUTPUT) {
			seq = ntohl(seq);
			ack = ntohl(ack);
			len = ntohs((u_short)len);
		}
#endif
		if (act == TA_OUTPUT)
			len -= sizeof (struct tcphdr);
		if (len)
			printf("[%X..%X)", seq, seq+len);
		else
			printf("%X", seq);
		printf("@%X", ack);
		flags = ti->ti_flags;
		if (flags) {
#ifndef lint
			char *cp = "<";
#define pf(f) { if (ti->ti_flags&TH_/**/f) { printf("%s%s", cp, "f"); cp = ","; } }
			pf(SYN); pf(ACK); pf(FIN); pf(RST);
#endif
			printf(">");
		}
		break;

	case TA_USER:
		printf("%s", prurequests[req&0xff]);
		if ((req & 0xff) == PRU_SLOWTIMO)
			printf("<%s>", tcptimers[req>>8]);
		break;
	}
	if (tp)
		printf(" -> %s", tcpstates[tp->t_state]);
	/* print out internal state of tp !?! */
	printf("\n");
	if (tp == 0)
		return;
	printf("\trcv_(nxt,wnd) (%X,%x) snd_(una,nxt,max) (%X,%X,%X)\n",
	    tp->rcv_nxt, tp->rcv_wnd, tp->snd_una, tp->snd_nxt, tp->snd_max);
	printf("\tsnd_(wl1,wl2,wnd) (%X,%X,%x)\n",
	    tp->snd_wl1, tp->snd_wl2, tp->snd_wnd);
}
