/*
 * ifconfig.c -- show network interfaces, set the Ethernet address
 *
 * usage: ifconfig [interface]		show all interfaces, or one
 *        ifconfig interface a.b.c.d	set the address (root)
 *
 * UniPlus+ has no SIOCSIFADDR, only UniSoft's SIOCCIADDR, which sets the
 * address of the Ethernet interface (eb0).  With the September 2026
 * socket.c it takes effect at once: the old network route is removed and
 * the interface re-initialised with the new address.  An older kernel
 * only stores the address for the next boot.  Routes through a gateway
 * on the old network have to be deleted and added again with route.
 *
 * The interface list is read from the kernel (ifnet) through /unix and
 * /dev/kmem, so /unix must be the running kernel.
 */

#include <stdio.h>
#include <a.out.h>
#include "net/socket.h"
#include "net/in.h"

/* struct ifnet, checked against the compiled kernel */
#define IFSIZE	116
#define IF_NAME	0		/* char * */
#define IF_UNIT	4		/* short */
#define IF_MTU	6		/* short */
#define IF_FLAGS 12		/* short */
#define IF_ADDR	24		/* struct sockaddr; sin_addr at +4 */
#define IF_BROAD 40		/* struct sockaddr */
#define IF_SNDLEN 64		/* if_snd.ifq_len */
#define IF_SNDMAX 68		/* if_snd.ifq_maxlen */
#define IF_SNDDROP 72		/* if_snd.ifq_drops */
#define IF_IPKTS 92
#define IF_IERRS 96
#define IF_OPKTS 100
#define IF_OERRS 104
#define IF_COLLS 108
#define IF_NEXT	112

#define IFF_UP		0x1
#define IFF_BROADCAST	0x2
#define IFF_DEBUG	0x4
#define IFF_ROUTE	0x8
#define IFF_POINTOPOINT	0x10
#define IFF_NOTRAILERS	0x20
#define IFF_RUNNING	0x40
#define IFF_NOARP	0x80

#define MAXIF	16

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

/* dotted quad to host-order (== network-order on 68000) address */
parseaddr(s, ap)
register char *s;
unsigned long *ap;
{
	unsigned long a = 0;
	register int i, n;

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

prtaddr(a)
unsigned long a;
{
	printf("%d.%d.%d.%d", (int)((a >> 24) & 0xff), (int)((a >> 16) & 0xff),
	    (int)((a >> 8) & 0xff), (int)(a & 0xff));
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
		perror("ifconfig: /dev/kmem");
		exit(1);
	}
	lseek(kfd, addr, 0);
	return (read(kfd, buf, n) == n ? 0 : -1);
}

/* "eb0" from a kernel struct ifnet */
ifname(ifn, buf)
char *ifn, *buf;
{
	char name[9];
	register int i;

	strcpy(buf, "?");
	if (kread(getlong(ifn + IF_NAME), name, 8) < 0)
		return;
	name[8] = '\0';
	for (i = 0; name[i] >= 'a' && name[i] <= 'z'; i++)
		;
	name[i] = '\0';
	sprintf(buf, "%s%d", name, getshort(ifn + IF_UNIT));
}

prtflags(f)
int f;
{
	static struct {
		int bit;
		char *name;
	} fl[] = {
		IFF_UP, "UP", IFF_BROADCAST, "BROADCAST", IFF_DEBUG, "DEBUG",
		IFF_ROUTE, "ROUTE", IFF_POINTOPOINT, "POINTOPOINT",
		IFF_NOTRAILERS, "NOTRAILERS", IFF_RUNNING, "RUNNING",
		IFF_NOARP, "NOARP", 0, 0
	};
	register int i, n = 0;

	printf("flags=%x<", f & 0xffff);
	for (i = 0; fl[i].bit; i++)
		if (f & fl[i].bit)
			printf("%s%s", n++ ? "," : "", fl[i].name);
	printf(">");
}

show(ifn)
char *ifn;
{
	char name[16];
	int flags = getshort(ifn + IF_FLAGS);

	ifname(ifn, name);
	printf("%s: ", name);
	prtflags(flags);
	printf(" mtu %d\n", getshort(ifn + IF_MTU));
	printf("\tinet ");
	prtaddr((unsigned long)getlong(ifn + IF_ADDR + 4));
	if (flags & IFF_BROADCAST) {
		printf(" broadcast ");
		prtaddr((unsigned long)getlong(ifn + IF_BROAD + 4));
	}
	printf("\n\tpackets in %ld errors %ld, out %ld errors %ld, collisions %ld\n",
	    getlong(ifn + IF_IPKTS), getlong(ifn + IF_IERRS),
	    getlong(ifn + IF_OPKTS), getlong(ifn + IF_OERRS),
	    getlong(ifn + IF_COLLS));
	printf("\tsend queue %ld of %ld, dropped %ld\n", getlong(ifn + IF_SNDLEN),
	    getlong(ifn + IF_SNDMAX), getlong(ifn + IF_SNDDROP));
}

/* read the interface list into ifs; returns how many */
getifs(ifs)
char ifs[][IFSIZE];
{
	char head[4];
	long a;
	int n;

	if ((a = ksym("_ifnet")) == 0 || kread(a, head, 4) < 0) {
		fprintf(stderr, "ifconfig: can't read ifnet from /unix and /dev/kmem\n");
		exit(1);
	}
	for (n = 0, a = getlong(head); a && n < MAXIF; n++) {
		if (kread(a, ifs[n], IFSIZE) < 0) {
			fprintf(stderr, "ifconfig: bad interface entry\n");
			break;
		}
		a = getlong(ifs[n] + IF_NEXT);
	}
	return (n);
}

setaddr(name, addrs, ifs, n)
char *name, *addrs;
char ifs[][IFSIZE];
int n;
{
	char buf[16];
	unsigned long a;
	long la;
	register int i;
	int s;

	if (parseaddr(addrs, &a) < 0 || a == 0) {
		fprintf(stderr, "ifconfig: bad address %s\n", addrs);
		exit(2);
	}
	for (i = 0; i < n; i++) {
		ifname(ifs[i], buf);
		if (strcmp(buf, name) == 0)
			break;
	}
	if (i == n) {
		fprintf(stderr, "ifconfig: no interface %s\n", name);
		exit(1);
	}
	if ((getshort(ifs[i] + IF_FLAGS) & IFF_BROADCAST) == 0) {
		fprintf(stderr, "ifconfig: only the Ethernet interface's address can be set\n");
		exit(1);
	}
	s = socket(SOCK_DGRAM, (struct sockproto *)0, (struct sockaddr *)0, 0);
	if (s < 0) {
		perror("ifconfig: socket");
		exit(1);
	}
	la = (long)htonl(a);
	if (ioctl(s, SIOCCIADDR, (char *)&la) < 0) {
		perror("ifconfig: SIOCCIADDR");
		exit(1);
	}
	close(s);
}

main(argc, argv)
int argc;
char *argv[];
{
	static char ifs[MAXIF][IFSIZE];
	char buf[16];
	register int i;
	int n, found = 0;

	if (argc > 3) {
		fprintf(stderr, "usage: ifconfig [interface [a.b.c.d]]\n");
		exit(2);
	}
	n = getifs(ifs);
	if (argc == 3) {
		setaddr(argv[1], argv[2], ifs, n);
		n = getifs(ifs);
	}
	for (i = 0; i < n; i++) {
		ifname(ifs[i], buf);
		if (argc == 1 || strcmp(buf, argv[1]) == 0) {
			show(ifs[i]);
			found++;
		}
	}
	if (argc > 1 && !found) {
		fprintf(stderr, "ifconfig: no interface %s\n", argv[1]);
		exit(1);
	}
	exit(0);
}
