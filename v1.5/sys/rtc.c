/* #define	HOWFAR */
/*
 * Real time clock driver
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Only reads and writes of 4 bytes (sizeof time_t, the standard unix
 * representation of time) are allowed for the real time clock.
 * The rtime structure contains the time most recently read or written.
 *
 * struct rtime {		See page 35 LHRM
 * 	time_t	rt_tod;		Seconds since the Epoch (unix time)
 * 	long	rt_alrm;	Seconds remaining to trigger alarm (unused)
 * 	short	rt_year;	year			(0 - 15)
 * 	short	rt_day;		julian day		(1 - 366)
 * 	short	rt_hour;	hour			(0 - 23)
 * 	short	rt_min;		minute			(0 - 59)
 * 	short	rt_sec;		second			(0 - 59)
 * 	short	rt_tenth;	tenths of a second	(0 - 9)
 * };
 */
#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/utsname.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "setjmp.h"
#include "sys/reg.h"
#include "sys/tty.h"
#include "sys/local.h"

struct rtime rtime;
int rtcrwait;		/* flag to sleep on waiting for read to complete */

/*
 * Read the real time clock.  The command to do this is issued through
 * the COPS.  In response, the keyboard gets 6 special interrupts with
 * the data.  kbintr stores this data in rtime and calls rtcsettod when
 * the sixth one has been handled.  rtcsettod calculates the time as
 * unix likes to think of it, which is as it is returned.
 */
/* ARGSUSED */
rtcread(dev)
dev_t dev;
{
	time_t rt;

	if (u.u_count != sizeof(time_t)) {
		u.u_error = ENXIO;
		return;
	}
	rt = rtcdoread();
	iomove((caddr_t)&rt, sizeof(time_t), B_READ);
}

/*
 * This routine is normally called from rtcread.  However the kernel
 * reads the clock by calling rtcdoread directly from clkset.
 */
rtcdoread()
{
	l2copscmd(READCLOCK);
	rtcrwait = 1;
	while (rtcrwait)
		(void) sleep((caddr_t)&rtcrwait, TTIPRI);
	return(rtime.rt_tod);
}

/*
 * Set rt_tod (real time in seconds since epoch) given the real
 * time clock time currently in the rest of the rtime structure.
 * Those values (rt_year, rt_day, rt_hour, rt_min, rt_sec) are
 * set in response to l2copscmd(READCLOCK), and are returned through
 * special keyboard interrupts (kbintr).
 */
#define dsize(x) (x%4==0 ?366 :365)
#define YEAR 70
rtcsettod ()
{
	register struct rtime *p = &rtime;
	register short i, j;

	i = -1;
	for (j=YEAR; j-YEAR<p->rt_year; j++)
		i += dsize(j);
	p->rt_tod = i + p->rt_day;		/* Days since epoch */
	p->rt_tod *= 24;
	p->rt_tod += p->rt_hour;
	p->rt_tod *= 60;
	p->rt_tod += p->rt_min;
	p->rt_tod *= 60;
	p->rt_tod += p->rt_sec;
#ifdef HOWFAR
	printf("DATE: %d.%d %d:%d.%d\n", p->rt_year, p->rt_day,
		p->rt_hour, p->rt_min, p->rt_sec);
#endif HOWFAR
	if (rtcrwait) {
		rtcrwait = 0;
		wakeup((caddr_t)&rtcrwait);
	}
}

/*
 * Set the real time clock to the time indicated.  Note that nt is in
 * seconds since midnight 1/1/1970 GMT.  The timezone has already been
 * corrected for.
 */
/* ARGSUSED */
rtcwrite(dev)
dev_t dev;
{
	register struct rtime *p = &rtime;
	register long nt;
	register long i, j;

	if (u.u_count != sizeof(time_t)) {
		u.u_error = ENXIO;
		return;
	}
	iomove((caddr_t)&rtime.rt_tod, sizeof(time_t), B_WRITE);
	nt = p->rt_tod;
	i = nt % 86400;		/* 86400 = 24*60*60 = number secs./day */
	j = nt / 86400;
	if (i < 0) {
		i += 86400;
		j -= 1;
	}
	nt = j;
	p->rt_sec = i % 60;
	i /= 60;
	p->rt_min = i % 60;
	i /= 60;
	p->rt_hour = i % 24;
	j = YEAR;
	for (j=YEAR; i=dsize(j); j++) {
		if (nt < i)
			break;
		nt -= i;
	}
	p->rt_year = j - YEAR;
	if (p->rt_year < 10) {
#ifdef HOWFAR
		printf("can't remember dates before 1980\n");
#endif HOWFAR
		u.u_error = EINVAL;
		return;
	}
	p->rt_day = ++nt;
	l2copscmd(SETCLOCK);	/* stop clock, start setup mode */
	for (i=0; i<5; i++)
		l2copscmd(CLKNIBBLE);	/* no alarm implemented yet */
	l2copscmd(CLKNIBBLE | (p->rt_year - 10));
	i = p->rt_day / 10;
	j = i / 10;
	l2copscmd((char)(CLKNIBBLE | j));
	l2copscmd((char)(CLKNIBBLE | (i % 10)));
	l2copscmd(CLKNIBBLE | (p->rt_day % 10));
	l2copscmd(CLKNIBBLE | (p->rt_hour / 10));
	l2copscmd(CLKNIBBLE | (p->rt_hour % 10));
	l2copscmd(CLKNIBBLE | (p->rt_min / 10));
	l2copscmd(CLKNIBBLE | (p->rt_min % 10));
	l2copscmd(CLKNIBBLE | (p->rt_sec / 10));
	l2copscmd(CLKNIBBLE | (p->rt_sec % 10));
	l2copscmd(STRTCLOCK);	/* start clock, leave timer disabled */
/*	l2copscmd(READCLOCK);	/* Now read it back */
}
