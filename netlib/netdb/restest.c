/*
 * restest.c -- show each step of a name server lookup, to find where
 * one fails.
 *
 * usage: restest [name]		(default www.tuhs.org)
 *
 * Reads /etc/resolv.conf with res_init(), makes the query with
 * res_mkquery(), then sends it to the first name server by hand
 * (socket, send, select for up to 10 seconds, receive) and prints what
 * each call returns and the answer's header.  select() is tried with
 * s + 1, then s + 2, to show whether the kernel has the selscan() fix.  Then does the same lookup
 * with res_send() and with gethostbyname().
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include "net/socket.h"
#include "net/in.h"
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>

extern int errno;
long time();

prsin(sp)
struct sockaddr_in *sp;
{
	printf("%s port %d", inet_ntoa(sp->sin_addr), (int)sp->sin_port);
}

dump(p, n)
char *p;
int n;
{
	int i;

	for (i = 0; i < n; i++) {
		printf(" %02x", p[i] & 0xff);
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n");
}

header(p, n)
char *p;
int n;
{
	HEADER *hp = (HEADER *)p;

	if (n < 12) {
		printf("  too short for a header\n");
		return;
	}
	printf("  id %d, bytes 2-3 %02x %02x: qr %d opcode %d aa %d tc %d rd %d ra %d rcode %d\n",
	    (int)getshort(p), p[2] & 0xff, p[3] & 0xff, hp->qr, hp->opcode,
	    hp->aa, hp->tc, hp->rd, hp->ra, hp->rcode);
	printf("  qdcount %d ancount %d nscount %d arcount %d\n",
	    (int)getshort(p + 4), (int)getshort(p + 6), (int)getshort(p + 8),
	    (int)getshort(p + 10));
}

main(argc, argv)
int argc;
char *argv[];
{
	char *name;
	char query[PACKETSZ], answer[PACKETSZ];
	struct sockaddr_in from;
	struct hostent *hp;
	char **ap;
	struct in_addr a;
	long mask, t;
	int i, n, qlen, s;

	name = argc > 1 ? argv[1] : "www.tuhs.org";
	setbuf(stdout, (char *)0);

	printf("res_init: %d\n", res_init());
	printf("  nscount %d, retrans %d, retry %d, options 0x%lx, domain \"%s\"\n",
	    _res.nscount, _res.retrans, _res.retry, _res.options, _res.defdname);
	for (i = 0; i < _res.nscount; i++) {
		printf("  server %d: family %d, ", i, _res.nsaddr_list[i].sin_family);
		prsin(&_res.nsaddr_list[i]);
		printf("\n");
	}

	qlen = res_mkquery(QUERY, name, C_IN, T_A, (char *)0, 0,
	    (struct rrec *)0, query, sizeof (query));
	printf("res_mkquery %s: %d bytes\n", name, qlen);
	if (qlen < 0)
		exit(1);
	dump(query, qlen);
	header(query, qlen);
	if (_res.nscount < 1) {
		printf("no name server in /etc/resolv.conf\n");
		exit(1);
	}

	s = socket(SOCK_DGRAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	printf("socket: %d", s);
	if (s < 0)
		printf(", errno %d", errno);
	printf("\n");
	errno = 0;
	n = send(s, (struct sockaddr *)&_res.nsaddr_list[0], query, qlen);
	printf("send to ");
	prsin(&_res.nsaddr_list[0]);
	printf(": %d, errno %d\n", n, errno);

	/*
	 * select(s + 1) fails on kernels without the selscan() fix
	 * (res_send.c); then try s + 2.
	 */
	t = time((long *)0);
	mask = 1L << s;
	errno = 0;
	n = select(s + 1, &mask, (long *)0, 10000L);
	printf("select(s + 1): %d, mask 0x%lx, errno %d, after %ld s\n", n,
	    mask, errno, time((long *)0) - t);
	if (n == 0) {
		mask = 1L << s;
		n = select(s + 2, &mask, (long *)0, 1000L);
		printf("select(s + 2): %d, mask 0x%lx, errno %d\n", n, mask,
		    errno);
	}
	if (n > 0) {
		for (i = 0; i < sizeof (from); i++)
			((char *)&from)[i] = 0;
		errno = 0;
		n = receive(s, (struct sockaddr *)&from, answer, sizeof (answer));
		printf("receive: %d, errno %d, from ", n, errno);
		prsin(&from);
		printf("\n");
		if (n > 0) {
			dump(answer, n);
			header(answer, n);
		}
	}
	close(s);

	t = time((long *)0);
	errno = 0;
	n = res_send(query, qlen, answer, sizeof (answer));
	printf("res_send: %d, errno %d, after %ld s\n", n, errno,
	    time((long *)0) - t);
	if (n > 0)
		header(answer, n);

	t = time((long *)0);
	h_errno = 0;
	hp = gethostbyname(name);
	printf("gethostbyname: %s, h_errno %d, after %ld s\n",
	    hp ? "found" : "NULL", h_errno, time((long *)0) - t);
	if (hp) {
		printf("  %s", hp->h_name);
		for (ap = hp->h_aliases; *ap; ap++)
			printf(" %s", *ap);
		for (ap = hp->h_addr_list; *ap; ap++) {
			bcopy(*ap, (char *)&a, sizeof (a));
			printf(" %s", inet_ntoa(a));
		}
		printf("\n");
	}
	exit(0);
}
