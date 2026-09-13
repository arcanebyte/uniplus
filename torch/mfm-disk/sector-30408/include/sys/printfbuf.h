/*
 * @(#)ROOT	printfbuf.h	1.1
 */

#define	PRINTF_MAGIC	0x63060L	/* magic number to mark buffer start */

struct	printfbuf {
	long	printf_magic;
	long	printf_bufx;
	short	printf_size;	/* actual size of printf_bufc */
	char	printf_bufc[1];
};

#ifdef	KERNEL
extern	struct	printfbuf	*printfbuf;	/* assigned in smachdep.c */
#endif	KERNEL
