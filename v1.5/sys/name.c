/* @(#)name.c	1.1 */
#include "sys/utsname.h"

#ifdef lint
#define	SYS	"sys"
#define	NODE	"node"
#define	REL	"rel"
#define	VER	"ver"
#define	MACH	"m68000"
#define	TIMESTAMP "timestamp"
#endif
struct utsname utsname = {
	SYS,
	NODE,
	REL,
	VER,
	MACH,
};

char timestamp[] = TIMESTAMP;
