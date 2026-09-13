/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Window Open User Include File             *
 *                                                                            *
 * COPYRIGHT : (c) 1986 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) wopen.h 1.3@(#)" */

#include <mmi/window.h>
#include <mmi/term.h>

/* flags requestint automatic events to be handled by the window library */

#define	WAEDRAG		(1<<0)	/* auto dragging */
#define	WAEGROW		(1<<1)	/* auto growing */
#define	WAEMENU		(1<<2)	/* auto menu selection */
#define	WAECLOSE	(1<<3)	/* auto close events */
#define	WAEUPFRONT	(1<<4)	/* auto up-fronting */
#define	WAEPASSON	(1<<5)	/* auto passon of events outside window */

#define	WAENONE		0
#define	WAEALL		(WAEDRAG|WAEGROW|WAEMENU|WAECLOSE|WAEUPFRONT|WAEPASSON)

/* the default automatic flags on opening a window */

#define	WAEDEFAULTS	(WAEMENU|WAECLOSE|WAEUPFRONT|WAEPASSON)

/* values for WOpens parameter wo_flags    */

#define	WONONUNIQUE  0   /* This window - regardless of it's current state */
#define	WOANYUNIQUE  1   /* any window - mus be unique    */
#define	WOTHISUNIQUE 2   /* this window - fail of already open */
#define	WOTTYONLY    (1<<15)	/* bit to signify only open the tty */

#define	NUMPROCS   17     /* number of event procedures */

#define	STDFD      1     /* fd for standard output */

#ifdef NEOPOLITAN
#   undef  NEOPOLITAN			/* defined in kernel header files */
#endif
#ifdef NEAPOLITAN
#   undef  NEAPOLITAN			/* defined in kernel header files */
#endif

#define W_NEOPOLITAN	3		/* window library internal */

#define NEOPOLITAN	(WUseNeop())	/* make library include code */
#define NEAPOLITAN	NEOPOLITAN

#define UPBUT		0x010000
#define DOWNBUT		0x020000
#define LEFTBUT		0x040000
#define RIGHTBUT	0x080000
#define VSBAR		0x100000
#define HSBAR		0x200000
#define GROWBUT		0x400000

#define VSCRLBAR	(UPBUT | VSBAR | DOWNBUT)
#define HSCRLBAR	(LEFTBUT | HSBAR | RIGHTBUT)

#define W_SCTRLMASK	(VSCRLBAR | HSCRLBAR | GROWBUT)

/*                                                            *****************
                                                              *               *
                                                              * W_SCROLLPAD   *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Contains information about the NEOPOLITAN window's 'pad'                    |
-------------------------------------------------------------------------------
*/

typedef struct
{
   rectangle	pad;
   rectangle	window;
   short	vertstep;
   short	horizstep;
} w_scrollpad;

/*                                                            *****************
                                                              *               *
                                                              * ULIMSTATE     *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| used for reading and writing the limits on width and height of a window.    |
-------------------------------------------------------------------------------
*/

typedef struct
{
   short winwidmin;
   short winwidmax;
   short winheimin;
   short winheimax;
} ulimstate;

