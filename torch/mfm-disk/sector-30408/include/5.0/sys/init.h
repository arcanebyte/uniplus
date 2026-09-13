/*	@(#)init.h UniPlus+ v.0.1.1 */
/* @(#)init.h	1.6 */
extern int cinit(),binit(),errinit(),iinit(),inoinit();
extern int finit(),msginit(),seminit();
extern int oem0init(), oem7init();
#ifdef X25_0
extern x25init();
#endif
#ifdef ST_0
extern	stinit();
#endif
#ifdef	VPM_0
extern	vpminit();
#endif

/*	Array containing the addresses of the various initializing	*/
/*	routines executed by "main" at boot time.			*/

struct init_tbl init7_tbl[] = {
	oem7init,	"oem7init",
	0
};

struct init_tbl init0_tbl[] = {
	inoinit,	"inoinit",
	cinit,		"cinit",
	binit,		"binit",
	errinit,	"errinit",
	finit,		"finit",
	iinit,		"iinit",
#ifdef	VPM_0
	vpminit,	"vpminit",
#endif
#ifdef X25_0
	x25init,	"x25init",
#endif
#ifdef ST_0
	stinit,		"stinit",
#endif
	msginit,	"msginit",
	seminit,	"seminit",
	oem0init,	"oem0init",
	0
};
