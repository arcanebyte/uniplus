/* @(#)err.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/utsname.h"
#include "sys/elog.h"
#include "sys/erec.h"

static	short	logging;

erropen(dev,flg)
{
	if(logging) {
		u.u_error = EBUSY;
		return;
	}
	if((flg&FWRITE) || dev != 0) {
		u.u_error = ENXIO;
		return;
	}
	if(suser()) {
		logstart();
		logging++;
	}
}

/* ARGSUSED */
errclose(dev,flg)
{
	logging = 0;
}

/* ARGSUSED */
errread(dev)
{
	register struct errhdr *eup;
	register n;
	struct errhdr	*geterec();

	if(logging == 0)
		return;
	eup = geterec();
	n = min((unsigned)eup->e_len, u.u_count);
	if (copyout((caddr_t)eup, u.u_base, n))
		u.u_error = EFAULT;
	else
		u.u_count -= n;
	freeslot(eup);
}