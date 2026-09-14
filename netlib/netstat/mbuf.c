#ifndef lint
static char sccsid[] = "@(#)mbuf.c	4.1 82/08/25";
#endif

/* Lisa: the kernel's headers, from /usr/src/include; see main.c. */
#include <sys/types.h>
#include "net/misc.h"
#define	U_LONG_DEFINED		/* net/misc.h has u_long and friends */
#include "net/mbuf.h"

struct	mbstat mbstat;
extern	int kmem;

/*
 * Print mbuf statistics.
 */
mbpr(mbaddr)
	off_t mbaddr;
{
	if (mbaddr == 0) {
		printf("mbstat: symbol not in namelist\n");
		return;
	}
	printf("mbufs:");
	klseek(kmem, (off_t)mbaddr, 0);
	if (read(kmem, &mbstat, sizeof (mbstat)) == sizeof (mbstat))
		printf(
	" mbufs %d mbfree %d clusters %d clfree %d drops %d\n",
		mbstat.m_mbufs, mbstat.m_mbfree,
		mbstat.m_clusters, mbstat.m_clfree, mbstat.m_drops);
}
