/* @(#)sys.c	1.2 */
/*
 *	indirect driver for controlling tty.
 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/proc.h"

/* ARGSUSED */
syopen(dev, flag)
{

	if (sycheck())
	(*cdevsw[(short)major(u.u_ttyd)].d_open)(minor(u.u_ttyd), flag);
}

/* ARGSUSED */
syread(dev)
{

	if (sycheck())
	(*cdevsw[(short)major(u.u_ttyd)].d_read)(minor(u.u_ttyd));
}

/* ARGSUSED */
sywrite(dev)
{

	if (sycheck())
	(*cdevsw[(short)major(u.u_ttyd)].d_write)(minor(u.u_ttyd));
}

/* ARGSUSED */
syioctl(dev, cmd, arg, mode)
{

	if (sycheck())
	(*cdevsw[(short)major(u.u_ttyd)].d_ioctl)(minor(u.u_ttyd), cmd, arg, mode);
}

sycheck()
{
	if (u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return(0);
	}
	if (*u.u_ttyp != u.u_procp->p_pgrp) {
		u.u_error = EIO;
		return(0);
	}
	return(1);
}
