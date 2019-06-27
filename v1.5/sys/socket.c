/*	socket.c	1.3	84/11/02	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/termio.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "net/ubavar.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/route.h"

/*
 * Socket support routines.
 *
 * DEAL WITH INTERRUPT NOTIFICATION.
 */

/*
 * Create a socket.
 */
socreate(aso, type, asp, asa, options)
	struct socket **aso;
	int type;
	struct sockproto *asp;
	struct sockaddr *asa;
	int options;
{
	register struct protosw *prp;
	register struct socket *so;
	int pf, proto, error;

	/*
	 * Use process standard protocol/protocol family if none
	 * specified by address argument.
	 */
	if (asp == 0) {
		pf = PF_INET;		/* should be u.u_protof */
		proto = 0;
	} else {
		pf = asp->sp_family;
		proto = asp->sp_protocol;
	}

	/*
	 * If protocol specified, look for it, otherwise
	 * for a protocol of the correct type in the right family.
	 */
	if (proto)
		prp = pffindproto(pf, proto);
	else
		prp = pffindtype(pf, type);
	if (prp == 0)
		return (EPROTONOSUPPORT);

	/*
	 * Get a socket structure.
	 */
	MSGET(so, struct socket, 1);
	if (so == 0)
		return (ENOBUFS);
	so->so_options = options;
	so->so_state = 0;
	if (u.u_uid == 0)
		so->so_state = SS_PRIV;

	/*
	 * Attach protocol to socket, initializing
	 * and reserving resources.
	 */
	so->so_proto = prp;
	error = (*prp->pr_usrreq)(so, PRU_ATTACH, 0, asa);
	if (error) {
#ifdef NEVER	/*
		 * This commenting-out came from jsq@utexas-11; see the
		 * iptcp11 messages.  Simply means temp lack of mbufs...
		 */
		if (so->so_snd.sb_mbmax || so->so_rcv.sb_mbmax)
			panic("socreate");
#endif
		so->so_state |= SS_USERGONE;
		sofree(so);
		return (error);
	}
	*aso = so;
	return (0);
}

sofree(so)
	struct socket *so;
{

	if (so->so_pcb || (so->so_state & SS_USERGONE) == 0)
		return;
	sbrelease(&so->so_snd);
	sbrelease(&so->so_rcv);
	MSFREE(so);
}

/*
 * Close a socket on last file table reference removal.
 * Initiate disconnect if connected.
 * Free socket when disconnect complete.
 *
 * THIS IS REALLY A UNIX INTERFACE ROUTINE
 */
soclose(so, exiting)
	register struct socket *so;
	int exiting;
{
	int s = splnet();		/* conservative */

	if (so->so_pcb == 0)
		goto discard;
	if (exiting)
		so->so_options |= SO_KEEPALIVE;
	if (so->so_state & SS_ISCONNECTED) {
		if ((so->so_state & SS_ISDISCONNECTING) == 0) {
			u.u_error = sodisconnect(so, (struct sockaddr *)0);
			if (u.u_error) {
				if (exiting)
					goto drop;
				splx(s);
				return;
			}
		}
		if ((so->so_options & SO_DONTLINGER) == 0) {
			if ((so->so_state & SS_ISDISCONNECTING) &&
			    (so->so_state & SS_NBIO) &&
			    exiting == 0) {
				u.u_error = EINPROGRESS;
				/*
				 * billn.  This is a kludge rendered unnecessary
				 * by a bug fix 12/27/83.
				so->so_state |= SS_USERGONE;
				 */
				splx(s);
				return;
			}
			/* should use tsleep here, for at most linger */
			while (so->so_state & SS_ISCONNECTED)
				(void) sleep((caddr_t)&so->so_timeo, PZERO+1);
		}
	}
drop:
	if (so->so_pcb) {
		u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_DETACH, 0, 0);
		if (exiting == 0 && u.u_error) {
			splx(s);
			return;
		}
	}
discard:
	so->so_state |= SS_USERGONE;
	sofree(so);
	splx(s);
}

/*ARGSUSED*/
sostat(so, sb)
	struct socket *so;
	struct stat *sb;
{

	/* bug fix by JC Stewart of sri; this zeros the kernel, should zero 
	   user data (!)...
	bzero((caddr_t)sb, sizeof (*sb));		/* XXX */
	return (0);					/* XXX */
}

/*
 * Accept connection on a socket.
 */
soaccept(so, asa)
	struct socket *so;
	struct sockaddr *asa;
{
	int s = splnet();
	int error;

	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;			/* XXX */
		goto bad;
	}
	if ((so->so_state & SS_CONNAWAITING) == 0) {
		error = ENOTCONN;
		goto bad;
	}
	so->so_state &= ~SS_CONNAWAITING;
	error = (*so->so_proto->pr_usrreq)(so, PRU_ACCEPT, 0, (caddr_t)asa);
bad:
	splx(s);
	return (error);
}

/*
 * Connect socket to a specified address.
 * If already connected or connecting, then avoid
 * the protocol entry, to keep its job simpler.
 */
soconnect(so, asa)
	struct socket *so;
	struct sockaddr *asa;
{
	int s = splnet();
	int error;

	if (so->so_state & (SS_ISCONNECTED|SS_ISCONNECTING)) {
		error = EISCONN;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so, PRU_CONNECT, 0, (caddr_t)asa);
bad:
	splx(s);
	return (error);
}

/*
 * Disconnect from a socket.
 * Address parameter is from system call for later multicast
 * protocols.  Check to make sure that connected and no disconnect
 * in progress (for protocol's sake), and then invoke protocol.
 */
sodisconnect(so, asa)
	struct socket *so;
	struct sockaddr *asa;
{
	int s = splnet();
	int error;

	if ((so->so_state & SS_ISCONNECTED) == 0) {
		error = ENOTCONN;
		goto bad;
	}
	if (so->so_state & SS_ISDISCONNECTING) {
		error = EALREADY;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so, PRU_DISCONNECT, 0, asa);
bad:
	splx(s);
	return (error);
}

/*
 * Send on a socket.
 * If send must go all at once and message is larger than
 * send buffering, then hard error.
 * Lock against other senders.
 * If must go all at once and not enough room now, then
 * inform user that this would block and do nothing.
 */
sosend(so, asa)
	register struct socket *so;
	struct sockaddr *asa;
{
	struct mbuf *top = 0;
	register struct mbuf **mp = &top;
	register struct mbuf *m;
	register struct user *up = &u;
	register u_int len;
	register int space;
	int error = 0, s;

	if (sosendallatonce(so) && up->u_count > so->so_snd.sb_hiwat)
		return (EMSGSIZE);
#ifdef notdef
	/* NEED TO PREVENT BUSY WAITING IN SELECT FOR WRITING */
	if ((so->so_snd.sb_flags & SB_LOCK) && (so->so_state & SS_NBIO))
		return (EWOULDBLOCK);
#endif
restart:
	sblock(&so->so_snd);
#define	snderr(errno)	{ error = errno; splx(s); goto release; }

again:
	s = splnet();
	if (so->so_state & SS_CANTSENDMORE) {
		psignal(up->u_procp, SIGPIPE);
		snderr(EPIPE);
	}
	if (so->so_error) {
		error = so->so_error;
		so->so_error = 0;				/* ??? */
		splx(s);
		goto release;
	}
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		if (so->so_proto->pr_flags & PR_CONNREQUIRED)
			snderr(ENOTCONN);
		if (asa == 0)
			snderr(EDESTADDRREQ);
	}
	if (top) {
		error = (*so->so_proto->pr_usrreq)(so, PRU_SEND, top, asa);
		top = 0;
		if (error) {
			splx(s);
			goto release;
		}
		mp = &top;
	}
	if (up->u_count == 0) {
		splx(s);
		goto release;
	}
	space = sbspace(&so->so_snd);
	if (space <= 0 || sosendallatonce(so) && space < up->u_count) {
		if (so->so_state & SS_NBIO)
			snderr(EWOULDBLOCK);
		sbunlock(&so->so_snd);
		sbwait(&so->so_snd);
		splx(s);
		goto restart;
	}
	splx(s);
	while (up->u_count && space > 0) {
		MGET(m, 1);
		if (m == NULL) {
			error = ENOBUFS;			/* SIGPIPE? */
			goto release;
		}
		m->m_off = MMINOFF;
		len = (u_int)MIN(((int)MLEN), ((int)up->u_count));
		iomove(mtod(m, caddr_t), (int)len, B_WRITE);
		if (error = up->u_error) goto release;
		m->m_len = len;
		*mp = m;
		mp = &m->m_next;
		space = sbspace(&so->so_snd);
	}
	goto again;

release:
	sbunlock(&so->so_snd);
	if (top)
		m_freem(top);
	return (error);
}

soreceive(so, asa)
	register struct socket *so;
	struct sockaddr *asa;
{
	register struct mbuf *m, *n;
	u_int len;
	int eor, s, error = 0, cnt = u.u_count;
	caddr_t base = u.u_base;

restart:
	sblock(&so->so_rcv);
	s = splnet();

#define	rcverr(errno)	{ error = errno; splx(s); goto release; }
	if (so->so_rcv.sb_cc == 0) {
		if (so->so_error) {
			error = so->so_error;
			so->so_error = 0;
			splx(s);
			goto release;
		}
		if (so->so_state & SS_CANTRCVMORE) {
			splx(s);
			goto release;
		}
		if ((so->so_state & SS_ISCONNECTED) == 0 &&
		    (so->so_proto->pr_flags & PR_CONNREQUIRED))
			rcverr(ENOTCONN);
		if (so->so_state & SS_NBIO)
			rcverr(EWOULDBLOCK);
		sbunlock(&so->so_rcv);
		sbwait(&so->so_rcv);
		splx(s);
		goto restart;
	}
	m = so->so_rcv.sb_mb;
	if (m == 0)
		panic("receive");
	if (so->so_proto->pr_flags & PR_ADDR) {
		if (m->m_len != sizeof (struct sockaddr))
			panic("soreceive addr");
		if (asa)
			bcopy(mtod(m, caddr_t), (caddr_t)asa, sizeof (*asa));
		so->so_rcv.sb_cc -= m->m_len;
		so->so_rcv.sb_mbcnt -= MSIZE;
		m = m_free(m);
		if (m == 0)
			panic("receive 2");
		so->so_rcv.sb_mb = m;
	}
	so->so_state &= ~SS_RCVATMARK;
	if (so->so_oobmark && cnt > so->so_oobmark)
		cnt = so->so_oobmark;
	eor = 0;
	do {
		len = MIN(m->m_len, cnt);
		splx(s);
		iomove(mtod(m, caddr_t), (int)len, B_READ);
		cnt -= len;
		s = splnet();
		if (len == m->m_len) {
			eor = (int)m->m_act;
			sbfree(&so->so_rcv, m);
			so->so_rcv.sb_mb = m->m_next;
			MFREE(m, n);
		} else {
			m->m_off += len;
			m->m_len -= len;
			so->so_rcv.sb_cc -= len;
		}
	} while ((m = so->so_rcv.sb_mb) && cnt && !eor);
	if ((so->so_proto->pr_flags & PR_ATOMIC) && eor == 0)
		do {
			if (m == 0)
				panic("receive 3");
			sbfree(&so->so_rcv, m);
			eor = (int)m->m_act;
			so->so_rcv.sb_mb = m->m_next;
			MFREE(m, n);
			m = n;
		} while (eor == 0);
	if ((so->so_proto->pr_flags & PR_WANTRCVD) && so->so_pcb)
		(*so->so_proto->pr_usrreq)(so, PRU_RCVD, 0, 0);
	if (so->so_oobmark) {
		so->so_oobmark -= u.u_base - base;
		if (so->so_oobmark == 0)
			so->so_state |= SS_RCVATMARK;
	}
release:
	sbunlock(&so->so_rcv);
	splx(s);
	return (error);
}

sohasoutofband(so)
	struct socket *so;
{
	struct proc *pfind();

	if (so->so_pgrp == 0)
		return;
	if (so->so_pgrp > 0)
		/*
		gsignal(so->so_pgrp, SIGURG);
		*/
		signal(so->so_pgrp, SIGURG);
	else {
		struct proc *p = pfind(-so->so_pgrp);

		if (p)
			psignal(p, SIGURG);
	}
}

/*ARGSUSED*/
soioctl(so, cmd, cmdp)
	register struct socket *so;
	int cmd;
	register caddr_t cmdp;
{
	extern struct uba_device ubdinit[];

	long *iaddrp = (long *)((int)ubdinit + 0xA);

	switch (cmd) {

	case SIOCCIADDR:
		/* set internet address ie write into uba flags */
		if (u.u_ruid != 0 && u.u_uid != 0) {
			u.u_error = EPERM;
			return;
		}
		if (copyin(cmdp, (caddr_t)iaddrp, sizeof (long))) {
			u.u_error = EFAULT;
			return;
		}
		return;

	case SIOCGIADDR:
		if (copyout((caddr_t)iaddrp, cmdp, sizeof (long)))
			u.u_error = EFAULT;
		return;

	case FIONBIO: {
		int nbio;
		if (copyin(cmdp, (caddr_t)&nbio, sizeof (nbio))) {
			u.u_error = EFAULT;
			return;
		}
		if (nbio)
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		return;
	}

	case FIOASYNC: {
		int async;
		if (copyin(cmdp, (caddr_t)&async, sizeof (async))) {
			u.u_error = EFAULT;
			return;
		}
		if (async)
			so->so_state |= SS_ASYNC;
		else
			so->so_state &= ~SS_ASYNC;
		return;
	}

	case FIONREAD: {
		long nread = so->so_rcv.sb_cc;
		if (copyout((caddr_t)&nread, cmdp, sizeof (nread)))
			u.u_error = EFAULT;
		return;
	}

	case SIOCSKEEP: {
		int keep;
		if (copyin(cmdp, (caddr_t)&keep, sizeof (keep))) {
			u.u_error = EFAULT;
			return;
		}
		if (keep)
			so->so_options |= SO_KEEPALIVE;
		else
			so->so_options &= ~SO_KEEPALIVE;
		return;
	}

	case SIOCGKEEP: {
		int keep = (so->so_options & SO_KEEPALIVE) != 0;
		if (copyout((caddr_t)&keep, cmdp, sizeof (keep)))
			u.u_error = EFAULT;
		return;
	}

	case SIOCSLINGER: {
		int linger;
		if (copyin(cmdp, (caddr_t)&linger, sizeof (linger))) {
			u.u_error = EFAULT;
			return;
		}
		so->so_linger = linger;
		if (so->so_linger)
			so->so_options &= ~SO_DONTLINGER;
		else
			so->so_options |= SO_DONTLINGER;
		return;
	}

	case SIOCGLINGER: {
		int linger = so->so_linger;
		if (copyout((caddr_t)&linger, cmdp, sizeof (linger))) {
			u.u_error = EFAULT;
			return;
		}
	}
	case SIOCSPGRP: {
		int pgrp;
		if (copyin(cmdp, (caddr_t)&pgrp, sizeof (pgrp))) {
			u.u_error = EFAULT;
			return;
		}
		so->so_pgrp = pgrp;
		return;
	}

	case SIOCGPGRP: {
		int pgrp = so->so_pgrp;
		if (copyout((caddr_t)&pgrp, cmdp, sizeof (pgrp))) {
			u.u_error = EFAULT;
			return;
		}
	}

	case SIOCDONE: {
		int flags;
		if (copyin(cmdp, (caddr_t)&flags, sizeof (flags))) {
			u.u_error = EFAULT;
			return;
		}
		flags++;
		if (flags & FREAD) {
			int s = splimp();
			socantrcvmore(so);
			sbflush(&so->so_rcv);
			splx(s);
		}
		if (flags & FWRITE)
			u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_SHUTDOWN, (struct mbuf *)0, 0);
		return;
	}

	case SIOCSENDOOB: {
		char oob;
		struct mbuf *m;
		if (copyin(cmdp, (caddr_t)&oob, sizeof (oob))) {
			u.u_error = EFAULT;
			return;
		}
		m = m_get(M_DONTWAIT);
		if (m == 0) {
			u.u_error = ENOBUFS;
			return;
		}
		m->m_off = MMINOFF;
		m->m_len = 1;
		*mtod(m, caddr_t) = oob;
		(*so->so_proto->pr_usrreq)(so, PRU_SENDOOB, m, 0);
		return;
	}

	case SIOCRCVOOB: {
		struct mbuf *m = m_get(M_DONTWAIT);
		if (m == 0) {
			u.u_error = ENOBUFS;
			return;
		}
		m->m_off = MMINOFF; *mtod(m, caddr_t) = 0;
		(*so->so_proto->pr_usrreq)(so, PRU_RCVOOB, m, 0);
		if (copyout(mtod(m, caddr_t), cmdp, sizeof (char))) {
			u.u_error = EFAULT;
			return;
		}
		(void) m_free(m);
		return;
	}

	case SIOCATMARK: {
		int atmark = (so->so_state&SS_RCVATMARK) != 0;
		if (copyout((caddr_t)&atmark, cmdp, sizeof (atmark))) {
			u.u_error = EFAULT;
			return;
		}
		return;
	}

	/* routing table update calls */
	case SIOCADDRT:
	case SIOCDELRT:
	case SIOCCHGRT: {
		struct rtentry route;
		if (!suser())
			return;
		if (copyin(cmdp, (caddr_t)&route, sizeof (route))) {
			u.u_error = EFAULT;
			return;
		}
		u.u_error = rtrequest(cmd, &route);
		return;
	}

	/* type/protocol specific ioctls */
	}
	u.u_error = EOPNOTSUPP;
}
