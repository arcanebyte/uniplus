/*	af.c	4.7	82/10/17	*/
#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/af.h"

/*
 * Address family support routines
 */
int	null_hash(), null_netmatch();
#define	AFNULL \
	{ null_hash,	null_netmatch }

#ifdef INET
extern int inet_hash(), inet_netmatch();
#define	AFINET \
	{ inet_hash,	inet_netmatch }
#else
#define	AFINET	AFNULL
#endif

#ifdef PUP
extern int pup_hash(), pup_netmatch();
#define	AFPUP \
	{ pup_hash,	pup_netmatch }
#else
#define	AFPUP	AFNULL
#endif

struct afswitch afswitch[AF_MAX] = {
	AFNULL,	AFNULL,	AFINET,	AFINET,	AFPUP,
	AFNULL,	AFNULL,	AFNULL,	AFNULL,	AFNULL,
	AFNULL
};

/*ARGSUSED*/
null_hash(addr, hp)
	struct sockaddr *addr;
	struct afhash *hp;
{

	hp->afh_nethash = hp->afh_hosthash = 0;
}

/*ARGSUSED*/
null_netmatch(a1, a2)
	struct sockaddr *a1, *a2;
{

	return (0);
}
