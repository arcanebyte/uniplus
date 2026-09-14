/*
 * compat.h -- what the 4.1c ftp expects that the 4.1a socket calls on
 * UniPlus+ lack, from 2.9BSD's TCP4_1a conversion, for the Lisa.
 */
#include "bsd.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/* No gettimeofday() or ftime(): whole seconds from time(). */
struct timeval { long tv_sec; long tv_usec; };
struct timezone { int tz_minuteswest; int tz_dsttime; };
#define gettimeofday(a,b) ((a)->tv_sec = time((long *)0), (a)->tv_usec = 0)

#ifndef CTRL
#define CTRL(x) 037&'x'
#endif

#define SOL_SOCKET	0
#define SO_REUSEADDR	0

/* accept() and connect() with 4.2BSD arguments, in compat.c */
#define accept		Faccept
#define connect		Fconnect
