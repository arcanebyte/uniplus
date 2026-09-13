#include <stdio.h>
#include <ctype.h>
#ifndef NCP
#include <sys/types.h>
#include <sys/socket.h>
#include <net/in.h>
struct sockaddr_in hisctladdr = { AF_INET, IPPORT_WHOIS };
struct sockaddr_in myctladdr;
#else NCP
#include <sys/netopen.h>
struct nopenparams openparams;

#define ICPSOCK 43	/* name server is socket 43 on NIC */
#endif NCP

char *hostname = "sri-nic";

/* Hack to do NAME/FINGER ptcl on NICNAME server */

FILE *netbuf;
FILE *netin;
char ttbuffer[BUFSIZ];

main(argc, argv)
int argc;
char *argv[];
{
#ifndef NCP
	long hnum, rhost ();
#else NCP
	long int hnum, gethost();	/* number of foreign host */
#endif
	register int netfd;
	register int c;

	if(argc < 2)
	  {	printf("Usage: %s name-string\n", argv[0]);
		exit(1);
	  }

	setbuf(stdout,ttbuffer);

#ifndef NCP
	hnum = rhost (&hostname);
#else NCP
	hnum = gethost(hostname);
#endif NCP
	if (hnum == -1L) {
		printf("Unknown host:%s\n", hostname);
		exit(2);
	}
#ifndef NCP
	hisctladdr.sin_addr.s_addr = hnum;
#if vax || pdp11
	hisctladdr.sin_port = htons(hisctladdr.sin_port);
#endif
	netfd = socket(SOCK_STREAM, 0, (struct sockaddr *)0, 0);
	if (netfd < 0) {
		perror("socket");
		exit(1);
	}
	if (connect(netfd, &hisctladdr)) {
		perror("connect");
		exit(1);
	}
	if (socketaddr(netfd, &myctladdr) < 0) {
		perror("socketaddr");
		exit(1);
	}
#else NCP
	openparam.o_fskt = ICPSOCK;
	openparam.o_host = hnum;
	openparam.o_timeo = 30;
	if (  (netfd = open("/dev/network", &openparam)) == -1)
	  {	printf("Can't connect\n");
		exit(1);
	  }
#endif NCP
	printf("NIC Name Server:\n");
	netbuf = fdopen(netfd, "w");
	netin = fopen("/dev/null", "r");
	close(fileno(netin));
	fileno(netin) = fileno(netbuf);

	fprintf(netbuf,"%s\r\n", argv[1]);
	fflush(netbuf);

	while ((c=fgetc(netin)) != EOF)
		if (c != '\r')
			putchar(c);
	putchar('\n');
	fflush(stdout);
}
