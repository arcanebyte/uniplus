/*	ipc.c	4.20	82/06/20	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "net/misc.h"
/*
#include "net/mbuf.h"
*/
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/in.h"
#include "net/in_systm.h"

/*
 * Socket system call interface.
 *
 * These routines interface the socket routines to UNIX,
 * isolating the system interface from the socket-protocol interface.
 *
 * TODO:
 *	SO_INTNOTIFY
 */


/*
 * Socket system call interface.  Copy sa arguments
 * set up file descriptor and call internal socket
 * creation routine.
 */
ssocket()
{
	register struct ua {
		int	type;
		struct	sockproto *asp;
		struct	sockaddr *asa;
		int	options;
	} *uap = (struct ua *)u.u_ap;
	struct sockproto sp;
	struct sockaddr sa;
	struct socket *so;
	register struct file *fp;

	if ((fp = falloc((struct inode *)0, FSOCKET|FREAD|FWRITE)) == NULL)
		return;
	if (uap->asp && copyin((caddr_t)uap->asp, (caddr_t)&sp, sizeof (sp)) ||
	    uap->asa && copyin((caddr_t)uap->asa, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		fp->f_count = 0;
		fp->f_next = ffreelist;
		ffreelist = fp;
		return;
	}
	u.u_error = socreate(&so, uap->type,
	    uap->asp ? &sp : 0, uap->asa ? &sa : 0, uap->options);
	if (u.u_error)
		goto bad;
	fp->f_socket = (off_t)so;
	return;
bad:
	u.u_ofile[u.u_rval1] = 0;
	fp->f_count = 0;
	fp->f_next = ffreelist;
	ffreelist = fp;
}

/*
 * Accept system call interface.
 */
saccept()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
	} *uap = (struct a *)u.u_ap;
	struct sockaddr sa;
	register struct file *fp;
	struct socket *so;
	int s;

	if (uap->asa && useracc((caddr_t)uap->asa, sizeof (sa), B_WRITE)==0) {
		u.u_error = EFAULT;
		return;
	}
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	s = splnet();
	so = (struct socket *)fp->f_socket;
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_CONNAWAITING) == 0) {
		u.u_error = EWOULDBLOCK;
		splx(s);
		return;
	}
	while ((so->so_state & SS_CONNAWAITING) == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		(void) sleep((caddr_t)&so->so_timeo, PZERO+1);
	}
	if (so->so_error) {
		u.u_error = so->so_error;
		splx(s);
		return;
	}
	u.u_error = soaccept(so, &sa);
	if (u.u_error) {
		splx(s);
		return;
	}
	if (uap->asa)
		(void) copyout((caddr_t)&sa, (caddr_t)uap->asa, sizeof (sa));
	/* deal with new file descriptor case */
	/* u.u_r.r_val1 = ... */
	splx(s);
}

/*
 * Connect socket to foreign peer; system call
 * interface.  Copy sa arguments and call internal routine.
 */
sconnect()
{
	register struct ua {
		int	fdes;
		struct	sockaddr *a;
	} *uap = (struct ua *)u.u_ap;
	struct sockaddr sa;
	register struct file *fp;
	register struct socket *so;
	int s;

	if (copyin((caddr_t)uap->a, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		return;
	}
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	so = (struct socket *)fp->f_socket;
	u.u_error = soconnect(so, &sa);
	if (u.u_error)
		return;
	s = splnet();
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EINPROGRESS;
		splx(s);
		return;
	}
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		(void) sleep((caddr_t)&so->so_timeo, PZERO+1);
	u.u_error = so->so_error;
	so->so_error = 0;
	splx(s);
}

/*
 * Send data on socket.
 */
ssend()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
		caddr_t	cbuf;
		unsigned count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct sockaddr sa;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	u.u_base = uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	if (useracc(uap->cbuf, uap->count, B_READ) == 0 ||
	    uap->asa && copyin((caddr_t)uap->asa, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = sosend((struct socket *)fp->f_socket, uap->asa ? &sa : 0);
	u.u_rval1 = uap->count - u.u_count;
}

/*
 * Receive data on socket.
 */
sreceive()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
		caddr_t	cbuf;
		u_int	count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct sockaddr sa;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	u.u_base = uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	if (useracc(uap->cbuf, uap->count, B_WRITE) == 0 ||
	    uap->asa && copyin((caddr_t)uap->asa, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = soreceive((struct socket *)fp->f_socket, uap->asa ? &sa : 0);
	if (u.u_error)
		return;
	if (uap->asa)
		(void) copyout((caddr_t)&sa, (caddr_t)uap->asa, sizeof (sa));
	u.u_rval1 = uap->count - u.u_count;
}

/*
 * Get socket address.
 */
ssocketaddr()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct sockaddr addr;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	so = (struct socket *)fp->f_socket;
	u.u_error =
		(*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, (caddr_t)&addr);
	if (u.u_error)
		return;
	if (copyout((caddr_t)&addr, (caddr_t)uap->asa, sizeof (addr)))
		u.u_error = EFAULT;
}
