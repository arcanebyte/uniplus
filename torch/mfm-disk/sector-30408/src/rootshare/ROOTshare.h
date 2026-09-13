/*
 * Data items required by routines in the ROOTshare library are
 * declared together according to the following two structures
 * (one for initialized data, one for uninitialized data). Pointers
 * to each of these structures are provided at known locations
 * (USTART+4 and USTART+8).
 */

#include <stdio.h>

struct share_data {
	int	s_errno;
	char	**s_environ;
	int	sbrk_end;
	unsigned	char	*s_stdbuf[2];
	FILE	s_iob[_NFILE];
	FILE	*s_lastbuf;
	unsigned	char *s_bufendtab[_NFILE+1];
};

#define	errno		(SH_DATA->s_errno)
#define	environ		(SH_DATA->s_environ)
#define	_stdbuf		(SH_DATA->s_stdbuf)
#define	_iob		(SH_DATA->s_iob)
#define	_lastbuf	(SH_DATA->s_lastbuf)
#define	_bufendtab	(SH_DATA->s_bufendtab)

typedef int (*fn)();

/* for gen/malloc.c */
union store {
	union store *ptr;
	int dummy[1];		/* ALIGN dummy[NALIGN]; */
	int calloc;		/*calloc clears an array of integers*/
};

struct	share_bss {
/* +0 */	fn	s_sigtab[32];	/* 32 = NSIG from sys/signal.h */
/* +128 */	union store s_allocs[2];
/* +152 */	union store *s_allocp;
/* +156 */	union store *s_alloct;
/* +160 */	union store *s_allocx;
/* +164 */	union store *s_allocend;
/* +168 */	char	s_buf[80];	/* 80=NDIG in gen/ecvt.c */
/* +248 */	unsigned	char	s_sibuf[BUFSIZ+8];
/* +1280 */	unsigned	char	s_sobuf[BUFSIZ+8];
/* +2312 */	unsigned	char	s_smbuf[_NFILE+1][_SBFSIZ];
/* +2480 */
};

#define	_sibuf	(SH_BSS->s_sibuf)
#define	_sobuf	(SH_BSS->s_sobuf)
#define	_smbuf	(SH_BSS->s_smbuf)

extern	struct	share_data	*SH_DATA;
extern	struct	share_bss	*SH_BSS;
