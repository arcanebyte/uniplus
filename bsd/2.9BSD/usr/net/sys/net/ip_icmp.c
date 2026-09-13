/*	ip_icmp.c	4.17	82/06/20	*/

#include "param.h"
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include "../net/in.h"
#include "../net/in_systm.h"
#include "../net/ip.h"
#include "../net/ip_icmp.h"

/*
 * ICMP routines: error generation, receive packet processing, and
 * routines to turnaround packets back to the originator, and
 * host table maintenance routines.
 */
int	icmpprintfs = 0;

/*
 * Generate an error packet of type error
 * in response to bad packet ip.
 */
icmp_error(oip, type, code)
	struct ip *oip;
	int type, code;
{
	register unsigned oiplen = oip->ip_hl << 2;
	register struct icmp *icp;
	struct mbuf *m;
	struct mbuf *om = dtom(oip);
	struct ip *nip;

	MAPSAVE();
	if (icmpprintfs)
		printf("icmp_error(%x, %d, %d)\n", oip, type, code);
	/*
	 * Make sure that the old IP packet had 8 bytes of data to return;
	 * if not, don't bother.  Also don't EVER error if the old
	 * packet protocol was ICMP.
	 */
	if (oip->ip_len < 8 || oip->ip_p == IPPROTO_ICMP)
		goto free;

	/*
	 * First, formulate icmp message
	 */
	m = m_get(M_DONTWAIT);
	if (m == 0)
		goto free;
	m->m_len = oiplen + 8 + ICMP_MINLEN;
	m->m_off = MMAXOFF - m->m_len;
	icp = mtod(m, struct icmp *);
	icp->icmp_type = type;
	icp->icmp_void = 0;
	if (type == ICMP_PARAMPROB) {
		icp->icmp_pptr = code;
		code = 0;
	}
	icp->icmp_code = code;
	MBCOPY(om, 0, m, (&icp->icmp_ip - icp), oiplen + 8);
	nip = &icp->icmp_ip;
	nip->ip_len += oiplen;
#if vax || pdp11
	nip->ip_len = htons((u_short)nip->ip_len);
#endif

	/*
	 * Now, copy old ip header in front of icmp
	 * message.  This allows us to reuse any source
	 * routing info present.
	 */
	m->m_off -= oiplen;
	nip = mtod(m, struct ip *);
	MBCOPY(om, 0, m, 0, oiplen);
	nip->ip_len = m->m_len + oiplen;
	nip->ip_p = IPPROTO_ICMP;
	/* icmp_send adds ip header to m_off and m_len, so we deduct here */
	m->m_off += oiplen;
	icmp_reflect(nip);

free:
	m_freem(om);
	MAPREST();
}

static char icmpmap[] = {
	-1,		 -1,		-1,
	PRC_UNREACH_NET, PRC_QUENCH, 	PRC_REDIRECT_NET,
	-1,		 -1,		-1,
	-1,		 -1,		PRC_TIMXCEED_INTRANS,
	PRC_PARAMPROB,	 -1,		-1,
	-1,		 -1
};

static struct sockproto icmproto = { AF_INET, IPPROTO_ICMP };
static struct sockaddr_in icmpsrc = { AF_INET };
static struct sockaddr_in icmpdst = { AF_INET };

/*
 * Process a received ICMP message.
 */
icmp_input(m)
	struct mbuf *m;
{
	register struct icmp *icp;
	register struct ip *ip = mtod(m, struct ip *);
	int icmplen = ip->ip_len, hlen = ip->ip_hl << 2, i, (*ctlfunc)();
	extern u_char ip_protox[];

	/*
	 * Locate icmp structure in mbuf, and check
	 * that not corrupted and of at least minimum length.
	 */
	if (icmpprintfs)
		printf("icmp_input from %u.%u.%u.%u, len %d\n",
		(unsigned)ip->ip_src.s_addr.s_net,
		(unsigned)ip->ip_src.s_addr.s_host,
		(unsigned)ip->ip_src.s_addr.s_lh,
		(unsigned)ip->ip_src.s_addr.s_impno,
		icmplen);
	if (icmplen < ICMP_MINLEN)
		goto free;
	m->m_len -= hlen;
	m->m_off += hlen;
	/* need routine to make sure header is in this mbuf here */
	icp = mtod(m, struct icmp *);
	i = icp->icmp_cksum;
	icp->icmp_cksum = 0;
	if (i != in_cksum(m, icmplen)) {
		printf("icmp: cksum %x\n", i);
		goto free;
	}

	/*
	 * Message type specific processing.
	 */
	if (icmpprintfs)
		printf("icmp_input, type %d code %d\n", icp->icmp_type,
			icp->icmp_code);
	switch (i = UCHAR(icp->icmp_type)) {

	case ICMP_UNREACH:
	case ICMP_TIMXCEED:
	case ICMP_PARAMPROB:
	case ICMP_SOURCEQUENCH:		/* BRL -- REDIRECT removed */
		/*
		 * Problem with previous datagram; advise
		 * higher level routines.
		 */
#if vax || pdp11
		icp->icmp_ip.ip_len = ntohs(icp->icmp_ip.ip_len);
#endif
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp))
			goto free;
		if (icmpprintfs)
			printf("deliver to protocol %d\n", icp->icmp_ip.ip_p);
		if (ctlfunc = protosw[ip_protox[UCHAR(icp->icmp_ip.ip_p)]].pr_ctlinput) {
#if !pdp11
			(*ctlfunc)(icmpmap[i] + icp->icmp_code, (caddr_t)icp);
#endif
#if pdp11
			struct icmp *icpcopy;
			icpcopy = (struct icmp *) m_sget(icmplen);
			if (icpcopy == 0) goto free;
			bcopy((caddr_t)icp, (caddr_t)icpcopy, icmplen);
			(*ctlfunc)(icmpmap[i] + icp->icmp_code, (caddr_t)icpcopy);
			m_sfree((caddr_t)icpcopy);
#endif pdp11
		}
		goto free;

	case ICMP_ECHO:
		icp->icmp_type = ICMP_ECHOREPLY;
		goto reflect;

	case ICMP_TSTAMP:
		if (icmplen < ICMP_TSLEN)
			goto free;
		icp->icmp_type = ICMP_TSTAMPREPLY;
		icp->icmp_rtime = iptime();
		icp->icmp_ttime = icp->icmp_rtime;	/* bogus, do later! */
		goto reflect;
		
	case ICMP_IREQ:
		/* fill in source address zero fields! */
		goto reflect;

	case ICMP_REDIRECT:		/* BRL */
	case ICMP_ECHOREPLY:
	case ICMP_TSTAMPREPLY:
	case ICMP_IREQREPLY:
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp))
			goto free;
		icmpsrc.sin_addr = ip->ip_src;
		icmpdst.sin_addr = ip->ip_dst;
		raw_input(m, &icmproto, (struct sockaddr *)&icmpsrc,
		  (struct sockaddr *)&icmpdst);
		return;		/* raw_input already freed it */

	default:
		goto free;
	}
reflect:
	ip->ip_len += hlen;		/* since ip_input deducts this */
	icmp_reflect(ip);
	return;				/* THIS ONE WAS MISSING */
free:
	m_freem(m);
}

/*
 * Reflect the ip packet back to the source
 * TODO: rearrange ip source routing options.
 */
icmp_reflect(ip)
	struct ip *ip;
{
	struct in_addr t;

	t = ip->ip_dst;
	ip->ip_dst = ip->ip_src;
	ip->ip_src = t;
	icmp_send(ip);
}

int	generateicmpmsgs = 1;

/*
 * Send an icmp packet back to the ip level,
 * after supplying a checksum.
 */
icmp_send(ip)
	struct ip *ip;
{
	register int hlen = ip->ip_hl << 2;
	register struct icmp *icp;
	register struct mbuf *m = dtom(ip);

	if (!generateicmpmsgs)
		return;
	icp = mtod(m, struct icmp *);
	icp->icmp_cksum = 0;
	icp->icmp_cksum = in_cksum(m, ip->ip_len - hlen);
	m->m_off -= hlen;
	m->m_len += hlen;
	if (icmpprintfs)
		printf("icmp_send dst %u.%u.%u.%u src %u.%u.%u.%u\n",
		 (unsigned)ip->ip_dst.s_addr.s_net,
		 (unsigned)ip->ip_dst.s_addr.s_host,
		 (unsigned)ip->ip_dst.s_addr.s_lh,
		 (unsigned)ip->ip_dst.s_addr.s_impno,
		 (unsigned)ip->ip_src.s_addr.s_net,
		 (unsigned)ip->ip_src.s_addr.s_host,
		 (unsigned)ip->ip_src.s_addr.s_lh,
		 (unsigned)ip->ip_src.s_addr.s_impno);
	(void) ip_output(m, 0, 0, 0);
}

n_time
iptime()
{
	int s = spl6();
	u_long t;

	t = (time % SECDAY) * 1000 + lbolt * hz;
	splx(s);
	return (htonl(t));
}
