/*
 * hosttest.c -- exercise libnetdb.a against /etc/hosts, /etc/services,
 * /etc/protocols and /etc/networks, and the resolver.
 *
 * usage: hosttest [name ...]
 *
 * With no arguments, checks the resolver's packet layout (the HEADER
 * bit fields, res_mkquery, dn_expand), then looks up the entries the
 * sample data bases in netlib/etc should have, and prints PASS or FAIL
 * for each.  With arguments, prints what gethostbyname() (or inet_addr()
 * for a dotted address and gethostbyaddr()) finds for each: through the
 * name servers in /etc/resolv.conf, then /etc/hosts.
 */

#include <stdio.h>
#include <sys/types.h>
#include "net/socket.h"
#include "net/in.h"
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>

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
		printf("%s: unknown host (h_errno %d)\n", name, h_errno);
		return;
	}
	printf("%s: %s", name, hp->h_name);
	for (ap = hp->h_aliases; *ap; ap++)
		printf(" %s", *ap);
	for (ap = hp->h_addr_list; *ap; ap++) {
		bcopy(*ap, (char *)&a, sizeof (a));
		printf(" %s", inet_ntoa(a));
	}
	printf("\n");
}

/* the query res_mkquery() should make for lisa.example, type A, id 1 */
char	qwant[] = {
	0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	4, 'l', 'i', 's', 'a', 7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 0,
	0, 1, 0, 1
};

resolver()
{
	char buf[PACKETSZ], name[MAXDNAME];
	HEADER *hp;
	int n;

	check("sizeof (HEADER) is 12", sizeof (HEADER) == 12);
	n = res_mkquery(QUERY, "lisa.example", C_IN, T_A, (char *)0, 0,
	    (struct rrec *)0, buf, sizeof (buf));
	check("res_mkquery length", n == sizeof (qwant));
	check("res_mkquery bytes", n == sizeof (qwant) &&
	    bcmp(buf, qwant, n) == 0);
	check("dn_expand", dn_expand(buf, buf + n, buf + 12, name,
	    sizeof (name)) == 14 && strcmp(name, "lisa.example") == 0);
	check("getshort", getshort(buf + n - 4) == T_A);

	/* a compression pointer to offset 12 */
	buf[n] = (char)0xc0;
	buf[n + 1] = 12;
	check("dn_expand pointer", dn_expand(buf, buf + n + 2, buf + n, name,
	    sizeof (name)) == 2 && strcmp(name, "lisa.example") == 0);

	/* a response: QR, AA; RA, rcode 3 (NXDOMAIN) */
	buf[2] = (char)0x84;
	buf[3] = (char)0x83;
	hp = (HEADER *)buf;
	check("HEADER bit fields", hp->qr == 1 && hp->opcode == 0 &&
	    hp->aa == 1 && hp->tc == 0 && hp->rd == 0 && hp->ra == 1 &&
	    hp->pr == 0 && hp->rcode == NXDOMAIN);
	check("HEADER counts", ntohs(hp->qdcount) == 1 &&
	    hp->ancount == 0);
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

	resolver();

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
