#define _WHOAMI			/* so param.h won't include us again */

#define %IDENT%
#define MYNAME	"%ident%"	/* for uucp */
#define PDP11	%PDP%
/* #define NONFP		/* if no floating point unit */

#ifdef	KERNEL
#    include "localopts.h"
#else
#    include <sys/localopts.h>
#endif
