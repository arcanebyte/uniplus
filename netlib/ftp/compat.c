/*
 * compat.c -- 4.2BSD-style socket calls on top of the 4.1a ones, from
 * 2.9BSD's ftp conversion.  Must not include ftp.h (which includes
 * compat.h), so the real accept() and connect() are called here.
 *
 * Lisa: bcopy/bzero come from bsd.h instead.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <net/in.h>
#include <netdb.h>

shutdown (fd, how)
int fd, *how;
{
}

bind (fd, where, much, opt)
int fd;
char *where;
int much;
int opt;
{
	return (fd);
}

listen (fd, timer)
int fd, timer;
{
	return (fd);
}

setsockopt (fd, type, flag, arg4, arg5)
{
	return (fd);
}

/* The 4.1a accept() connects the listening socket itself. */
Faccept (fd, what, size, flag)
int fd, flag;
char *what, *size;
{
	if (accept (fd, what))
		return (-1);
	return (dup (fd));
}

Fconnect (fd, what, size, flag)
int fd, size, flag;
char *what;
{
	return (connect (fd, what));
}
