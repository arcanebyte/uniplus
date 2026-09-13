/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.close.c
 * Function:	InitClose()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To open the clock close box control
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <math.h>
#include <time.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <wlib.h>
#include "clock.h"

/* ---------- Forward declarations: ------------------------------- */

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

extern int ClEvent ();		/* Called when control is activated */

extern font thefont;		/* The font we are using */
extern rectangle disprect;	/* rectangle used for displaying clock face */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.close.c 1.2@(#)";

static rectangle closerect;	/* local rectangle framing close control */
static char *closestr ={"\262"};/* The XXX close box symbol */
static int closectrl;		/* the returned control id for the close box */

/*                                                                *************
                                                                  *           *
                                                                  * INITCLOSE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Initialise the close control.                                               |
-------------------------------------------------------------------------------
*/

InitClose ()
{
/* First calculate the size of the bounding box */

   closerect.width = thefont.maxwidth;
   closerect.height = thefont.ascent + thefont.descent;
   RGToL (&closerect.width);
   closerect.width += 16;
   closerect.height += 16;

/* Make it square */

   if (closerect.width < closerect.height)
      closerect.width = closerect.height;
   else
      closerect.height = closerect.width;

/* center it in non-display part of window. Note the local window width and 
 * height are constants defined by WINWIDTH & WINHEIGHT */

   closerect.x = (WINWIDTH - disprect.width - closerect.width) / 2
                    + disprect.width;
   closerect.y = (WINHEIGHT - closerect.height) / 2;

/* open the control */

   closectrl = OpCtrl (PUSHBUT,		/* Control type */
                       CLOSECTRL,	/* User id for control */
                       &closerect,	/* The bounding box */
                       0,		/* The active region - same as above */
                       closestr,	/* The label */
                       0, 0,		/* Half-tones and form - none */
                       ClEvent,		/* Called when control is activated */
                       0);		/* Flags... */

/* OpCtrl leaves it hidden (not visible), so show it! */

   ShowCtrl (closectrl);
}
