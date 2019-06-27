/*	ip_input.c	1.65	83/02/23	*/
#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/ip.h"
#include "net/ip_icmp.h"

#include "net/if.h"
#include "net/route.h"

#include "net/in_pcb.h"
#include "net/ip_var.h"
#include "net/tcp.h"

u_char	ip_protox[IPPROTO_MAX];
int	ipqmaxlen = IFQ_MAXLEN;
struct	ifnet *ifinet;			/* first inet interface */

/*
 * IP initialization: fill in IP protocol switch table.
 * All protocols not implemented in kernel go to raw IP protocol handler.
 */
ip_init()
{
	register struct protosw *pr;
	register int i;

	pr = pffindproto(PF_INET, IPPROTO_RAW);
	if (pr == 0)
		panic("ip_init");
	for (i = 0; i < IPPROTO_MAX; i++)
		ip_protox[i] = pr - inetsw;
	/* billn  - meld with old code
	for (pr = inetdomain.dom_protosw;
	    pr <= inetdomain.dom_protoswNPROTOSW; pr++)
		if (pr->pr_family == PF_INET &&
		    pr->pr_protocol && pr->pr_protocol != IPPROTO_RAW)
			ip_protox[pr->pr_protocol] = pr - inetsw;
	*/
	for (pr = protosw; pr <= protoswLAST; pr++)
		if (pr->pr_family == PF_INET &&
		    pr->pr_protocol && pr->pr_protocol != IPPROTO_RAW)
			ip_protox[pr->pr_protocol] = pr - protosw;
	ipq.next = ipq.prev = &ipq;
	/* billn...
	ip_id = time.tv_sec & 0xffff;
	*/
	ip_id = time & 0xffff;
	ipintrq.ifq_maxlen = ipqmaxlen;
	ifinet = if_ifwithaf(AF_INET);
}

short	ipcksum = 1;
struct	ip *ip_reass();
struct	sockaddr_in ipaddr = { AF_INET };

/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassamble.  If complete and fragment queue exists, discard.
 * Process options.  Pass to next level.
 */
ipintr()
{
	register struct ip *ip;
	register struct mbuf *m;
	struct mbuf *m0;
	register int i;
	register struct ipq *fp;
	int hlen, s;

next:
	/*
	 * Get next datagram off input queue and get IP header
	 * in first mbuf.
	 */
	s = splimp();
	IF_DEQUEUE(&ipintrq, m);
	splx(s);
	if (m == 0)
		return;
	if ((m->m_off > MMAXOFF || m->m_len < sizeof (struct ip)) &&
	    (m = m_pullup(m, sizeof (struct ip))) == 0) {
		ipstat.ips_toosmall++;
		goto next;
	}
	ip = mtod(m, struct ip *);
	if ((hlen = ip->ip_hl << 2) > m->m_len) {
		if ((m = m_pullup(m, hlen)) == 0) {
			ipstat.ips_badhlen++;
			goto next;
		}
		ip = mtod(m, struct ip *);
	}
	if (ipcksum)
		if (ip->ip_sum = in_cksum(m, hlen)) {
			ipstat.ips_badsum++;
			goto bad;
		}

	/*
	 * Convert fields to host representation.
	 */
	ip->ip_len = ntohs((u_short)ip->ip_len);
	if (ip->ip_len < hlen) {
		ipstat.ips_badlen++;
		goto bad;
	}
	ip->ip_id = ntohs(ip->ip_id);
	ip->ip_off = ntohs((u_short)ip->ip_off);

	/*
	 * Check that the amount of data in the buffers
	 * is as at least much as the IP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	i = -ip->ip_len;
	m0 = m;
	for (;;) {
		i += m->m_len;
		if (m->m_next == 0)
			break;
		m = m->m_next;
	}
	if (i != 0) {
		if (i < 0) {
			ipstat.ips_tooshort++;
printf("[iptooshort by %d]", i);
			goto bad;
		}
		if (i <= m->m_len)
			m->m_len -= i;
		else
			m_adj(m0, -i);
	}
	m = m0;

	/*
	 * Process options and, if not destined for us,
	 * ship it on.  ip_dooptions returns 1 when an
	 * error was detected (causing an icmp message
	 * to be sent).
	 */
	if (hlen > sizeof (struct ip) && ip_dooptions(ip))
		goto next;

	/*
	 * Fast check on the first internet
	 * interface in the list.
	 */
	if (ifinet) {
		struct sockaddr_in *sin;

		sin = (struct sockaddr_in *)&ifinet->if_addr;
		if (sin->sin_addr.s_addr == ip->ip_dst.s_addr)
			goto ours;
		sin = (struct sockaddr_in *)&ifinet->if_broadaddr;
		if ((ifinet->if_flags & IFF_BROADCAST) &&
		    sin->sin_addr.s_addr == ip->ip_dst.s_addr)
			goto ours;
	}
/* BEGIN GROT */
#if NND > 0
#include "nd.h"
	/*
	 * Diskless machines don't initially know
	 * their address, so take packets from them
	 * if we're acting as a network disk server.
	 */
	if (ip->ip_dst.s_addr == INADDR_ANY &&
	    (in_netof(ip->ip_src) == INADDR_ANY &&
	     in_lnaof(ip->ip_src) != INADDR_ANY))
		goto ours;
#endif
/* END GROT */
	ipaddr.sin_addr = ip->ip_dst;
	if (if_ifwithaddr((struct sockaddr *)&ipaddr) == 0) {
		ip_forward(ip);
		goto next;
	}

ours:
	/*
	 * Look for queue of fragments
	 * of this datagram.
	 */
	for (fp = ipq.next; fp != &ipq; fp = fp->next)
		if (ip->ip_id == fp->ipq_id &&
		    ip->ip_src.s_addr == fp->ipq_src.s_addr &&
		    ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
		    ip->ip_p == fp->ipq_p)
			goto found;
	fp = 0;
found:

	/*
	 * Adjust ip_len to not reflect header,
	 * set ip_mff if more fragments are expected,
	 * convert offset of this to bytes.
	 */
	ip->ip_len -= hlen;
	((struct ipasfrag *)ip)->ipf_mff = 0;
	if (ip->ip_off & IP_MF)
		((struct ipasfrag *)ip)->ipf_mff = 1;
	ip->ip_off <<= 3;

	/*
	 * If datagram marked as having more fragments
	 * or if this is not the first fragment,
	 * attempt reassembly; if it succeeds, proceed.
	 */
	if (((struct ipasfrag *)ip)->ipf_mff || ip->ip_off) {
		ip = ip_reass((struct ipasfrag *)ip, fp);
		if (ip == 0)
			goto next;
		hlen = ip->ip_hl << 2;
		m = dtom(ip);
	} else
		if (fp)
			ip_freef(fp);

	/*
	 * Switch out to protocol's input routine.
	 */
	(*inetsw[ip_protox[ip->ip_p]].pr_input)(m);
	goto next;
bad:
	m_freem(m);
	goto next;
}

/*
 * Take incoming datagram fragment and try to
 * reassemble it into whole datagram.  If a chain for
 * reassembly of this datagram already exists, then it
 * is given as fp; otherwise have to make a chain.
 */
struct ip *
ip_reass(ip, fp)
	register struct ipasfrag *ip;
	register struct ipq *fp;
{
	register struct mbuf *m = dtom(ip);
	register struct ipasfrag *q;
	struct mbuf *t;
	int hlen = ip->ip_hl << 2;
	int i, next;

	/*
	 * Presence of header sizes in mbufs
	 * would confuse code below.
	 */
	m->m_off += hlen;
	m->m_len -= hlen;

	/*
	 * If first fragment to arrive, create a reassembly queue.
	 */
	if (fp == 0) {
		/* billn -- meld with old
		if ((t = m_get(M_WAIT, MT_FTABLE)) == NULL)
		*/
		if ((t = m_get(M_WAIT)) == NULL)
			goto dropfrag;
		fp = mtod(t, struct ipq *);
		insque(fp, &ipq);
		fp->ipq_ttl = IPFRAGTTL;
		fp->ipq_p = ip->ip_p;
		fp->ipq_id = ip->ip_id;
		fp->ipq_next = fp->ipq_prev = (struct ipasfrag *)fp;
		fp->ipq_src = ((struct ip *)ip)->ip_src;
		fp->ipq_dst = ((struct ip *)ip)->ip_dst;
		q = (struct ipasfrag *)fp;
		goto insert;
	}

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = q->ipf_next)
		if (q->ip_off > ip->ip_off)
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if (q->ipf_prev != (struct ipasfrag *)fp) {
		i = q->ipf_prev->ip_off + q->ipf_prev->ip_len - ip->ip_off;
		if (i > 0) {
			if (i >= ip->ip_len)
				goto dropfrag;
			m_adj(dtom(ip), i);
			ip->ip_off += i;
			ip->ip_len -= i;
		}
	}

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct ipasfrag *)fp && ip->ip_off + ip->ip_len > q->ip_off) {
		i = (ip->ip_off + ip->ip_len) - q->ip_off;
		if (i < q->ip_len) {
			q->ip_len -= i;
			q->ip_off += i;
			m_adj(dtom(q), i);
			break;
		}
		q = q->ipf_next;
		m_freem(dtom(q->ipf_prev));
		ip_deq(q->ipf_prev);
	}

insert:
	/*
	 * Stick new segment in its place;
	 * check for complete reassembly.
	 */
	ip_enq(ip, q->ipf_prev);
	next = 0;
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = q->ipf_next) {
		if (q->ip_off != next)
			return (0);
		next += q->ip_len;
	}
	if (q->ipf_prev->ipf_mff)
		return (0);

	/*
	 * Reassembly is complete; concatenate fragments.
	 */
	q = fp->ipq_next;
	m = dtom(q);
	t = m->m_next;
	m->m_next = 0;
	m_cat(m, t);
	q = q->ipf_next;
	while (q != (struct ipasfrag *)fp) {
		t = dtom(q);
		q = q->ipf_next;
		m_cat(m, t);
	}

	/*
	 * Create header for new ip packet by
	 * modifying header of first packet;
	 * dequeue and discard fragment reassembly header.
	 * Make header visible.
	 */
	ip = fp->ipq_next;
	ip->ip_len = next;
	((struct ip *)ip)->ip_src = fp->ipq_src;
	((struct ip *)ip)->ip_dst = fp->ipq_dst;
	remque(fp);
	(void) m_free(dtom(fp));
	m = dtom(ip);
	m->m_len += sizeof (struct ipasfrag);
	m->m_off -= sizeof (struct ipasfrag);
	return ((struct ip *)ip);

dropfrag:
	m_freem(m);
	return (0);
}

/*
 * Free a fragment reassembly header and all
 * associated datagrams.
 */
ip_freef(fp)
	struct ipq *fp;
{
	register struct ipasfrag *q, *p;

	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = p) {
		p = q->ipf_next;
		ip_deq(q);
		m_freem(dtom(q));
	}
	remque(fp);
	(void) m_free(dtom(fp));
}

/*
 * Put an ip fragment on a reassembly chain.
 * Like insque, but pointers in middle of structure.
 */
ip_enq(p, prev)
	register struct ipasfrag *p, *prev;
{

	p->ipf_prev = prev;
	p->ipf_next = prev->ipf_next;
	prev->ipf_next->ipf_prev = p;
	prev->ipf_next = p;
}

/*
 * To ip_enq as remque is to insque.
 */
ip_deq(p)
	register struct ipasfrag *p;
{

	p->ipf_prev->ipf_next = p->ipf_next;
	p->ipf_next->ipf_prev = p->ipf_prev;
}

/*
 * IP timer processing;
 * if a timer expires on a reassembly
 * queue, discard it.
 */
ip_slowtimo()
{
	register struct ipq *fp;
	int s = splnet();

	fp = ipq.next;
	if (fp == 0) {
		splx(s);
		return;
	}
	while (fp != &ipq) {
		--fp->ipq_ttl;
		fp = fp->next;
		if (fp->prev->ipq_ttl == 0)
			ip_freef(fp->prev);
	}
	splx(s);
}

/*
 * Drain off all datagram fragments.
 */
ip_drain()
{

	while (ipq.next != &ipq)
		ip_freef(ipq.next);
}

/*
 * Do option processing on a datagram,
 * possibly discarding it if bad options
 * are encountered.
 */
ip_dooptions(ip)
	struct ip *ip;
{
	register u_char *cp;
	int opt, optlen, cnt, code, type;
	struct in_addr *sin;
	register struct ip_timestamp *ipt;
	register struct ifnet *ifp;
	struct in_addr t;

	cp = (u_char *)(ip + 1);
	cnt = (ip->ip_hl << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else
			optlen = cp[1];
		switch (opt) {

		default:
			break;

		/*
		 * Source routing with record.
		 * Find interface with current destination address.
		 * If none on this machine then drop if strictly routed,
		 * or do nothing if loosely routed.
		 * Record interface address and bring up next address
		 * component.  If strictly routed make sure next
		 * address on directly accessible net.
		 */
		case IPOPT_LSRR:
		case IPOPT_SSRR:
			if (cp[2] < 4 || cp[2] > optlen - (sizeof (long) - 1))
				break;
			sin = (struct in_addr *)(cp + cp[2]);
			ipaddr.sin_addr = *sin;
			ifp = if_ifwithaddr((struct sockaddr *)&ipaddr);
			type = ICMP_UNREACH, code = ICMP_UNREACH_SRCFAIL;
			if (ifp == 0) {
				if (opt == IPOPT_SSRR)
					goto bad;
				break;
			}
			t = ip->ip_dst; ip->ip_dst = *sin; *sin = t;
			cp[2] += 4;
			if (cp[2] > optlen - (sizeof (long) - 1))
				break;
			ip->ip_dst = sin[1];
			if (opt == IPOPT_SSRR &&
			    if_ifonnetof((int)(in_netof(ip->ip_dst))) == 0)
				goto bad;
			break;

		case IPOPT_TS:
			code = cp - (u_char *)ip;
			type = ICMP_PARAMPROB;
			ipt = (struct ip_timestamp *)cp;
			if (ipt->ipt_len < 5)
				goto bad;
			if (ipt->ipt_ptr > ipt->ipt_len - sizeof (long)) {
				if (++ipt->ipt_oflw == 0)
					goto bad;
				break;
			}
			sin = (struct in_addr *)(cp+cp[2]);
			switch (ipt->ipt_flg) {

			case IPOPT_TS_TSONLY:
				break;

			case IPOPT_TS_TSANDADDR:
				if (ipt->ipt_ptr + 8 > ipt->ipt_len)
					goto bad;
				if (ifinet == 0)
					goto bad;	/* ??? */
				*sin++ = ((struct sockaddr_in *)&ifinet->if_addr)->sin_addr;
				break;

			case IPOPT_TS_PRESPEC:
				ipaddr.sin_addr = *sin;
				if (if_ifwithaddr((struct sockaddr *)&ipaddr) == 0)
					continue;
				if (ipt->ipt_ptr + 8 > ipt->ipt_len)
					goto bad;
				ipt->ipt_ptr += 4;
				break;

			default:
				goto bad;
			}
			*(n_time *)sin = iptime();
			ipt->ipt_ptr += 4;
		}
	}
	return (0);
bad:
	icmp_error(ip, type, code);
	return (1);
}

/*
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Second argument is buffer to which options
 * will be moved, and return value is their length.
 */
ip_stripoptions(ip, mopt)
	struct ip *ip;
	struct mbuf *mopt;
{
	register int i;
	register struct mbuf *m;
	int olen;

	olen = (ip->ip_hl<<2) - sizeof (struct ip);
	m = dtom(ip);
	ip++;
	if (mopt) {
		mopt->m_len = olen;
		mopt->m_off = MMINOFF;
		bcopy((caddr_t)ip, mtod(m, caddr_t), (int)olen);
	}
	i = m->m_len - (sizeof (struct ip) + olen);
	bcopy((caddr_t)ip+olen, (caddr_t)ip, (int)i);
	m->m_len -= olen;
}

u_char inetctlerrmap[] = {
	ECONNABORTED,	ECONNABORTED,	0,		0,
	0,		0,
	EHOSTDOWN,	EHOSTUNREACH,	ENETUNREACH,	EHOSTUNREACH,
	ECONNREFUSED,	ECONNREFUSED,	EMSGSIZE,	0,
	0,		0,		0,		0
};

#ifdef notdef
ip_ctlinput(cmd, arg)
	int cmd;
	caddr_t arg;
{
	struct in_addr *in;
	int tcp_abort(), udp_abort();
	extern struct inpcb tcb, udb;

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;
	if (inetctlerrmap[cmd] == 0)
		return;		/* XXX */
	if (cmd == PRC_IFDOWN)
		in = &((struct sockaddr_in *)arg)->sin_addr;
	else if (cmd == PRC_HOSTDEAD || cmd == PRC_HOSTUNREACH)
		in = (struct in_addr *)arg;
	else
		in = &((struct icmp *)arg)->icmp_ip.ip_dst;
/* THIS IS VERY QUESTIONABLE, SHOULD HIT ALL PROTOCOLS */
	in_pcbnotify(&tcb, in, (int)inetctlerrmap[cmd], tcp_abort);
	in_pcbnotify(&udb, in, (int)inetctlerrmap[cmd], udp_abort);
}
#endif

int	ipprintfs = 0;
int	ipforwarding = 1;
/*
 * Forward a packet.  If some error occurs return the sender
 * and icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repetoire
 * of codes and types.
 */
ip_forward(ip)
	register struct ip *ip;
{
	register int error, type, code;
	struct mbuf *mopt, *mcopy;

	if (ipprintfs)
		printf("forward: src %x dst %x ttl %x\n", ip->ip_src,
			ip->ip_dst, ip->ip_ttl);
	if (ipforwarding == 0) {
		/* can't tell difference between net and host */
		type = ICMP_UNREACH, code = ICMP_UNREACH_NET;
		goto sendicmp;
	}
	if (ip->ip_ttl < IPTTLDEC) {
		type = ICMP_TIMXCEED, code = ICMP_TIMXCEED_INTRANS;
		goto sendicmp;
	}
	ip->ip_ttl -= IPTTLDEC;
	/* billn -- meld with old
	mopt = m_get(M_DONTWAIT, MT_DATA);
	*/
	mopt = m_get(M_DONTWAIT);
	if (mopt == NULL) {
		m_freem(dtom(ip));
		return;
	}

	/*
	 * Save at most 64 bytes of the packet in case
	 * we need to generate an ICMP message to the src.
	 */
	/* billn -- imin -> MIN */
	mcopy = m_copy(dtom(ip), 0, MIN(ip->ip_len, 64));
	ip_stripoptions(ip, mopt);

	/* last 0 here means no directed broadcast */
	if ((error = ip_output(dtom(ip), mopt, (struct route *)0, 0)) == 0) {
		if (mcopy)
			m_freem(mcopy);
		return;
	}
	ip = mtod(mcopy, struct ip *);
	type = ICMP_UNREACH, code = 0;		/* need ``undefined'' */
	switch (error) {

	case ENETUNREACH:
	case ENETDOWN:
		code = ICMP_UNREACH_NET;
		break;

	case EMSGSIZE:
		code = ICMP_UNREACH_NEEDFRAG;
		break;

	/* billn - meld with old
	case EPERM:
		code = ICMP_UNREACH_PORT;
		break;

	*/
	case ENOBUFS:
		type = ICMP_SOURCEQUENCH;
		break;

	case EHOSTDOWN:
	case EHOSTUNREACH:
		code = ICMP_UNREACH_HOST;
		break;
	}
sendicmp:
	icmp_error(ip, type, code);
}
