/*
 * looptest.c -- one-shot TCP loopback test for UniPlus+ running unix.net
 *
 * usage: looptest [port]		(default port 5000)
 *
 * Forks.  The child makes a listening socket on the port and echoes
 * one connection back.  The parent connects to 127.0.0.1, sends a
 * message, reads the echo, compares it and prints PASS or FAIL.
 *
 * 4.1a BSD sockets: the local address goes to socket(), SO_ACCEPTCONN
 * makes it passive, and accept() connects that same socket.
 * No pipe characters in this file: LisaEm's keyboard turns them into
 * '?' when pasted.
 */

#include <stdio.h>
#include <signal.h>
#include "net/socket.h"
#include "net/in.h"

char msg[] = "Hello from UniPlus+ on the Lisa, over TCP loopback.";

/* fill in an Internet socket address */
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

/* child: listen, accept one connection, echo until the peer closes */
server(port)
int port;
{
	struct sockaddr_in sin, from;
	char buf[512];
	int s, n;

	setaddr(&sin, 0L, port);
	s = socket(SOCK_STREAM, (struct sockproto *)0,
	    (struct sockaddr *)&sin, SO_ACCEPTCONN);
	if (s < 0) {
		perror("looptest server: socket");
		exit(1);
	}
	if (accept(s, (struct sockaddr *)&from) < 0) {
		perror("looptest server: accept");
		exit(1);
	}
	while ((n = read(s, buf, sizeof (buf))) > 0)
		if (write(s, buf, n) != n) {
			perror("looptest server: write");
			exit(1);
		}
	close(s);
	exit(0);
}

main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in sin;
	char buf[512];
	int s, n, got, len, port, pid, i, status;

	port = 5000;
	if (argc > 1)
		port = atoi(argv[1]);

	pid = fork();
	if (pid < 0) {
		perror("looptest: fork");
		exit(1);
	}
	if (pid == 0)
		server(port);

	sleep(2);		/* let the child start listening */

	setaddr(&sin, 0x7f000001L, port);	/* 127.0.0.1 */
	s = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	if (s < 0) {
		perror("looptest: socket");
		kill(pid, SIGTERM);
		exit(1);
	}
	if (connect(s, (struct sockaddr *)&sin) < 0) {
		perror("looptest: connect");
		kill(pid, SIGTERM);
		exit(1);
	}
	printf("looptest: connected to 127.0.0.1 port %d\n", port);

	len = strlen(msg);
	if (write(s, msg, len) != len) {
		perror("looptest: write");
		kill(pid, SIGTERM);
		exit(1);
	}

	got = 0;
	while (got < len) {
		n = read(s, buf + got, sizeof (buf) - got);
		if (n <= 0)
			break;
		got += n;
	}
	close(s);

	kill(pid, SIGTERM);
	wait(&status);

	if (got != len) {
		printf("looptest: FAIL, sent %d bytes, got %d back\n", len, got);
		exit(1);
	}
	for (i = 0; i < len; i++)
		if (buf[i] != msg[i]) {
			printf("looptest: FAIL, byte %d differs\n", i);
			exit(1);
		}
	buf[len] = '\0';
	printf("looptest: echoed \"%s\"\n", buf);
	printf("looptest: PASS, %d bytes round trip\n", len);
	exit(0);
}
