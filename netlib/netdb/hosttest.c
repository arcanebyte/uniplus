/*
 * hosttest.c -- exercise libnetdb.a against /etc/hosts, /etc/services,
 * /etc/protocols and /etc/networks.
 *
 * usage: hosttest [name ...]
 *
 * With no arguments, looks up the entries the sample data bases in
 * netlib/etc should have and prints PASS or FAIL for each.  With
 * arguments, prints what gethostbyname() (or inet_addr() for a dotted
 * address and gethostbyaddr()) finds for each.
 */

#include <stdio.h>
#include <sys/types.h>
#include "net/socket.h"
#include "net/in.h"
#include <netdb.h>

struct in_addr	inet_makeaddr();
int	failed;

check(what, ok)
char *what;
int ok;
{
	printf("%s %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failed++;
}

/* does hp hold the address s? */
isaddr(hp, s)
struct hostent *hp;
char *s;
{
	u_long a = inet_addr(s);

	return (hp != NULL && bcmp(hp->h_addr, (char *)&a, sizeof (a)) == 0);
}

show(name)
char *name;
{
	struct hostent *hp;
	struct in_addr a;
	char **ap;

	if (*name >= '0' && *name <= '9') {
		a.s_addr = inet_addr(name);
		hp = gethostbyaddr((char *)&a, sizeof (a), AF_INET);
	} else
		hp = gethostbyname(name);
	if (hp == NULL) {
		printf("%s: unknown host\n", name);
		return;
	}
	bcopy(hp->h_addr, (char *)&a, sizeof (a));
	printf("%s: %s", name, hp->h_name);
	for (ap = hp->h_aliases; *ap; ap++)
		printf(" %s", *ap);
	printf(" %s\n", inet_ntoa(a));
}

main(argc, argv)
int argc;
char *argv[];
{
	struct hostent *hp;
	struct servent *sp;
	struct protoent *pp;
	struct netent *np;
	struct in_addr a;
	int i;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
			show(argv[i]);
		exit(0);
	}

	a.s_addr = inet_addr("10.0.2.15");
	check("inet_addr 10.0.2.15", a.s_addr == 0x0a00020fL);
	check("inet_ntoa", strcmp(inet_ntoa(a), "10.0.2.15") == 0);
	check("inet_netof class A", inet_netof(a) == 10);
	check("inet_lnaof class A", inet_lnaof(a) == 0x0000020fL);
	a.s_addr = inet_addr("128.32.0.1");
	check("inet_netof class B", inet_netof(a) == 0x8020);
	a.s_addr = inet_addr("192.168.1.7");
	check("inet_netof class C", inet_netof(a) == 0xc0a801L);
	check("inet_lnaof class C", inet_lnaof(a) == 7);
	a = inet_makeaddr((u_long)0xc0a801L, (u_long)7);
	check("inet_makeaddr", a.s_addr == inet_addr("192.168.1.7"));
	check("inet_network", inet_network("10") == 10);

	hp = gethostbyname("localhost");
	check("gethostbyname localhost", isaddr(hp, "127.0.0.1"));
	hp = gethostbyname("gateway");
	check("gethostbyname gateway (alias)", isaddr(hp, "10.0.2.2"));
	a.s_addr = inet_addr("10.0.2.15");
	hp = gethostbyaddr((char *)&a, sizeof (a), AF_INET);
	check("gethostbyaddr 10.0.2.15", hp != NULL && strcmp(hp->h_name, "lisa") == 0);
	check("gethostbyname nosuchhost", gethostbyname("nosuchhost") == NULL);

	sp = getservbyname("telnet", "tcp");
	check("getservbyname telnet/tcp", sp != NULL && sp->s_port == 23);
	sp = getservbyname("tftp", "udp");
	check("getservbyname tftp/udp", sp != NULL && sp->s_port == 69);
	sp = getservbyport(21, "tcp");
	check("getservbyport 21/tcp", sp != NULL && strcmp(sp->s_name, "ftp") == 0);
	pp = getprotobyname("tcp");
	check("getprotobyname tcp", pp != NULL && pp->p_proto == 6);
	pp = getprotobynumber(17);
	check("getprotobynumber 17", pp != NULL && strcmp(pp->p_name, "udp") == 0);
	np = getnetbyname("loopback-net");
	check("getnetbyname loopback-net", np != NULL && np->n_net == 127);
	np = getnetbyaddr(10, AF_INET);
	check("getnetbyaddr 10", np != NULL);

	printf("%s\n", failed ? "FAIL" : "PASS");
	exit(failed != 0);
}
