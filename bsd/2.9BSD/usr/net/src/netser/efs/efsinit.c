#ifndef lint
static char sccsid[] = "@(#)efsinit.c	4.1 6/27/82";
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/in.h>
#include <errno.h>
#include <pwd.h>
#include <wait.h>
#include <signal.h>
#include <sgtty.h>
#include <sys/efs.h>

#define EFSDEV		"/dev/efs"

struct hosttable hosttable[EFS_NHT];

long int rhost();

main(argc, argv)
	int argc;
	char *argv[];
{
	char *hp;
	register int nh, iaddr, fd;

	nh = 0;
	while(argc > 1) {
		hp = argv[1];
		iaddr = rhost(&hp);
		if(iaddr == -1)
			fprintf(stderr, "%s: address unknown\n", argv[1]);
		else {
			strcpy(hosttable[nh].ht_name, argv[1]);
			hosttable[nh].ht_addr.sin_family = AF_INET;
			hosttable[nh].ht_addr.sin_port = 
						htons(IPPORT_EFSSERVER);
			hosttable[nh].ht_addr.sin_addr.s_addr = iaddr;
			nh++;
		}
		argc--; argv++;
	}
	if((fd = open(EFSDEV, 0)) < 0) {
		perror("EFSDEV open");
		exit(1);
	}
	if(ioctl(fd, EFSIOSHTAB, hosttable) < 0) {
		perror("EFSDEV ioctl");
		exit(1);
	}
	exit(0);
}
