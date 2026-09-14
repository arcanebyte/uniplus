/*
 * netdb.h -- network data base library for UniPlus+ on the Apple Lisa,
 * from the 4.1c BSD back-port in 2.9BSD (netdb.h 4.1 82/10/05).
 *
 * Structures returned by the network data base library.  All addresses
 * are supplied in host order, and returned in network order (suitable
 * for use in system calls); on the 68000 the two are the same.
 *
 * The data bases are /etc/hosts, /etc/networks, /etc/protocols and
 * /etc/services.  There is no name server: gethostbyname() only knows
 * the names in /etc/hosts.
 */
#include "bsd.h"

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

/*
 * The Lisa cc keeps 7 characters of an external name, so the long names
 * are renamed apart, as 2.9BSD did for the PDP-11 (and for the set/end
 * routines, which would clash with sethostname() and each other).
 */
#define	gethostbyname	gethbyname
#define	gethostbyaddr	gethbyaddr
#define	gethostent	gethent
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

struct hostent	*gethostbyname(), *gethostbyaddr(), *gethostent();
struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
struct servent	*getservbyname(), *getservbyport(), *getservent();
struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();

u_long	inet_addr();
u_long	inet_netof();
u_long	inet_network();
u_long	inet_lnaof();
char	*inet_ntoa();
/* after net/in.h:  struct in_addr inet_makeaddr(); */
