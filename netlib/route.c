/*
 * route.c -- show and change the routing table on UniPlus+ unix.net
 *
 * usage: route [show]
 *        route add dest gateway
 *        route delete dest gateway
 *
 * dest is a dotted address or "default" (0.0.0.0).  As in 4.1a BSD's
 * route(8), dest is a host route if its host part is non-zero for its
 * class (10.0.2.99), and a network route otherwise (10.0.0.0).  The
 * route goes through a gateway unless gateway is one of this system's
 * own addresses, which makes it a direct route on that interface.
 * add and delete need root.
 *
 * The kernel has no call to list routes, so show reads its routing
 * hash tables (rthost, rtnet) from /dev/kmem, using /unix for their
 * addresses: /unix must be the running kernel.  A default route needs
 * a kernel whose rtalloc() falls back to it (route.c, September 2026).
 */

#include <stdio.h>
#include <errno.h>
#include <a.out.h>
#include "net/socket.h"
#include "net/in.h"
#include "net/route.h"

#define MBHDR	12		/* struct mbuf: m_next, m_off, m_len, m_free */
#define RTSIZE	48		/* struct rtentry */
#define MAXCHAIN 64		/* entries per hash chain before giving up */

extern int errno;

/* dotted quad or "default" to host-order (== network-order on 68000) address */
parseaddr(s, ap)
register char *s;
unsigned long *ap;
{
	unsigned long a = 0;
	register int i, n;

	if (strcmp(s, "default") == 0) {
		*ap = 0;
		return (0);
	}
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

/* host part of an address for its class, as the kernel's in_lnaof() */
unsigned long
lnaof(a)
unsigned long a;
{
	if ((a & 0x80000000L) == 0)
		return (a & 0x00ffffffL);
	if ((a & 0xc0000000L) == 0x80000000L)
		return (a & 0x0000ffffL);
	return (a & 0x000000ffL);
}

/* print an address, padded to a 17 character column if pad is set */
prtaddr(a, pad)
unsigned long a;
int pad;
{
	char buf[20];

	if (a == 0)
		strcpy(buf, "default");
	else
		sprintf(buf, "%d.%d.%d.%d", (int)((a >> 24) & 0xff),
		    (int)((a >> 16) & 0xff), (int)((a >> 8) & 0xff),
		    (int)(a & 0xff));
	printf(pad ? "%-17s" : "%s", buf);
}

long
getlong(p)
register char *p;
{
	return (((long)(p[0] & 0xff) << 24) + ((long)(p[1] & 0xff) << 16) +
	    ((long)(p[2] & 0xff) << 8) + (long)(p[3] & 0xff));
}

short
getshort(p)
register char *p;
{
	return ((short)(((p[0] & 0xff) << 8) + (p[1] & 0xff)));
}

/* address of a symbol in /unix (names keep 8 characters), 0 if not found */
long
ksym(name)
char *name;
{
	struct nlist nl[2];

	strncpy(nl[0].n_name, name, 8);
	nl[0].n_value = 0;
	nl[1].n_name[0] = '\0';
	nlist("/unix", nl);
	return ((long)nl[0].n_value);
}

int kfd = -1;

/* read n bytes of kernel memory; returns 0 on success */
kread(addr, buf, n)
long addr;
char *buf;
int n;
{
	if (kfd < 0 && (kfd = open("/dev/kmem", 0)) < 0) {
		perror("route: /dev/kmem");
		exit(1);
	}
	lseek(kfd, addr, 0);
	return (read(kfd, buf, n) == n ? 0 : -1);
}

/* interface name and unit ("eb0") from a kernel struct ifnet pointer */
ifname(ifp, buf)
long ifp;
char *buf;
{
	char b[6], name[9];
	register int i;

	strcpy(buf, "?");
	if (ifp == 0 || kread(ifp, b, 6) < 0)
		return;
	if (kread(getlong(b), name, 8) < 0)
		return;
	name[8] = '\0';
	for (i = 0; name[i] >= 'a' && name[i] <= 'z'; i++)
		;
	name[i] = '\0';
	sprintf(buf, "%s%d", name, getshort(b + 4));
}

/* print one hash table: 7 bucket heads of mbuf chains holding rtentries */
showtab(sym)
char *sym;
{
	char heads[4 * RTHASHSIZ], mb[MBHDR], rt[RTSIZE], ifn[16];
	long addr, m;
	register int b, n;
	short flags;

	if ((addr = ksym(sym)) == 0) {
		fprintf(stderr, "route: no %s in /unix\n", sym);
		exit(1);
	}
	if (kread(addr, heads, sizeof (heads)) < 0) {
		fprintf(stderr, "route: can't read %s\n", sym);
		exit(1);
	}
	for (b = 0; b < RTHASHSIZ; b++) {
		m = getlong(heads + 4 * b);
		for (n = 0; m && n < MAXCHAIN; n++) {
			if (kread(m, mb, MBHDR) < 0 ||
			    kread(m + getlong(mb + 4), rt, RTSIZE) < 0) {
				fprintf(stderr, "route: bad entry in %s\n", sym);
				break;
			}
			/* rt_dst and rt_gateway are sockaddr_in: address at +4 */
			prtaddr((unsigned long)getlong(rt + 4 + 4), 1);
			prtaddr((unsigned long)getlong(rt + 20 + 4), 1);
			flags = getshort(rt + 36);
			printf("%c%c%c    %4d %8ld  ", (flags & RTF_UP) ? 'U' : ' ',
			    (flags & RTF_GATEWAY) ? 'G' : ' ',
			    (flags & RTF_HOST) ? 'H' : ' ',
			    getshort(rt + 38), getlong(rt + 40));
			ifname(getlong(rt + 44), ifn);
			printf("%s\n", ifn);
			m = getlong(mb);
		}
	}
}

show()
{
	printf("Destination      Gateway          Flags Refs      Use  Interface\n");
	showtab("_rthost");
	showtab("_rtnet");
}

/* is a one of this system's addresses: the interface's or loopback? */
islocal(s, a)
int s;
unsigned long a;
{
	long mine;

	if (a == 0x7f000001L)
		return (1);
	if (ioctl(s, SIOCGIADDR, (char *)&mine) == 0 &&
	    (unsigned long)mine == a)
		return (1);
	return (0);
}

setaddr(sp, a)
struct sockaddr *sp;
unsigned long a;
{
	register struct sockaddr_in *sin = (struct sockaddr_in *)sp;
	register char *p;
	register int i;

	p = (char *)sp;
	for (i = 0; i < sizeof (struct sockaddr); i++)
		*p++ = 0;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(a);
}

change(add, dests, gws)
int add;
char *dests, *gws;
{
	struct rtentry rt;
	unsigned long dst, gw;
	register char *p;
	register int i;
	int s;

	if (parseaddr(dests, &dst) < 0) {
		fprintf(stderr, "route: bad destination %s\n", dests);
		exit(2);
	}
	if (parseaddr(gws, &gw) < 0 || gw == 0) {
		fprintf(stderr, "route: bad gateway %s\n", gws);
		exit(2);
	}
	s = socket(SOCK_DGRAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	if (s < 0) {
		perror("route: socket");
		exit(1);
	}

	p = (char *)&rt;
	for (i = 0; i < sizeof (rt); i++)
		*p++ = 0;
	setaddr(&rt.rt_dst, dst);
	setaddr(&rt.rt_gateway, gw);
	rt.rt_flags = RTF_UP;
	if (lnaof(dst) != 0)
		rt.rt_flags |= RTF_HOST;
	if (!islocal(s, gw))
		rt.rt_flags |= RTF_GATEWAY;

	if (ioctl(s, add ? SIOCADDRT : SIOCDELRT, (char *)&rt) < 0) {
		if (errno == EEXIST)
			fprintf(stderr, "route: already in the table\n");
		else if (errno == ESRCH)
			fprintf(stderr, "route: not in the table\n");
		else if (errno == ENETUNREACH)
			fprintf(stderr, "route: gateway isn't on a local network\n");
		else
			perror(add ? "route: add" : "route: delete");
		exit(1);
	}
	printf("%s %s ", add ? "add" : "delete",
	    (rt.rt_flags & RTF_HOST) ? "host" : "net");
	prtaddr(dst, 0);
	printf(": gateway ");
	prtaddr(gw, 0);
	printf("%s\n", (rt.rt_flags & RTF_GATEWAY) ? "" : " (direct)");
	close(s);
}

usage()
{
	fprintf(stderr, "usage: route [show]\n");
	fprintf(stderr, "       route add dest gateway\n");
	fprintf(stderr, "       route delete dest gateway\n");
	fprintf(stderr, "dest is a.b.c.d or default\n");
	exit(2);
}

main(argc, argv)
int argc;
char *argv[];
{
	if (argc == 1 || (argc == 2 && strcmp(argv[1], "show") == 0))
		show();
	else if (argc == 4 && strcmp(argv[1], "add") == 0)
		change(1, argv[2], argv[3]);
	else if (argc == 4 && (strcmp(argv[1], "delete") == 0 ||
	    strcmp(argv[1], "del") == 0))
		change(0, argv[2], argv[3]);
	else
		usage();
	exit(0);
}
