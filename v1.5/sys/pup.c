/*	pup.c	4.2	82/06/20	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/af.h"
#include "net/pup.h"

#ifdef PUP
pup_hash(spup, hp)
	struct sockaddr_pup *spup;
	struct afhash *hp;
{
	hp->afh_nethash = spup->spup_addr.pp_net;
	hp->afh_hosthash = spup->spup_addr.pp_host;
	if (hp->afh_hosthash < 0)
		hp->afh_hosthash = -hp->afh_hosthash;
}

pup_netmatch(spup1, spup2)
	struct sockaddr_pup *spup1, *spup2;
{
	return (spup1->spup_addr.pp_net == spup2->spup_addr.pp_net);
}
#endif
