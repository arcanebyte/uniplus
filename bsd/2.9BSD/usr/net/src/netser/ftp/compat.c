#include <sys/types.h>
#include <sys/socket.h>
#include <net/in.h>
#include <netdb.h>
/*
 * Must not include ftp.h (which includes compat.h),
 * so fake network system calls (accept, connect, etc.) work. 
 */

bzero (what, size)
register char *what, *size;
{
	while (size-- > 0)
		*what++ = 0;
}

bcopy (from, to, size)
register char *from, *to;
register int size;
{
	while (size-- > 0)
		*to++ = *from++;
}

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
	return (connect (fd, what, flag));
}
