/*	inet_lnaof.c	4.2	82/10/07	*/

#include <sys/types.h>
#include <net/in.h>
#include <netdb.h>

/*
 * Return the local network address portion of an
 * internet address; handles class a/b/c network
 * number formats.
 */
u_long
inet_lnaof(in)
	struct in_addr in;
{
#if	!pdp11
#define	IN_LNAOF(in) \
	(((in).s_addr&IN_CLASSA) == 0 ? (in).s_addr&IN_CLASSA_LNA : \
		((in).s_addr&IN_CLASSB) == 0 ? (in).s_addr&IN_CLASSB_LNA : \
			(in).s_addr&IN_CLASSC_LNA)
	return ((int)htonl((u_long)IN_LNAOF(in)));
#else
	u_long lna;

	in.s_addr = htonl(in.s_addr);
	lna = IN_LNAOF(in);
	lna = (long)(lna << 16) | (long)((lna >> 16) & 0xffff);
	in.s_addr = ntohl(in.s_addr);
	return(lna);
#endif
}
