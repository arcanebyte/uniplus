#include <sys/param.h>
#include <signal.h>
#include <sys/socket.h>
#include <net/in.h>
#include <stdio.h>

struct sockaddr_in hisctladdr = {AF_INET, 83 };
char buf[256];

main(argc,argv)
int argc;
char *argv[];
{	register i, netfd;

	if (argc <= 1) {
		printf("whois who?\n");
		exit();
	}
	hisctladdr.sin_port = htons(hisctladdr.sin_port);
	hisctladdr.sin_addr.s_addr = htonl(01200400063l);
	if ((netfd = socket(SOCK_STREAM, 0, (struct sockaddr *)0, 0)) < 0
	    || connect(netfd, &hisctladdr)) {
		perror(raddr(hisctladdr.sin_addr.s_addr));
		exit(1);
	}
	sprintf(buf, "%s\r\n", argv[1]);
	write (netfd, buf, strlen(buf));
	while ((i = read(netfd, buf, sizeof(buf))) > 0) write(1, buf, i);
	exit(0);
}
