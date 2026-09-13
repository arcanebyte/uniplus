/*	@(#)core.h	2.1	 */
/* machine dependent stuff for core files */

#ifdef vax
#define TXTRNDSIZ 512L
#define stacktop(siz) (0x80000000L)
#define stackbas(siz) (0x80000000L-siz)
#endif

#ifdef pdp11
#define TXTRNDSIZ 8192L
#define stacktop(siz) (0x10000L)
#define stackbas(siz) (0x10000L-siz)
#endif

#ifdef u3b
#define TXTRNDSIZ 0x20000
#define stacktop(siz) 0xC0000
#define stackbas(siz) (0xC0000 + siz)
#endif

#ifdef m68k
#define TXTRNDSIZ 512L
#ifdef M68020	/* 32-bit addresses */
#define stacktop(siz) (0xffffffffL)	/* can't represent 2^32 */
#define stackbas(siz) (-siz)		/* 2^32 - siz */
#else		/* 24-bit addresses */
#define stacktop(siz) (0x1000000L)
#define stackbas(siz) (0x1000000L-siz)
#endif
#endif
