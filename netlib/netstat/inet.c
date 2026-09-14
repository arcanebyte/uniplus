#ifndef lint
static char sccsid[] = "@(#)inet.c	4.4 82/10/07";
#endif

/* Lisa: the kernel's headers, from /usr/src/include; see main.c. */
#include <sys/types.h>
#include "net/misc.h"
#define	U_LONG_DEFINED		/* net/misc.h has u_long and friends */
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/route.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/in_pcb.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "net/tcp.h"
#include "net/tcpip.h"
#include "net/tcp_seq.h"
#define TCPSTATES
#include "net/tcp_fsm.h"
#include "net/tcp_timer.h"
#include "net/tcp_var.h"
#include "net/udp.h"
#include "net/udp_var.h"
#include "netdb.h"

struct	inpcb inpcb;
struct	tcpcb tcpcb;
struct	socket socket;
struct	protosw proto;
extern	int kmem;
extern	int Aflag;
extern	int aflag;
extern	int nflag;

static	int first = 1;
char	*inetname();

/*
 * Print a summary of connections related to an Internet
 * protocol.  For TCP, also give state of connection.
 * Listening processes (aflag) are suppressed unless the
 * -a (all) flag is specified.
 */
protopr(off, name)
	off_t off;
	char *name;
{
	struct inpcb cb;
	register struct inpcb *prev, *next;
	int istcp;

	if (off == 0) {
		printf("%s control block: symbol not in namelist\n", name);
		return;
	}
	istcp = strcmp(name, "tcp") == 0;
	klseek(kmem, (off_t)off, 0);
	read(kmem, &cb, sizeof (struct inpcb));
	inpcb = cb;
	prev = (struct inpcb *)off;
	if (first) {
		printf("Active connections");
		if (aflag)
			printf(" (including servers)");
		putchar('\n');
		if (Aflag)
			printf("%-8.8s ", "PCB");
		printf("%-5.5s %-6.6s %-6.6s  %-18.18s %-18.18s %s\n",
			"Proto", "Recv-Q", "Send-Q",
			"Local Address", "Foreign Address", "(state)");
		first = 0;
	}
	while (inpcb.inp_next != (struct inpcb *)off) {
		char *cp;

		next = inpcb.inp_next;
		klseek(kmem, (off_t)next, 0);
		read(kmem, &inpcb, sizeof (inpcb));
		if (inpcb.inp_prev != prev) {
			printf("???\n");
			break;
		}
		if (!aflag &&
		  inet_lnaof(inpcb.inp_laddr.s_addr) == INADDR_ANY) {
			prev = next;
			continue;
		}
		klseek(kmem, (off_t)inpcb.inp_socket, 0);
		read(kmem, &socket, sizeof (socket));
		if (istcp) {
			klseek(kmem, (off_t)inpcb.inp_ppcb, 0);
			read(kmem, &tcpcb, sizeof (tcpcb));
		}
		if (Aflag)
			printf("%8x ", inpcb.inp_ppcb);
		printf("%-5.5s %6d %6d ", name, socket.so_rcv.sb_cc,
			socket.so_snd.sb_cc);
		inetprint(&inpcb.inp_laddr, inpcb.inp_lport, name);
		inetprint(&inpcb.inp_faddr, inpcb.inp_fport, name);
		if (istcp) {
			if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
				printf(" %d", tcpcb.t_state);
			else
				printf(" %s", tcpstates[tcpcb.t_state]);
		}
		putchar('\n');
		prev = next;
	}
}

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */
inetprint(in, port, proto)
	register struct in_addr *in;
	int port;
	char *proto;
{
	struct servent *sp = 0;
	char line[80], *cp, *index();

	sprintf(line, "%.10s.", inetname(*in));
	cp = index(line, '\0');
	if (!nflag && port)
		sp = getservbyport(port, proto);
	if (sp || port == 0)
		sprintf(cp, "%.8s", sp ? sp->s_name : "*");
	else
		sprintf(cp, "%d", port);
	printf(" %-18.18s", line);
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give 
 * numeric value, otherwise try for symbolic name.
 */
char *
inetname(in)
	struct in_addr in;
{
	char *cp = 0;
	static char line[50];
	u_long lna,net;

	net = inet_netof(in);
	lna = inet_lnaof(in);

	if (!nflag) {
		if (lna == INADDR_ANY) {
			struct netent *np;

			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		} else {
			struct hostent *hp;

			hp = gethostbyaddr(&in, sizeof (struct in_addr),
				AF_INET);
			if (hp)
				cp = hp->h_name;
		}
	}
	if (in.s_addr == INADDR_ANY)
		strcpy(line, "*");
	else if (cp)
		strcpy(line, cp);
	else {
		char *ucp = (char *)&in;	/* Lisa: u_char is signed here */
		sprintf(line, "%d.%d.%d.%d", ucp[0] & 0xff, ucp[1] & 0xff,
			ucp[2] & 0xff, ucp[3] & 0xff);
	}
	return (line);
}
