/*
 * netdb.h -- network data base library for UniPlus+ on the Apple Lisa,
 * from the 4.1c BSD back-port in 2.9BSD (netdb.h 4.1 82/10/05), with
 * struct hostent and h_errno from 4.3BSD (netdb.h 5.7 86/05/12) for the
 * name server routines.
 *
 * Structures returned by the network data base library.  All addresses
 * are supplied in host order, and returned in network order (suitable
 * for use in system calls); on the 68000 the two are the same.
 *
 * gethostbyname() and gethostbyaddr() ask the name servers in
 * /etc/resolv.conf, then look in /etc/hosts.  The other data bases are
 * /etc/networks, /etc/protocols and /etc/services.
 */
#include "bsd.h"

struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
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

/*
 * The Lisa cc keeps 7 characters of an external name, so the long names
 * are renamed apart, as 2.9BSD did for the PDP-11 (and for the set/end
 * routines, which would clash with sethostname() and each other).
 */
#define	gethostbyname	gethbyname
#define	gethostbyaddr	gethbyaddr
#define	sethostent	sethent
#define	endhostent	endhent
#define	getnetbyname	getnbyname
#define	getnetbyaddr	getnbyaddr
#define	getnetent	getnent
#define	setnetent	setnent
#define	endnetent	endnent
#define	getservbyname	getsbyname
#define	getservbyport	getsbyport
#define	getservent	getsent
#define	setservent	setsent
#define	endservent	endsent
#define	getprotobyname	getpbyname
#define	getprotobynumber getpbnumber
#define	getprotoent	getpent
#define	setprotoent	setpent
#define	endprotoent	endpent
#define	inet_netof	inet_nof
#define	inet_network	inet_nwk
#define	_gethtbyname	_ghtbyname
#define	_gethtbyaddr	_ghtbyaddr

struct hostent	*gethostbyname(), *gethostbyaddr();
struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
struct servent	*getservbyname(), *getservbyport(), *getservent();
struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 */
extern  int h_errno;

#define	HOST_NOT_FOUND	1 /* Authoritive Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_ADDRESS	4 /* Valid host name, no address, look for MX record */

u_long	inet_addr();
u_long	inet_netof();
u_long	inet_network();
u_long	inet_lnaof();
char	*inet_ntoa();
/* after net/in.h:  struct in_addr inet_makeaddr(); */
