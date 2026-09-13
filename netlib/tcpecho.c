/*
 * tcpecho.c -- minimal TCP echo server for UniPlus+ running unix.net
 *
 * usage: tcpecho port
 *
 * 4.1a BSD sockets: the local address goes to socket(), SO_ACCEPTCONN
 * makes it passive, and accept() connects that same socket.  So each
 * connection gets a fresh listening socket, served one at a time.
 * Everything received is written back until the peer closes.
 */

#include <stdio.h>
#include "net/socket.h"
#include "net/in.h"

fail(what)
char *what;
{
	perror(what);
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in sin, from;
	register char *p;
	register int i;
	int s, n, port;
	char buf[512];

	/* no pipe characters in this file: LisaEm's keyboard turns them into '?' when pasted */
	port = (argc == 2) ? atoi(argv[1]) : 0;
	if (port > 65535)
		port = 0;
	if (port <= 0) {
		fprintf(stderr, "usage: %s port\n", argv[0]);
		exit(2);
	}

	for (;;) {
		p = (char *)&sin;
		for (i = 0; i < sizeof (sin); i++)
			*p++ = 0;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);	/* any local address */

		s = socket(SOCK_STREAM, (struct sockproto *)0,
		    (struct sockaddr *)&sin, SO_ACCEPTCONN);
		if (s < 0)
			fail("socket");
		printf("tcpecho: listening on port %d\n", port);
		fflush(stdout);

		if (accept(s, (struct sockaddr *)&from) < 0)
			fail("accept");
		printf("tcpecho: connection from %d.%d.%d.%d port %d\n",
		    (int)((from.sin_addr.s_addr >> 24) & 0xff),
		    (int)((from.sin_addr.s_addr >> 16) & 0xff),
		    (int)((from.sin_addr.s_addr >> 8) & 0xff),
		    (int)(from.sin_addr.s_addr & 0xff),
		    (int)ntohs(from.sin_port));
		fflush(stdout);

		while ((n = read(s, buf, sizeof (buf))) > 0)
			if (write(s, buf, n) != n) {
				perror("write");
				break;
			}
		if (n < 0)
			perror("read");
		close(s);
	}
}
