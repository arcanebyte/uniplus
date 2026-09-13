#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#if pdp11
#define receive_data rec_data
#define wait3 wait2
#define initgroups(a,b)
#endif

#if TCP4_1a
#include <sys/timeb.h>
struct timeval { long tv_sec; long tv_usec; };
struct timeb ftimeb;
#define gettimeofday(a,b) ( ftime (&ftimeb), \
(a)->tv_sec = ftimeb.time, (a)->tv_usec = ftimeb.millitm)
#endif

#ifndef CTRL
#define CTRL(x) 037&'x'
#endif

#if TCP4_1a
#define SOL_SOCKET	0
#define SO_REUSEADDR	0
#endif

#if TCP4_1a
#define accept		Faccept
#define connect		Fconnect
#endif
