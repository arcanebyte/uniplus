/*	netdb.h	4.1	82/10/05	*/
/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	*h_addr;	/* address */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char	*n_name;	/* official name of net */
	char	**n_aliases;	/* alias list */
	int	n_addrtype;	/* net address type */
	long	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

#ifdef	pdp11
#define	gethostbyname	gethbyname
#define	gethostbyaddr	gethbyaddr
#define	gethostent	gethent
#define	getnetbyname	getnbyname
#define	getnetbyaddr	getnbyaddr
#define	getnetent	getnent
#define	getservbyname	getsbyname
#define	getservbyport	getsbyport
#define	getservent	getsent
#define	getprotobyname	getpbyname
#define	getprotobynumber getpbnumber
#define	getprotoent	getpent
#define	inet_netof	inet_nof
#define	inet_network	inet_nwk
#endif	pdp11

struct hostent	*gethostbyname(), *gethostbyaddr(), *gethostent();
struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
struct servent	*getservbyname(), *getservbyport(), *getservent();
struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();

long inet_addr();
long inet_netof();
long inet_network();
long rhost();	/* deprecated */
long inet_lnaof();
