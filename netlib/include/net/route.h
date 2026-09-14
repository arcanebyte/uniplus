/*
 * net/route.h -- routing table entry for SIOCADDRT/SIOCDELRT
 *
 * [K] Layout from the kernel's include/net/route.h (4.1a BSD, route.h
 * 82/06 era): a 48 byte entry with the key and value as struct sockaddr.
 * The kernel keeps entries in two hash tables of mbuf chains, rthost[]
 * and rtnet[], RTHASHSIZ (7) buckets each.
 */

#ifndef	_ROUTE_
#define	_ROUTE_

struct rtentry {
	unsigned long	rt_hash;	/* [K] host or network number */
	struct	sockaddr rt_dst;	/* [K] destination */
	struct	sockaddr rt_gateway;	/* [K] next hop, or the interface address */
	short	rt_flags;		/* [K] RTF_ flags */
	short	rt_refcnt;		/* [K] routes in use by sockets */
	unsigned long	rt_use;		/* [K] packets sent */
	char	*rt_ifp;		/* [K] kernel struct ifnet * */
};

#define	RTF_UP		0x1		/* [K] route usable */
#define	RTF_GATEWAY	0x2		/* [K] destination is reached through a gateway */
#define	RTF_HOST	0x4		/* [K] host route (network route otherwise) */

#define	RTHASHSIZ	7		/* [K] */

#endif
