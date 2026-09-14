/*
 * ping.c -- ICMP echo for UniPlus+ running unix.net
 *
 * usage: ping [-d] [-w seconds] a.b.c.d [count]	(default 4; run as root)
 *
 * Sends ICMP echo requests on a raw ICMP socket, one a second, and
 * prints each echo reply with its round trip time (1/60 s resolution).
 * It waits up to -w seconds (default 2) for each reply.  Late replies
 * still count, and replies seen twice are marked (DUP!).  Round trip
 * times are in the Lisa's time: a LisaEm running faster than real time
 * makes remote hosts look slow, so use -w or a realistic CPU speed.
 *
 * -d turns on the kernel's ICMP console messages (icmpprintfs) while
 * ping runs, and prints the raw input queue (rawintrq) and netisr
 * before and after, from /dev/kmem.
 *
 * Needs a kernel whose proto.c has the SOCK_RAW IPPROTO_ICMP entry.  On
 * an older unix.net, socket() for raw ICMP calls through a null pointer
 * and crashes the kernel, so ping first looks for that entry in the
 * running kernel's protocol table (/unix and /dev/kmem) and refuses to
 * go on without it.
 *
 * The kernel strips the IP header before handing an echo reply to a raw
 * socket, and zeroes its checksum.  icmp_input() also treats the first
 * data byte as the header length of an embedded IP packet and drops
 * replies too short for it, so the data starts with a 0 byte.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <a.out.h>
#include <sys/types.h>
#include <sys/times.h>
#include "net/socket.h"
#include "net/in.h"

#define DATALEN	56		/* ICMP data bytes, 64 with the header */
#define HZ	60		/* times() ticks a second */
#define PRSIZE	44		/* struct protosw: 4 shorts, 9 pointers */
#define NPRSCAN	16		/* protocol table entries to look through */
#define MAXSEQ	1000		/* largest count */

int timedout;
int debug;
long sentat[MAXSEQ + 1];	/* times() when each request was sent */
char seen[MAXSEQ + 1];		/* a reply to it has been counted */

fail(what)
char *what;
{
	perror(what);
	if (debug)
		kflag("_icmppri", 0);
	exit(1);
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

short
getshort(p)
register char *p;
{
	return ((short)(((p[0] & 0xff) << 8) + (p[1] & 0xff)));
}

long
getlong(p)
register char *p;
{
	return (((long)getshort(p) << 16) + (getshort(p + 2) & 0xffffL));
}

/*
 * Is there a SOCK_RAW, PF_INET, IPPROTO_ICMP entry with a user request
 * routine in the running kernel's protocol table?  1 yes, 0 no, -1 can't
 * tell.  The kernel's symbol table keeps 8 characters, so "_protosw" may
 * name protosw or protoswLAST, the pointer to its last entry.
 */
chkkern()
{
	struct nlist nl[2];
	char tab[NPRSCAN * PRSIZE];
	register char *e;
	register int i;
	long base, last;
	int fd;

	strncpy(nl[0].n_name, "_protosw", 8);
	nl[0].n_value = 0;
	nl[1].n_name[0] = '\0';
	if (nlist("/unix", nl) < 0 && nl[0].n_value == 0)
		return (-1);
	if (nl[0].n_value == 0)
		return (-1);
	if ((fd = open("/dev/kmem", 0)) < 0)
		return (-1);

	base = nl[0].n_value;
	lseek(fd, base, 0);
	if (read(fd, tab, PRSIZE) != PRSIZE) {
		close(fd);
		return (-1);
	}
	/* entry 0 is IP: type, family, protocol, flags 0 and an output routine */
	if (getlong(tab) != 0 || getlong(tab + 12) == 0) {
		/* protoswLAST: walk back from the last entry to entry 0 */
		last = getlong(tab);
		for (i = 0; i < NPRSCAN; i++) {
			base = last - (long)i * PRSIZE;
			lseek(fd, base, 0);
			if (read(fd, tab, PRSIZE) != PRSIZE)
				break;
			if (getlong(tab) == 0 && getlong(tab + 12) != 0)
				break;
		}
		if (i == NPRSCAN) {
			close(fd);
			return (-1);
		}
	}

	lseek(fd, base, 0);
	if (read(fd, tab, sizeof (tab)) != sizeof (tab)) {
		close(fd);
		return (-1);
	}
	close(fd);
	for (i = 0, e = tab; i < NPRSCAN; i++, e += PRSIZE)
		if (getshort(e) == SOCK_RAW && getshort(e + 2) == PF_INET &&
		    getshort(e + 4) == IPPROTO_ICMP && getlong(e + 24) != 0)
			return (1);
	return (0);
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

/* read or write n bytes of kernel memory; returns n on success */
kmem(addr, buf, n, wr)
long addr;
char *buf;
int n, wr;
{
	int fd, r;

	if (addr == 0 || (fd = open("/dev/kmem", wr ? 2 : 0)) < 0)
		return (-1);
	lseek(fd, addr, 0);
	r = wr ? write(fd, buf, n) : read(fd, buf, n);
	close(fd);
	return (r);
}

/* set an int kernel variable */
kflag(name, v)
char *name;
int v;
{
	char b[4];

	b[0] = v >> 24;
	b[1] = v >> 16;
	b[2] = v >> 8;
	b[3] = v;
	if (kmem(ksym(name), b, 4, 1) != 4)
		fprintf(stderr, "ping: can't set %s\n", name);
}

/* -d: the kernel state that decides whether a reply reaches the socket */
kdump(when)
char *when;
{
	char q[20];

	if (kmem(ksym("_rawintr"), q, 20, 0) == 20)
		printf("%s: rawintrq len %ld maxlen %ld drops %ld\n", when,
		    getlong(q + 8), getlong(q + 12), getlong(q + 16));
	else
		printf("%s: can't read rawintrq\n", when);
	if (kmem(ksym("_netisr"), q, 4, 0) == 4)
		printf("%s: netisr %lx\n", when, getlong(q));
}

unsigned short
cksum(p, n)
register char *p;
register int n;
{
	register unsigned long sum = 0;

	while (n > 1) {
		sum += ((p[0] & 0xff) << 8) + (p[1] & 0xff);
		p += 2;
		n -= 2;
	}
	if (n)
		sum += (p[0] & 0xff) << 8;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return ((unsigned short)(~sum & 0xffff));
}

ontimer()
{
	timedout = 1;
	signal(SIGALRM, ontimer);
}

prtaddr(a)
unsigned long a;
{
	printf("%d.%d.%d.%d", (int)((a >> 24) & 0xff), (int)((a >> 16) & 0xff),
	    (int)((a >> 8) & 0xff), (int)(a & 0xff));
}

main(argc, argv)
int argc;
char *argv[];
{
	struct sockproto sp;
	struct sockaddr_in to, from;
	struct tms tms;
	char pkt[8 + DATALEN], buf[512];
	register char *p;
	register int i;
	unsigned long addr;
	unsigned short sum;
	long ticks;
	int s, n, count, seq, rseq, id, got, nrecv, ndup, wait, last;

	wait = 2;
	while (argc > 1 && argv[1][0] == '-') {
		if (strcmp(argv[1], "-d") == 0)
			debug = 1;
		else if (strcmp(argv[1], "-w") == 0 && argc > 2 &&
		    (wait = atoi(argv[2])) > 0) {
			argv++;
			argc--;
		} else
			argc = 0;		/* usage */
		argv++;
		argc--;
	}
	if (argc < 2 || argc > 3 || parseaddr(argv[1], &addr) < 0 ||
	    (argc == 3 && ((count = atoi(argv[2])) <= 0 || count > MAXSEQ))) {
		fprintf(stderr, "usage: ping [-d] [-w seconds] a.b.c.d [count]\n");
		fprintf(stderr, "count is at most %d\n", MAXSEQ);
		exit(2);
	}
	if (argc == 2)
		count = 4;

	switch (chkkern()) {
	case 0:
		fprintf(stderr, "ping: this kernel has no raw ICMP socket entry in proto.c;\n");
		fprintf(stderr, "ping: socket() would crash it.  Rebuild unix.net first.\n");
		exit(1);
	case -1:
		fprintf(stderr, "ping: can't read the protocol table from /unix and /dev/kmem\n");
		exit(1);
	}

	sp.sp_family = PF_INET;
	sp.sp_protocol = IPPROTO_ICMP;
	s = socket(SOCK_RAW, &sp, (struct sockaddr *)0, 0);
	if (s < 0)
		fail("ping: socket");

	p = (char *)&to;
	for (i = 0; i < sizeof (to); i++)
		*p++ = 0;
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = htonl(addr);

	if (debug) {
		kdump("before");
		kflag("_icmppri", 1);
	}

	id = getpid() & 0xffff;
	signal(SIGALRM, ontimer);
	printf("PING ");
	prtaddr(addr);
	printf(": %d data bytes\n", DATALEN);

	nrecv = ndup = 0;
	for (seq = 1; seq <= count; seq++) {
		pkt[0] = 8;			/* echo request */
		pkt[1] = 0;
		pkt[2] = pkt[3] = 0;
		pkt[4] = id >> 8;
		pkt[5] = id;
		pkt[6] = seq >> 8;
		pkt[7] = seq;
		for (i = 0; i < DATALEN; i++)
			pkt[8 + i] = i;		/* pkt[8] must stay 0, see above */
		sum = cksum(pkt, sizeof (pkt));
		pkt[2] = sum >> 8;
		pkt[3] = sum;

		sentat[seq] = times(&tms);
		seen[seq] = 0;
		if (send(s, (struct sockaddr *)&to, pkt, sizeof (pkt)) != sizeof (pkt))
			fail("ping: send");

		/*
		 * Wait for this request's reply.  Replies to earlier requests
		 * that come in meanwhile still count.  After the last request,
		 * keep listening for stragglers until the wait runs out.
		 */
		last = (seq == count);
		got = 0;
		timedout = 0;
		alarm(wait);
		while (!timedout && !(got && !(last && nrecv < count))) {
			n = receive(s, (struct sockaddr *)&from, buf, sizeof (buf));
			if (n < 0) {
				if (errno == EINTR)
					break;
				fail("ping: receive");
			}
			if (n < 8 || buf[0] != 0 || (getshort(buf + 4) & 0xffff) != id)
				continue;	/* not a reply to us */
			rseq = getshort(buf + 6);
			if (rseq < 1 || rseq > seq)
				continue;
			ticks = times(&tms) - sentat[rseq];
			printf("%d bytes from ", n);
			prtaddr(ntohl(from.sin_addr.s_addr));
			printf(": icmp_seq=%d time=%ld ms%s\n", rseq, ticks * 1000 / HZ,
			    seen[rseq] ? " (DUP!)" : "");
			if (seen[rseq])
				ndup++;
			else {
				seen[rseq] = 1;
				nrecv++;
			}
			if (rseq == seq)
				got = 1;
		}
		alarm(0);
		if (!seen[seq])
			printf("no reply for icmp_seq=%d within %d s\n", seq, wait);
		if (!last)
			sleep(1);
	}

	printf("--- ");
	prtaddr(addr);
	printf(" ping: %d sent, %d received", count, nrecv);
	if (ndup)
		printf(", %d duplicates", ndup);
	printf("\n");
	if (debug) {
		kflag("_icmppri", 0);
		kdump("after");
	}
	close(s);
	exit(nrecv ? 0 : 1);
}
