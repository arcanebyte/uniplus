#include <stdio.h>
/* convert the five characters in password format to a 30 bit binary integer. */
ucvtb (str)
register char *str;
{
	register unsigned long v = 0;
	register int i;

	for (i=0; i<5; i++) {
		v <<= 6;
		v += cv6(*str++);
	}
	return v;
}

/* Convert the integer s into a sequence of l ascii password characters */
cvtcy(s, d, l)
char *d;
{
	register unsigned long t = s;

	while (l-- > 0) {
		*d++ = mksymb(t & 0x3F);
		t >>= 6;
	}
	*d = 0;
}

/* Convert the integer x (between 0 and 63) to ascii password format */
mksymb(x)
register unsigned long x;
{
	if (x < 26) return 'a' + x;
	x -= 26;
	if (x < 26) return 'A' + x;
	x -= 26;
	if (x < 10) return '0' + x;
	x -= 10;
	if (x) return '/';
	return '.';
}

cv6 (x)
{
	if (x >= 'a' && x <= 'z')
		return (x - 'a');
	if (x >= 'A' && x <= 'Z')
		return (x - 'A') + 26;
	if (x >= '0' && x <= '9')
		return (x - '0') + 52;
	if (x == '.') return 62;
	return 63;
}

cvtbin(s, r, l)
char *r, *s;
{
	register int x;

	while (l >= 4) {
		x = cv6(r[3]);
		x <<= 6;
		x += cv6(r[2]);
		x <<= 6;
		x += cv6(r[1]);
		x <<= 6;
		x += cv6(r[0]);
		*s++ = (x >> 16) & 0xFF;
		*s++ = (x >> 8) & 0xFF;
		*s++ = x & 0xFF;
		r += 4;
		l -= 4;
	}
	if (l == 3) {
		x = cv6(r[2]);
		x <<= 6;
		x += cv6(r[1]);
		x <<= 6;
		x += cv6(r[0]);
		*s++ = (x >> 8) & 0xFF;
		*s++ = x & 0xFF;
		l -= 3;
		r += 3;
	}
	if (l == 2) {
		x = cv6(r[1]);
		x <<= 6;
		x += cv6(r[0]);
		*s++ = x & 0xFF;
		l -= 2;
		r += 2;
	}
	if (l) printf("INCORRECT USAGE OF CVTBIN function\n");
	*s++ = 0;
}

cvt64 (r, s, l)
register char *r, *s;
{
	register int x;

	while (l >= 3) {
		x = *s++ & 0xFF;
		x <<= 8;
		x += *s++ & 0xFF;
		x <<= 8;
		x += *s++ & 0xFF;
		cvtcy(x, r, 4);
		r += 4;
		l -= 3;
	}
	if (l == 2) {
		x = *s++ & 0xFF;
		x <<= 8;
		x += *s++ & 0xFF;
		cvtcy(x, r, 3);
		r += 3;
		l -= 2;
	}
	if (l == 1) {
		x = *s++ & 0xFF;
		cvtcy(x, r, 2);
		r += 2;
	}
	*r = 0;
}

pdate (x)
unsigned long x;
{
	printf("   %x",x);
}

gdate (s)
register char *s;
{
	register long v = 0;

	while (*s) {
		v <<= 4;
		if (*s <= '9' && *s >= '0')
			v += *s - '0';
		else if (*s >= 'a' && *s <= 'f')
			v += *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'F')
			v += *s - 'A' + 10;
		else {
			printf("Invalid input format\n");
			return 0;
		}
		s++;
	}
	return v;
}

#ifdef DATES
#include <sys/types.h>
#include <time.h>

#define when break; case
#define TIMEZONE 8		/* hours behind GMT */
#define	YR 0
#define MO 1
#define DY 2
#define HR 3
#define MN 4
#define SD 5
#define ERRTHRESH	6
#define NOTDIGIT	7
#define BADYEAR		8
#define BADMONTH	9
#define BADDAY		10
#define BADHOUR		11
#define BADMIN		12
#define BADSEC		13

/* Date String TO Time Vector (human date string)  -- Function to convert a
 *				string of the form YYMMDDhhmmss (gmtime)
 *				into an integer value (time_t) (the number of
 *				seconds since the epoch -- GMT.)
 */
				
time_t dstotv (hd)  register char *hd;
{	short p, v, l;
	time_t  r;
	static short mlen [] = { 31,  59,  90, 120, 151, 181,
				212, 243, 273, 304, 334, 365 };

  for (p=YR; p<ERRTHRESH; p++)
  { if   (*hd<'0' || *hd++>'9' ||
          *hd<'0' || *hd++>'9')	p  = NOTDIGIT;
    				v  = (hd[-2] - '0') * 10 + hd[-1] - '0';
    switch (p)
    { when YR:			v -= 68;
		       if (v<2) v += 100;
	          		l  = (v%4 == 0) ? 1 : 0;
				r  = (time_t)((v-2)*365 + (v-1)/4);
      when MO: if (v<1 || v>12) p  = BADMONTH;
		       if (--v)	r += (time_t)(mlen[v-1]);
	             if (v > 1) r += (time_t)l;		   /* add leap day */
      when DY: if (v<1 || v>31)	p  = BADDAY;
				r += (time_t)(v-1);
      when HR:        if (v>23)	p  = BADHOUR;
				r  = r * 24L + (time_t)v;
				r += (time_t)(TIMEZONE);
      when MN:	      if (v>59)	p  = BADMIN;
				r  = r * 60L + (time_t)v;
      when SD:	      if (v>59)	p  = BADSEC;
				r  = r * 60L + (time_t)v;
    }
  }
  if (p != ERRTHRESH) return (time_t)(- (p - ERRTHRESH));
  return r;
}

pdate(i)
	int i;
{
	char *cp, *tvtods();

	if (i & 0x80000000) {
		printf("+");
		i &= 0x7FFFFFFF;
	}
	if (cp = tvtods(i))
		printf("%s\n",cp);
	else
		printf("CONVERSION\n");
}

gdate (cp)
	char *cp;
{
	int flg = 0;
	int v;

	gets(cp);
	if (*cp == '+') {
		cp++;
		flg = 1;
	}
	if (*cp == ' ') cp++;
	v = dstotv(cp);
	if (flg) v |= 0x80000000;
	return v;
}

char *
tvtods(t) time_t t;
{	struct tm *tv, *gmtime();
	static char buf[14];
	register char *p;

  tv = gmtime(&t);
  p = buf;
  tv->tm_mon++;
  if (tv->tm_year >= 100) tv->tm_year -= 100;
  *p++ = tv->tm_year / 10;
  *p++ = tv->tm_year % 10;
  *p++ = tv->tm_mon / 10;
  *p++ = tv->tm_mon % 10;
  *p++ = tv->tm_mday / 10;
  *p++ = tv->tm_mday % 10;
  *p++ = tv->tm_hour / 10;
  *p++ = tv->tm_hour % 10;
  *p++ = tv->tm_min / 10;
  *p++ = tv->tm_min % 10;
  *p++ = tv->tm_sec / 10;
  *p++ = tv->tm_sec % 10;
  *p = '\0';
  for (p=buf; p<buf+12; p++)
    *p += '0';
  return buf;
}
#endif DATES
