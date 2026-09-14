/*
 * net/in.h -- reconstructed user-level Internet definitions for
 * UniPlus+ V.1.5+ (UCB_NET kernel).  See net/socket.h for the
 * [K]/[B] provenance tags.
 */

#define	sockaddr_in	sock_in		/* [K] cc tags are unique to 8 chars (net/misc.h) */
#define	IPPROTO_ICMP	1		/* [B] */
#define	IPPROTO_TCP	6		/* [B] */
#define	IPPROTO_UDP	17		/* [B] */
#define	IPPROTO_RAW	255		/* [B] */

#define	IPPORT_RESERVED	1024		/* [B] ports below need uid 0 (in_pcb.c) */

/*
 * Internet address, network byte order.  The kernel only uses
 * s_addr; 4.1a also overlaid byte and word views on it.
 */
struct in_addr {
	unsigned long	s_addr;		/* [K] */
};

#define	INADDR_ANY	0x00000000L	/* [K] */

/*
 * Class A/B/C network numbers, 68000 byte order: test the class with
 * (s_addr & IN_CLASSA) == 0 and so on, as in 4.1a.
 */
#define	IN_CLASSA	0x80000000L	/* [U] */
#define	IN_CLASSA_NET	0xff000000L	/* [U] 8 bits of net # */
#define	IN_CLASSA_LNA	0x00ffffffL	/* [U] */
#define	IN_CLASSB	0x40000000L	/* [U] */
#define	IN_CLASSB_NET	0xffff0000L	/* [U] 16 bits of net # */
#define	IN_CLASSB_LNA	0x0000ffffL	/* [U] */
#define	IN_CLASSC_NET	0xffffff00L	/* [U] 24 bits of net # */
#define	IN_CLASSC_LNA	0x000000ffL	/* [U] */

/*
 * Internet socket address.  Must be the same size as
 * struct sockaddr (16 bytes).
 */
struct sockaddr_in {
	short		sin_family;	/* [K] AF_INET */
	unsigned short	sin_port;	/* [K] network byte order */
	struct in_addr	sin_addr;	/* [K] network byte order */
	char		sin_zero[8];	/* [K] pad to 16 bytes */
};

/*
 * The 68000 is big-endian, so network order is host order.
 * (in_pcb.c on this kernel likewise treats htons() as a no-op.)
 */
#define	htons(x)	((unsigned short)(x))
#define	ntohs(x)	((unsigned short)(x))
#define	htonl(x)	((unsigned long)(x))
#define	ntohl(x)	((unsigned long)(x))
