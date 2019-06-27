/* @(#)acct.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/acct.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/file.h"

/*
 * Perform process accounting functions.
 */

sysacct()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
	} *uap;
	static aclock;

	uap = (struct a *)u.u_ap;
	if (aclock || !suser())
		return;
	aclock++;
	switch (uap->fname) {
	case NULL:
		if (acctp) {
			plock(acctp);
			iput(acctp);
			acctp = NULL;
		}
		break;
	default:
		if (acctp) {
			u.u_error = EBUSY;
			break;
		}
		ip = namei(uchar, 0);
		if (ip == NULL)
			break;
		if ((ip->i_mode & IFMT) != IFREG)
			u.u_error = EACCES;
		else
			(void) access(ip, IWRITE);
		if (u.u_error) {
			iput(ip);
			break;
		}
		acctp = ip;
		prele(ip);
	}
	aclock--;
}

/*
 * On exit, write a record on the accounting file.
 */
acct(st)
{
	register struct inode *ip;
	register struct user *up;
	register struct acct *ap;
	off_t siz;

	if ((ip=acctp) == NULL)
		return;
	up = &u;
	plock(ip);
	ap = &acctbuf;
	bcopy((caddr_t)up->u_comm, (caddr_t)ap->ac_comm, sizeof(ap->ac_comm));
	ap->ac_btime = up->u_start;
	ap->ac_utime = compress(up->u_utime);
	ap->ac_stime = compress(up->u_stime);
	ap->ac_etime = compress(lbolt - up->u_ticks);
	ap->ac_mem = compress(up->u_mem);
	ap->ac_io = compress(up->u_ioch);
	ap->ac_rw = compress(up->u_ior+up->u_iow);
	ap->ac_uid = up->u_ruid;
	ap->ac_gid = up->u_rgid;
	ap->ac_tty = up->u_ttyp ? up->u_ttyd : NODEV;
	ap->ac_stat = st;
	ap->ac_flag = up->u_acflag;
	siz = ip->i_size;
	up->u_offset = siz;
	up->u_base = (caddr_t)ap;
	up->u_count = sizeof(acctbuf);
	up->u_segflg = 1;
	up->u_error = 0;
	up->u_limit = (daddr_t)5000;
	up->u_fmode = FWRITE;
	writei(ip);
	if (up->u_error)
		ip->i_size = siz;
	prele(ip);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
compress(t)
register time_t t;
{
	register exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
}
