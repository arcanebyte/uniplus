/*
 * netinfo.c -- show UniPlus+ network settings (unix.net kernel)
 *
 * usage: netinfo
 *
 * Prints the host name (gethostname, syscall 71) and the Internet
 * address the kernel was configured with (SIOCGIADDR on a socket,
 * read from the ubdinit flags in conf.c), plus its network class.
 * No pipe characters in this file: LisaEm's keyboard turns them into
 * '?' when pasted.
 */

#include <stdio.h>
#include "net/socket.h"
#include "net/in.h"

main()
{
	char name[33];
	unsigned long a;
	long addr;
	int s, i;

	for (i = 0; i < sizeof (name); i++)
		name[i] = '\0';
	if (gethostname(name, sizeof (name) - 1) < 0)
		perror("netinfo: gethostname");
	else
		printf("host name:        %s\n", name);

	s = socket(SOCK_DGRAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	if (s < 0) {
		perror("netinfo: socket");
		exit(1);
	}
	if (ioctl(s, SIOCGIADDR, (char *)&addr) < 0) {
		perror("netinfo: ioctl SIOCGIADDR");
		exit(1);
	}
	close(s);

	a = ntohl((unsigned long)addr);
	printf("internet address: %d.%d.%d.%d (0x%08lx)\n",
	    (int)((a >> 24) & 0xff), (int)((a >> 16) & 0xff),
	    (int)((a >> 8) & 0xff), (int)(a & 0xff), a);
	if ((a & 0x80000000L) == 0)
		printf("network:          class A, net %d\n", (int)((a >> 24) & 0xff));
	else if ((a & 0xc0000000L) == 0x80000000L)
		printf("network:          class B, net %d.%d\n",
		    (int)((a >> 24) & 0xff), (int)((a >> 16) & 0xff));
	else
		printf("network:          class C, net %d.%d.%d\n",
		    (int)((a >> 24) & 0xff), (int)((a >> 16) & 0xff),
		    (int)((a >> 8) & 0xff));
	printf("loopback:         127.0.0.1 (lo0)\n");
	exit(0);
}
