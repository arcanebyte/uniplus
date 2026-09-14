/*
 * bsd.h -- what Berkeley network programs expect from <sys/types.h> and
 * libc that UniPlus+ System V spells differently or lacks.
 */
#ifndef	U_LONG_DEFINED
#define	U_LONG_DEFINED
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
#endif

/* The Lisa libc has the System V memory and string routines. */
#define	bcopy(from, to, n)	memcpy((char *)(to), (char *)(from), (n))
#define	bzero(p, n)		memset((char *)(p), 0, (n))
#define	bcmp(a, b, n)		memcmp((char *)(a), (char *)(b), (n))
#define	index			strchr
#define	rindex			strrchr
extern char	*memcpy(), *memset(), *strchr(), *strrchr();
