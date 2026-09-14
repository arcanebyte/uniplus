/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sethostent.c	6.3 (Berkeley) 4/10/86";
#endif LIBC_SCCS and not lint

/*
 * Lisa: from 4.3BSD.  Without a virtual circuit, sethostent(1) only keeps
 * the datagram socket open.  sethostfile() did nothing and its name
 * clashes with sethostname() in 7 characters, so it is left out.
 */

#include <sys/types.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>

sethostent(stayopen)
{
	if (stayopen)
		_res.options |= RES_STAYOPEN;
}

endhostent()
{
	_res.options &= ~RES_STAYOPEN;
	_res_close();
}

