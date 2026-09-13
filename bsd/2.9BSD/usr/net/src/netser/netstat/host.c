/*
 *	NOTE: not converted to the 11 !!!!
 *	dgc, tek, 2/83
 */

#ifndef lint
static char sccsid[] = "@(#)host.c	4.1 82/08/25";
#endif

#include <imp.h>
#if	NIMP > 0
#include <sys/types.h>
#include <sys/mbuf.h>
#include <net/in.h>
#include <net/if_imp.h>
#include <net/if_imphost.h>
#define	h_addr	h_xaddr
#include <netdb.h>
#undef	h_addr

extern	int kmem;
extern 	int nflag;
extern	char *inetname();

/*
 * Print the host tables associated with the ARPANET IMP.
 * Symbolic addresses are shown unless the nflag is given.
 */
hostpr(hostsaddr)
	off_t hostsaddr;
{
	struct mbuf *hosts, mb;
	register struct mbuf *m;
	register struct hmbuf *mh;
	register struct host *hp;
	struct hostent *p;
	char flagbuf[10], *flags;
	int first = 1;

	if (hostsaddr == 0) {
		printf("hosts: symbol not in namelist\n");
		return;
	}
	klseek(kmem, (off_t)hostsaddr, 0);
	read(kmem, &hosts, sizeof (hosts));
	m = hosts;
	printf("IMP Host Table\n");
	printf("%-5.5s %-15.15s %-4.4s %-9.9s %-4.4s %s\n",
		"Flags", "Host", "Qcnt", "Q Address", "RFNM", "Timer");
	while (m) {
		klseek(kmem, (off_t)m, 0);
		read(kmem, &mb, sizeof (mb));
		m = &mb;
		mh = mtod(m, struct hmbuf *);
		if (mh->hm_count == 0) {
			m = m->m_next;
			continue;
		}
		for (hp = mh->hm_hosts; hp < mh->hm_hosts + HPMBUF; hp++) {
			if ((hp->h_flags&HF_INUSE) == 0 && hp->h_timer == 0)
				continue;
			flags = flagbuf;
			*flags++ = hp->h_flags&HF_INUSE ? 'A' : 'F';
			if (hp->h_flags&HF_DEAD)
				*flags++ = 'D';
			if (hp->h_flags&HF_UNREACH)
				*flags++ = 'U';
			*flags = '\0';
			printf("%-5.5s %-15.15s %-4d %-9x %-4d %d\n",
				flagbuf,
				inetname(hp->h_addr),
				hp->h_qcnt,
				hp->h_q,
				hp->h_rfnm,
				hp->h_timer);
		}
		m = m->m_next;
	}
}
#endif	NIMP
