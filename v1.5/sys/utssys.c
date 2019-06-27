/* @(#)utssys.c	1.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/filsys.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/utsname.h"

utssys()
{
	register i;
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	struct {
		daddr_t	f_tfree;
		ino_t	f_tinode;
		char	f_fname[6];
		char	f_fpack[6];
	} ust;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	switch(uap->type) {

case 0:		/* uname */
	if (copyout((caddr_t)&utsname, uap->cbuf, sizeof(struct utsname)))
		up->u_error = EFAULT;
	return;

/* case 1 was umask */

case 2:		/* ustat */
	for(i=0; i<v.v_mount; i++) {
		register struct mount *mp;

		mp = &mount[(short)i];
		if (mp->m_flags==MINUSE && brdev(mp->m_dev)==brdev(uap->mv)) {
			register struct filsys *fp;

			fp = mp->m_bufp->b_un.b_filsys;
			ust.f_tfree = FsLTOP(mp->m_dev, fp->s_tfree);
			ust.f_tinode = fp->s_tinode;
			bcopy(fp->s_fname, ust.f_fname, sizeof(ust.f_fname));
			bcopy(fp->s_fpack, ust.f_fpack, sizeof(ust.f_fpack));
			if (copyout((caddr_t)&ust, uap->cbuf, 18))
				up->u_error = EFAULT;
			return;
		}
	}
	up->u_error = EINVAL;
	return;

case 33:	/* uvar */
	if (copyout((caddr_t)&v, uap->cbuf, sizeof(struct var)))
		up->u_error = EFAULT;
	return;

default:
	up->u_error = EFAULT;
	}
}
