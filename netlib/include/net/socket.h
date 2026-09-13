/*
 * net/socket.h -- reconstructed user-level socket definitions for
 * UniPlus+ V.1.5+ with the UCB_NET kernel (unix.net).
 *
 * The original header did not survive.  This kernel is 4.1a BSD
 * derived (netipc.c is "ipc.c 4.20 82/06/20"), NOT 4.2BSD: there are
 * no bind(), listen(), shutdown(), setsockopt(), recvfrom() or
 * sendto() system calls.
 *
 * Network errno values (EWOULDBLOCK 55 .. ECONNREFUSED 81) are already
 * in the stock <sys/errno.h>, and libc's perror() knows their text.
 *
 * Provenance tags:
 *   [K] layout or value is forced by the surviving kernel source
 *   [B] value taken from 4.1a/4.1c BSD; the kernel only uses the
 *       symbol, so verify before relying on it
 */

/*
 * Socket types.  proto.c matches these against pr_type.
 */
#define	SOCK_STREAM	1		/* [B] TCP */
#define	SOCK_DGRAM	2		/* [B] UDP */
#define	SOCK_RAW	3		/* [B] raw IP (root only in practice) */

/*
 * Option flags, passed as the 4th argument of socket() and stored
 * directly in so_options.
 */
#define	SO_DEBUG	0x01		/* [B] record TCP trace */
#define	SO_ACCEPTCONN	0x02		/* [B] passive open ("listen") */
#define	SO_DONTLINGER	0x04		/* [B] don't wait for FIN ack on close */
#define	SO_KEEPALIVE	0x08		/* [B] keep connections alive */
#define	SO_DONTROUTE	0x10		/* [B] just use interface addresses */

/*
 * Protocol selector, optional 2nd argument of socket().
 * If a null pointer is passed the kernel uses PF_INET and the
 * default protocol for the socket type (socket.c: socreate).
 */
struct sockproto {
	short	sp_family;		/* [K] protocol family */
	short	sp_protocol;		/* [K] protocol within family */
};

#define	PF_UNSPEC	0		/* [B] */
#define	PF_INET		2		/* [B] */

/*
 * Generic socket address.  The kernel always copies exactly
 * sizeof (struct sockaddr) bytes in and out (netipc.c), and
 * soreceive() panics unless an address mbuf is this size.
 */
struct sockaddr {
	short	sa_family;		/* [K] address family */
	char	sa_data[14];		/* [K] 16 bytes total */
};

#define	AF_UNSPEC	0		/* [B] */
#define	AF_INET		2		/* [B] */

/*
 * System calls (see sockcall.s).  All return -1 and set errno on
 * failure.
 *
 *   s = socket(type, sockproto *, sockaddr *local, options)	 73
 *	local != 0 binds the local address/port;
 *	options & SO_ACCEPTCONN makes it a listening socket.
 *   accept(s, sockaddr *peer)					 74
 *	waits until s ITSELF becomes connected; returns 0, not a
 *	new descriptor.  s stops listening once connected.
 *   connect(s, sockaddr *remote)				 75
 *   n = receive(s, sockaddr *from, buf, len)			 76
 *   n = send(s, sockaddr *to, buf, len)			 77
 *	from/to may be 0; read()/write() also work on a socket.
 *   socketaddr(s, sockaddr *local)				 78
 *   select(nfd, long *rmask, long *wmask, long timeout_ms)	 70
 *   gethostname(buf, len)					 71
 *   sethostname(buf, len)					 72
 *   netreset()							 79
 */
extern int	socket(), accept(), connect(), receive(), send();
extern int	socketaddr(), select(), gethostname(), sethostname();
extern int	netreset();
