/* @(#)lock.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/proc.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/text.h"
#include "sys/lock.h"

lock()
{
	struct a {
		long oper;
	};

	if (!suser())
		return;
	switch(((struct a *)u.u_ap)->oper) {
	case TXTLOCK:
		if ((u.u_lock&(PROCLOCK|TXTLOCK)) || textlock() == 0)
			goto bad;
		break;
	case PROCLOCK:
		if (u.u_lock&(PROCLOCK|TXTLOCK))
			goto bad;
		(void) textlock();
		proclock();
		break;
	case DATLOCK:
		if (u.u_lock&(PROCLOCK|DATLOCK))
			goto bad;
		u.u_lock |= DATLOCK;	/* NOP for VAX */
		break;
	case UNLOCK:
		if (punlock() == 0)
			goto bad;
		break;

	default:
bad:
		u.u_error = EINVAL;
	}
}

textlock()
{
	if (u.u_procp->p_textp == NULL)
		return(0);
	u.u_lock |= TXTLOCK;
	return(1);
}
		
tunlock()
{
	if (u.u_procp->p_textp == NULL || (u.u_lock&TXTLOCK) == 0)
		return;
	u.u_lock &= ~TXTLOCK;
}

proclock()
{
	u.u_procp->p_flag |= SSYS;
	u.u_lock |= PROCLOCK;
}

punlock()
{
	if ((u.u_lock&(PROCLOCK|TXTLOCK|DATLOCK)) == 0)
		return(0);
	u.u_procp->p_flag &= ~SSYS;
	u.u_lock &= ~PROCLOCK;
	u.u_lock &= ~DATLOCK;
	tunlock();
	return(1);
}
