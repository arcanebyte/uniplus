/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.anface.c
 * Function:	DAnClock(), IAnFace()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To draw the clock's analogue face
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <math.h>
#include <time.h>
#include <termio.h>
#include <fcntl.h>
#include <wlib.h>
#include "clock.h"

/* ---------- Forward declarations: ------------------------------- */

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

extern rectangle disprect;	/* Area to use to display clock face. Local */
extern inverse;			/* If true, then show face hilited */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.anface.c 1.2@(#)";

static rectangle clockface =	/* Clock face - relative to clock window */
{
   16, 16, RADIUS*2, RADIUS*2	/* These values are dummy */
};

/*                                                                *************
                                                                  *           *
                                                                  *  DANCLOCK *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Draw the clock face                                                         |
-------------------------------------------------------------------------------
*/

DAnClock ()
{
   int ang;
   register int x;

/* Ensure pen is down, prior to drawing with it! */

   DropPen();

/* Clear the window */

   ClrRect (&disprect);

   if (inverse)
      PenMode (M_NOT);
   else
      PenMode (M_COPY);

/* Clear the clock face */

   SetFPat (BITS_0);
   FillEll (&clockface);

/* Draw the clock perimiter in green */

   PenMode (M_COPY);
   SetFPat (GREEN);
   OFrameEll (&clockface); 

/* Draw the hour markers in red */

   if (inverse)
      SetFPat (GREEN);
   else
      SetFPat (RED);

   for( ang = 0, x = 0; x < 12; x++, ang += 30 )
   {

/* Draw the quarter-hour markers slightly larger */

   	if( x % 3 == 0 )
	{
      	    PenSize( BIGPENWIDTH, BIGPENHEIGHT );
      	    FrameArc( ang - 3, ang + 3, &clockface );
   	}
   	else
   	{
      	    PenSize( PENWIDTH, PENHEIGHT );
      	    FrameArc( ang - 1, ang + 1, &clockface );
   	}
   }
}

/*                                                                *************
                                                                  *           *
                                                                  *  IANFACE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Invert the face of the analgue clock                                        |
-------------------------------------------------------------------------------
*/

IAnFace ()
{
   InvEll (&clockface);
}

