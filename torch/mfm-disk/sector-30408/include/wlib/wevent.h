/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Event User Include File                   *
 *                                                                            *
 * COPYRIGHT : (c) 1986 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) wevent.h 1.2@(#)" */

#include <mmi/event.h>

/* Parameters and for GetClick() */

#define	SCLICKONLY	1   /* Single click only */
#define	DCLICKONLY	2   /* double click only */
#define	DORSCLICK	3   /* either */

/* Return codes from GetClick() */

#define	NOCLICK		0   /* no click of required type */
#define	LSINGLE		1   /* single click of left button */
#define	RSINGLE		2   /* single click of right button */
#define	LDOUBLE		3   /* double click of left button */
#define	RDOUBLE		4   /* work this one out for yourself */
#define	LCLICKANDDOWN	5  /* single click + mouse down - no second click */
#define	RCLICKANDDOWN	6  /* right button version of above */

#define SCROLLEVENT	(0x101)		/* derived event number n is 256 + n */

#define UPSCRL		1		/* message field contents for */
#define DOWNSCRL	2		/* the derived event SCROLLEVENT */
#define LEFTSCRL	3
#define RIGHTSCRL	4
#define VERTSCRL	5
#define HORIZSCRL	6

/*                                                            *****************
                                                              *               *
                                                              * W_CTRLINFO    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Information about a control passed to an event handler.                     |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int		id;
   int		userid;
   short	inareg;
   int		state;
} w_ctrlinfo;

/*                                                            *****************
                                                              *               *
                                                              * W_EVSTRUCT    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| 'Event' record for the action functions assiciated with controls.           |
-------------------------------------------------------------------------------
*/

typedef struct
{
   point	where;
   word		what;
   Dword	message;
   int		when;
   int		modifiers;
   int		fd;
   w_ctrlinfo	ctrl;
} w_EventRec;

