/*
 * udptest.c -- one-shot UDP loopback test for UniPlus+ running unix.net
 *
 * usage: udptest [port]		(default port 5001)
 *
 * Forks.  The child binds a datagram socket to the port, receives one
 * datagram and sends it back to whoever sent it.  The parent sends a
 * datagram to 127.0.0.1 from an ephemeral port, waits up to 10 seconds
 * for the reply, compares it and prints PASS or FAIL.
 *
 * 4.1a BSD sockets: the local address goes to socket(); send() and
 * receive() take the peer address as their second argument.
 * No pipe characters in this file: LisaEm's keyboard turns them into
 * '?' when pasted.
 */

#include <stdio.h>
#include <signal.h>
#include "net/socket.h"
#include "net/in.h"

char msg[] = "UDP datagram from UniPlus+ on the Lisa.";
int child;

setaddr(sp, addr, port)
struct sockaddr_in *sp;
unsigned long addr;
int port;
{
	register char *p;
	register int i;

	p = (char *)sp;
	for (i = 0; i < sizeof (*sp); i++)
		*p++ = 0;
	sp->sin_family = AF_INET;
	sp->sin_port = htons(port);
	sp->sin_addr.s_addr = htonl(addr);
}

prtaddr(sp)
struct sockaddr_in *sp;
{
	unsigned long a;

	a = ntohl(sp->sin_addr.s_addr);
	printf("%d.%d.%d.%d port %d", (int)((a >> 24) & 0xff),
	    (int)((a >> 16) & 0xff), (int)((a >> 8) & 0xff),
	    (int)(a & 0xff), (int)ntohs(sp->sin_port));
}

ontimer()
{
	printf("udptest: FAIL, no reply within 10 seconds\n");
	kill(child, SIGTERM);
	exit(1);
}

/* child: receive one datagram and send it back to its sender */
server(port)
int port;
{
	struct sockaddr_in sin, from;
	char buf[512];
	int s, n;

	setaddr(&sin, 0L, port);
	s = socket(SOCK_DGRAM, (struct sockproto *)0, (struct sockaddr *)&sin, 0);
	if (s < 0) {
		perror("udptest server: socket");
		exit(1);
	}
	setaddr(&from, 0L, 0);
	n = receive(s, (struct sockaddr *)&from, buf, sizeof (buf));
	if (n < 0) {
		perror("udptest server: receive");
		exit(1);
	}
	printf("udptest server: got %d bytes from ", n);
	prtaddr(&from);
	printf("\n");
	fflush(stdout);
	if (send(s, (struct sockaddr *)&from, buf, n) != n) {
		perror("udptest server: send");
		exit(1);
	}
	close(s);
	exit(0);
}

main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in to, from;
	char buf[512];
	int s, n, len, port, i, status;

	port = 5001;
	if (argc > 1)
		port = atoi(argv[1]);

	child = fork();
	if (child < 0) {
		perror("udptest: fork");
		exit(1);
	}
	if (child == 0)
		server(port);

	sleep(2);		/* let the child bind its socket */

	s = socket(SOCK_DGRAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	if (s < 0) {
		perror("udptest: socket");
		kill(child, SIGTERM);
		exit(1);
	}

	signal(SIGALRM, ontimer);
	alarm(10);

	setaddr(&to, 0x7f000001L, port);	/* 127.0.0.1 */
	len = strlen(msg);
	if (send(s, (struct sockaddr *)&to, msg, len) != len) {
		perror("udptest: send");
		kill(child, SIGTERM);
		exit(1);
	}

	setaddr(&from, 0L, 0);
	n = receive(s, (struct sockaddr *)&from, buf, sizeof (buf));
	alarm(0);
	if (n < 0) {
		perror("udptest: receive");
		kill(child, SIGTERM);
		exit(1);
	}
	close(s);
	wait(&status);

	printf("udptest: reply of %d bytes from ", n);
	prtaddr(&from);
	printf("\n");
	if (n != len) {
		printf("udptest: FAIL, sent %d bytes, got %d back\n", len, n);
		exit(1);
	}
	for (i = 0; i < len; i++)
		if (buf[i] != msg[i]) {
			printf("udptest: FAIL, byte %d differs\n", i);
			exit(1);
		}
	buf[len] = '\0';
	printf("udptest: echoed \"%s\"\n", buf);
	printf("udptest: PASS, %d byte datagram round trip\n", len);
	exit(0);
}
