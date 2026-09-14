
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)res_send.c	6.14 (Berkeley) 7/2/86";
#endif LIBC_SCCS and not lint

/*
 * Send query to name server and wait for reply.
 *
 * Lisa: from 4.3BSD, for the 4.1a socket calls: datagrams only, sent
 * with send() and read with receive(), which take the server's address
 * (no connect, sendto or recvfrom); select() takes long bit masks and a
 * timeout in milliseconds (and one more descriptor than it should need,
 * for older kernels' selscan()).  There is no virtual circuit (TCP), so a
 * truncated answer is returned as it is and a query longer than a
 * datagram fails with EMSGSIZE.  With no name server configured it
 * fails at once with ECONNREFUSED, what 4.3BSD's connected datagram
 * socket got from a host without named.  RES_STAYOPEN alone keeps the
 * socket open.
 */

#include <sys/types.h>
#include "bsd.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/nameser.h>
#include <resolv.h>

extern int errno;

static int s = -1;	/* socket used for communications */

#define KEEPOPEN RES_STAYOPEN

res_send(buf, buflen, answer, anslen)
	char *buf;
	int buflen;
	char *answer;
	int anslen;
{
	register int n;
	int retry, resplen, ns;
	int gotsomewhere = 0;
	u_short id;
	long dsmask, timeout;
	struct sockaddr_in from;
	HEADER *hp = (HEADER *) buf;
	HEADER *anhp = (HEADER *) answer;

#ifdef DEBUG
	if (_res.options & RES_DEBUG) {
		printf("res_send()\n");
		p_query(buf);
	}
#endif DEBUG
	if (!(_res.options & RES_INIT))
		if (res_init() == -1) {
			return(-1);
		}
	if (_res.nscount <= 0) {
		errno = ECONNREFUSED;
		return (-1);
	}
	if (buflen > PACKETSZ) {
		errno = EMSGSIZE;
		return (-1);
	}
	id = hp->id;
	/*
	 * Send request, RETRY times, or until successful
	 */
	for (retry = _res.retry; retry > 0; retry--) {
	   for (ns = 0; ns < _res.nscount; ns++) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("Querying server (# %d) address = %s\n", ns+1,
			      inet_ntoa(_res.nsaddr_list[ns].sin_addr));
#endif DEBUG
		/*
		 * Use datagrams.
		 */
		if (s < 0)
			s = socket(SOCK_DGRAM, (struct sockproto *)0,
			    (struct sockaddr *)0, 0);
		if (send(s, (struct sockaddr *)&_res.nsaddr_list[ns],
		    buf, buflen) != buflen) {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				perror("send");
#endif DEBUG
			continue;
		}
		/*
		 * Wait for reply
		 */
		timeout = (_res.retrans << (_res.retry - retry))
			/ _res.nscount;
		if (timeout <= 0)
			timeout = 1;
		timeout *= 1000L;
wait:
		dsmask = 1L << s;
		/*
		 * Lisa: s+2, as selscan() in kernels before the fix in
		 * syslocal.c never looked at descriptor nfds-1.
		 */
		n = select(s+2, &dsmask, (long *)NULL, timeout);
		if (n < 0) {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				perror("select");
#endif DEBUG
			continue;
		}
		if (n == 0) {
			/*
			 * timeout
			 */
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				printf("timeout\n");
#endif DEBUG
			gotsomewhere = 1;
			continue;
		}
		if ((resplen = receive(s, (struct sockaddr *)&from,
		    answer, anslen)) <= 0) {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				perror("receive");
#endif DEBUG
			continue;
		}
		gotsomewhere = 1;
		if (id != anhp->id) {
			/*
			 * response from old query, ignore it
			 */
#ifdef DEBUG
			if (_res.options & RES_DEBUG) {
				printf("old answer:\n");
				p_query(answer);
			}
#endif DEBUG
			goto wait;
		}
#ifdef DEBUG
		if (_res.options & RES_DEBUG) {
			printf("got answer:\n");
			p_query(answer);
		}
#endif DEBUG
		/*
		 * We are going to assume that the first server is preferred
		 * over the rest (i.e. it is on the local machine) and only
		 * keep that one open.
		 */
		if ((_res.options & KEEPOPEN) == KEEPOPEN && ns == 0) {
			return (resplen);
		} else {
			(void) close(s);
			s = -1;
			return (resplen);
		}
	   }
	}
	if (s >= 0) {
		(void) close(s);
		s = -1;
	}
	if (gotsomewhere == 0)
		errno = ECONNREFUSED;
	else
		errno = ETIMEDOUT;
	return (-1);
}

/*
 * This routine is for closing the socket if a virtual circuit is used and
 * the program wants to close it.  This provides support for endhostent()
 * which expects to close the socket.
 *
 * This routine is not expected to be user visible.
 */
_res_close()
{
	if (s != -1) {
		(void) close(s);
		s = -1;
	}
}
