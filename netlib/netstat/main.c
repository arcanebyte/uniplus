#ifndef lint
static char sccsid[] = "@(#)main.c	4.2 82/10/06";
#endif

/*
 * Lisa (UniPlus+ with unix.net), from 2.9BSD's netser/netstat: built
 * against the kernel's own headers (/usr/src/include), so the structures
 * read from /dev/kmem match the kernel; nlist from <a.out.h>; no IMP host
 * table (-h) and no VAX page map for crash dumps.
 */
#include <sys/types.h>
#include "net/misc.h"
#define	U_LONG_DEFINED		/* net/misc.h has u_long and friends */
#include <sys/param.h>
#include <ctype.h>
#include <errno.h>
#include "netdb.h"
#include <a.out.h>
#include <stdio.h>

struct nlist nl[] = {
#define	N_MBSTAT	0
	{ "_mbstat" },
#define	N_IPSTAT	1
	{ "_ipstat" },
#define	N_TCB		2
	{ "_tcb" },
#define	N_TCPSTAT	3
	{ "_tcpstat" },
#define	N_UDB		4
	{ "_udb" },
#define	N_UDPSTAT	5
	{ "_udpstat" },
#define	N_RAWCB		6
	{ "_rawcb" },
#define	N_IFNET		7
	{ "_ifnet" },
#define	N_RTHOST	8
	{ "_rthost" },
#define	N_RTNET		9
	{ "_rtnet" },
	{ "" },
};

struct protox {
	short	pr_index;	/* index into nlist of cb head */
	short	pr_wanted;	/* 1 if wanted, 0 otherwise */
	char	*pr_name;	/* well-known name */
} protox[] = {
	{ N_TCB,	1,	"tcp" },
	{ N_UDB,	1,	"udp" },
	{ -1,		0,	0 }
};

char	*unixf = "/unix";	/* Lisa: "system" is a libc routine */
char	*kmemf = "/dev/kmem";
int	kmem;
int	kflag;
int	Aflag;
int	aflag;
int	hflag;
int	iflag;
int	mflag;
int	nflag;
int	rflag;
int	sflag;
int	tflag;
int	interval;
char	usage[] = "[ -Aaimnrst ] [ interval ] [ system ] [ core ]";

main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	char *cp, *name;
	register struct protoent *p;

	name = argv[0];
	argc--, argv++;
  	while (argc > 0 && **argv == '-') {
		for (cp = &argv[0][1]; *cp; cp++)
		switch(argv[0][1]) {

		case 'A':
			Aflag++;
			break;

		case 'a':
			aflag++;
			break;

		case 'i':
			iflag++;
			break;

		case 'm':
			mflag++;
			break;

		case 'n':
			nflag++;
			break;

		case 'r':
			rflag++;
			break;

		case 's':
			sflag++;
			break;

		case 't':
			tflag++;
			break;

		default:
use:
			printf("usage: %s %s\n", name, usage);
			exit(1);
		}
		argv++, argc--;
	}
	if (argc > 0 && isdigit(argv[0][0])) {
		interval = atoi(argv[0]);
		if (interval <= 0)
			goto use;
		argv++, argc--;
		iflag++;
	}
	if (argc > 0) {
		unixf = *argv;
		argv++, argc--;
	}
	nlist(unixf, nl);
	if (nl[0].n_type == 0) {
		fprintf(stderr, "%s: no namelist\n", unixf);
		exit(1);
	}
	if (argc > 0) {
		kmemf = *argv;
		kflag++;
	}
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		fprintf(stderr, "cannot open ");
		perror(kmemf);
		exit(1);
	}
	if (mflag) {
		mbpr((off_t)nl[N_MBSTAT].n_value);
		exit(0);
	}
	/*
	 * Keep file descriptors open to avoid overhead
	 * of open/close on each call to get* routines.
	 */
	sethostent(1);
	setnetent(1);
	if (iflag) {
		intpr(interval, (off_t)nl[N_IFNET].n_value);
		exit(0);
	}
	if (rflag) {
		routepr((off_t)nl[N_RTHOST].n_value, (off_t)nl[N_RTNET].n_value);
		exit(0);
	}
	if (sflag) {
		/* Lisa: the order of /etc/protocols; no icmpstat */
		ip_stats((off_t)nl[N_IPSTAT].n_value, "ip");
		tcp_stats((off_t)nl[N_TCPSTAT].n_value, "tcp");
		udp_stats((off_t)nl[N_UDPSTAT].n_value, "udp");
		exit(0);
	}
	setprotoent(1);
	setservent(1);
	while (p = getprotoent()) {
		register struct protox *tp;

		for (tp = protox; tp->pr_index >= 0; tp++)
			if (strcmp(tp->pr_name, p->p_name) == 0)
				break;
		if (tp->pr_index < 0 || tp->pr_wanted == 0)
			continue;
		protopr((off_t)nl[tp->pr_index].n_value, p->p_name);
	}
	endprotoent();
}

/*
 * Seek into the kernel for a value.
 */
klseek(fd, base, off)
	int fd;
	long base;
	int off;
{

	lseek(fd, base, off);
}
