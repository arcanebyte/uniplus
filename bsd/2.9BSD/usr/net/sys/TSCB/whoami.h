#define _WHOAMI			/* so param.h won't include us again */

#define TSCB
#define MYNAME	"tscb"	/* for uucp */
#define PDP11	44
/* #define NONFP		/* if no floating point unit */

#ifdef	KERNEL
#    include "localopts.h"
#else
#    include <sys/localopts.h>
#endif
