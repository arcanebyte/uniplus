/* NFSSRC @(#)types.h	2.1 86/04/14 */
/*      @(#)types.h 1.1 86/02/03 SMI      */

/*
 * Rpc additions to <sys/types.h>
 */

#define	bool_t	int
#define	enum_t	int
#define	FALSE	(0)
#define	TRUE	(1)
#define __dontcare__	-1

#ifndef KERNEL
#define mem_alloc(bsize)	malloc(bsize)
#define mem_free(ptr, bsize)	free(ptr)
#ifndef major		/* ouch! */
#include <sys/types.h>
#endif
#else
#define mem_alloc(bsize)	kmem_alloc((u_int)bsize)
#define mem_free(ptr, bsize)	kmem_free((caddr_t)(ptr), (u_int)(bsize))
#endif
