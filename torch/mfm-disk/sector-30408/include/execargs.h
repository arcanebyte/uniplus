/*	@(#)execargs.h	2.1	 */
#ifdef vax
char **execargs = (char**)(0x7ffffffc);
#endif

#ifdef pdp11
char **execargs = (char**)(-2);
#endif

#ifdef u3b
/* empty till we can figure out what to do for the shell */
#endif

#ifdef m68k
#ifdef M68020	/* 32-bit addresses */
char **execargs = (char**)(0xfffffffc);
#else		/* 24-bit addresses */
char **execargs = (char**)(0xfffffc);
#endif
#endif
