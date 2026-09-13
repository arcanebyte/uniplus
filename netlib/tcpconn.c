/*
 * tcpconn.c -- minimal TCP client for UniPlus+ running unix.net
 *
 * usage: tcpconn a.b.c.d port [message]
 *
 * Connects, sends message followed by CR LF if one is given, then
 * copies everything received to stdout until the peer closes.
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

main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in sin;
	unsigned long addr;
	register char *p;
	register int i;
	int s, n, port;
	char buf[512];

	if (argc < 3 || parseaddr(argv[1], &addr) < 0 ||
	    (port = atoi(argv[2])) <= 0 || port > 65535) {
		fprintf(stderr, "usage: %s a.b.c.d port [message]\n", argv[0]);
		exit(2);
	}

	/* no local address: the kernel assigns an ephemeral port */
	s = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	if (s < 0)
		fail("socket");

	p = (char *)&sin;
	for (i = 0; i < sizeof (sin); i++)
		*p++ = 0;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(addr);
	if (connect(s, (struct sockaddr *)&sin) < 0)
		fail("connect");

	if (argc > 3) {
		for (n = 0; argv[3][n]; n++)
			;
		if (write(s, argv[3], n) != n || write(s, "\r\n", 2) != 2)
			fail("write");
	}

	while ((n = read(s, buf, sizeof (buf))) > 0)
		write(1, buf, n);
	if (n < 0)
		fail("read");

	close(s);
	exit(0);
}
