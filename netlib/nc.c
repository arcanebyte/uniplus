/*
 * nc.c -- a small netcat for UniPlus+ running unix.net
 *
 * usage: nc [-v] [-c] [-q] [-w secs] a.b.c.d port	connect (TCP)
 *        nc [-v] [-c] [-q] -l port			listen for one connection (TCP)
 *        nc [-v] -u a.b.c.d port			send datagrams (UDP)
 *        nc [-v] -u -l port				print datagrams received (UDP)
 *
 * Copies standard input to the connection and the connection to standard
 * output.
 *   -v  report connections and datagram senders on standard error
 *   -c  send each newline from standard input as CR LF (HTTP, SMTP, ...)
 *   -q  quit as soon as standard input ends; otherwise TCP keeps printing
 *       until the other side closes (there is no shutdown() to tell it)
 *   -w  give up connecting after secs seconds
 *   -u  UDP: each read from standard input is one datagram; replies are
 *       printed.  With -l, datagrams are only printed.
 *
 * The 4.1a socket API has no bind() or listen(): the local address goes to
 * socket(), SO_ACCEPTCONN makes it listen, and accept() connects that same
 * socket.  Standard input and the socket are served by two processes, as
 * select() here doesn't cover terminals: the parent copies the socket to
 * standard output, the child copies standard input to the socket.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "net/socket.h"
#include "net/in.h"

#define BUFLEN	1024

extern int errno;

int verbose, crlf, quit, udp, lflag, wsecs;
int child;

usage()
{
	fprintf(stderr, "usage: nc [-v] [-c] [-q] [-w secs] a.b.c.d port\n");
	fprintf(stderr, "       nc [-v] [-c] [-q] -l port\n");
	fprintf(stderr, "       nc [-v] -u a.b.c.d port\n");
	fprintf(stderr, "       nc [-v] -u -l port\n");
	exit(2);
}

fail(what)
char *what;
{
	perror(what);
	if (child > 0)
		kill(child, SIGTERM);
	exit(1);
}

/* dotted quad to host-order (== network-order on 68000) address */
parseaddr(s, ap)
register char *s;
unsigned long *ap;
{
	unsigned long a = 0;
	register int i, n;

	for (i = 0; i < 4; i++) {
		if (*s < '0' || *s > '9')
			return (-1);
		n = 0;
		while (*s >= '0' && *s <= '9') {
			n = n * 10 + (*s++ - '0');
			if (n > 255)
				return (-1);
		}
		a = (a << 8) | n;
		if (i < 3 && *s++ != '.')
			return (-1);
	}
	if (*s != '\0')
		return (-1);
	*ap = a;
	return (0);
}

setaddr(sin, a, port)
struct sockaddr_in *sin;
unsigned long a;
int port;
{
	register char *p;
	register int i;

	p = (char *)sin;
	for (i = 0; i < sizeof (*sin); i++)
		*p++ = 0;
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	sin->sin_addr.s_addr = htonl(a);
}

prtaddr(what, sin)
char *what;
struct sockaddr_in *sin;
{
	unsigned long a = ntohl(sin->sin_addr.s_addr);

	fprintf(stderr, "nc: %s %d.%d.%d.%d port %d\n", what,
	    (int)((a >> 24) & 0xff), (int)((a >> 16) & 0xff),
	    (int)((a >> 8) & 0xff), (int)(a & 0xff), (int)ntohs(sin->sin_port));
}

ontimer()
{
	fprintf(stderr, "nc: timed out\n");
	exit(1);
}

/* write all of buf, turning LF into CR LF if -c; returns -1 on error */
putall(fd, buf, n, conv)
int fd, n, conv;
char *buf;
{
	char out[2 * BUFLEN];
	register char *p, *q;
	register int i;
	int len;

	if (conv) {
		for (i = 0, p = buf, q = out; i < n; i++) {
			if (*p == '\n')
				*q++ = '\r';
			*q++ = *p++;
		}
		buf = out;
		n = q - out;
	}
	while (n > 0) {
		if ((len = write(fd, buf, n)) <= 0)
			return (-1);
		buf += len;
		n -= len;
	}
	return (0);
}

/* child: standard input to the socket, then exit */
tosock(s, to)
int s;
struct sockaddr_in *to;
{
	char buf[BUFLEN];
	int n;

	while ((n = read(0, buf, BUFLEN)) > 0) {
		if (udp) {
			if (send(s, (struct sockaddr *)to, buf, n) != n) {
				perror("nc: send");
				break;
			}
		} else if (putall(s, buf, n, crlf) < 0) {
			perror("nc: write");
			break;
		}
	}
	if (quit)
		kill(getppid(), SIGTERM);
	exit(0);
}

/* parent: the socket to standard output until it closes */
fromsock(s)
int s;
{
	struct sockaddr_in from;
	char buf[BUFLEN];
	int n;

	for (;;) {
		if (udp) {
			setaddr(&from, 0L, 0);
			n = receive(s, (struct sockaddr *)&from, buf, BUFLEN);
			if (n > 0 && verbose)
				prtaddr("datagram from", &from);
		} else
			n = read(s, buf, BUFLEN);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			perror(udp ? "nc: receive" : "nc: read");
			break;
		}
		if (n == 0 && !udp)
			break;
		if (putall(1, buf, n, 0) < 0)
			break;
	}
	if (verbose && !udp)
		fprintf(stderr, "nc: connection closed\n");
}

main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in sin, peer;
	unsigned long addr;
	int s, port, type;

	while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
		if (strcmp(argv[1], "-v") == 0)
			verbose = 1;
		else if (strcmp(argv[1], "-c") == 0)
			crlf = 1;
		else if (strcmp(argv[1], "-q") == 0)
			quit = 1;
		else if (strcmp(argv[1], "-u") == 0)
			udp = 1;
		else if (strcmp(argv[1], "-l") == 0)
			lflag = 1;
		else if (strcmp(argv[1], "-w") == 0 && argc > 2 &&
		    (wsecs = atoi(argv[2])) > 0) {
			argv++;
			argc--;
		} else
			usage();
		argv++;
		argc--;
	}
	if (argc != (lflag ? 2 : 3))
		usage();
	if (!lflag && parseaddr(argv[1], &addr) < 0)
		usage();
	port = atoi(argv[argc - 1]);
	if (port <= 0 || port > 65535)
		usage();
	type = udp ? SOCK_DGRAM : SOCK_STREAM;

	if (lflag) {
		setaddr(&sin, 0L, port);	/* any local address */
		s = socket(type, (struct sockproto *)0, (struct sockaddr *)&sin,
		    udp ? 0 : SO_ACCEPTCONN);
		if (s < 0)
			fail("nc: socket");
		if (verbose)
			fprintf(stderr, "nc: listening on %s port %d\n",
			    udp ? "UDP" : "TCP", port);
		if (!udp) {
			setaddr(&peer, 0L, 0);
			if (accept(s, (struct sockaddr *)&peer) < 0)
				fail("nc: accept");
			if (verbose)
				prtaddr("connection from", &peer);
		}
	} else {
		setaddr(&peer, addr, port);
		s = socket(type, (struct sockproto *)0, (struct sockaddr *)0, 0);
		if (s < 0)
			fail("nc: socket");
		if (!udp) {
			if (wsecs) {
				signal(SIGALRM, ontimer);
				alarm(wsecs);
			}
			if (connect(s, (struct sockaddr *)&peer) < 0)
				fail("nc: connect");
			alarm(0);
			if (verbose)
				prtaddr("connected to", &peer);
		}
	}

	/* a UDP listener only prints; everything else copies both ways */
	if (!(udp && lflag)) {
		child = fork();
		if (child < 0)
			fail("nc: fork");
		if (child == 0)
			tosock(s, &peer);
	}
	fromsock(s);
	if (child > 0)
		kill(child, SIGTERM);
	close(s);
	exit(0);
}
