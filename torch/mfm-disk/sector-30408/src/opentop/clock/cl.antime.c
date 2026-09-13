/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.antime.c
 * Function:	DAnTime()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To draw the time on the analogue clock face
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

extern double twopi;
extern double hours, minutes, seconds;
extern int inverse;

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.antime.c 1.3@(#)";

/*                                                                *************
                                                                  *           *
                                                                  *  DANTIME  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Draw the clock hands. 'mode' is ERASE or DRAWTIME                         |
| OX & OY are at the center of the analogue face                              |
-------------------------------------------------------------------------------
*/

DAnTime(mode)
{
   double angle;		/* Angle in radians around the clock face */
   register int pmode = M_COPY; /* Default to drawing in inverse */

   if (mode == ERASE)
   {
      if (!inverse)
         pmode = M_NSANDD;      /* ~source & destination */
      else
         pmode = M_OR;		/* source | destination */
   }
   else
   {
      if (inverse)
         pmode = M_NOT;
   }
   
   PenMode (pmode);

/* Set colour of lines */

   SetFPat (RED);

/* Draw the hour hand (hour hand is twice the thickness of minute hand) */
/* BIGPENWIDTH & BIGPENHEIGHT define the (local coords) pen size */

   PenSize( BIGPENWIDTH, BIGPENHEIGHT );
   MoveTo( OX, OY );
   angle = (hours * twopi) / 12.0 + minutes * twopi / (12 * 60); 

   DrawBy( (int) ( (RADIUS -1 - BIGPENWIDTH - 8 ) * 0.5 * sin( angle ) ),
           (int) (-(RADIUS -1 - BIGPENHEIGHT - 8 ) * 0.5 * cos( angle ) ));

/* Draw the minute hand pen size should be PENWIDTH & PENHEIGHT */

   PenSize (PENWIDTH, PENHEIGHT);
   MoveTo (OX, OY);
   DrawBy ((int) ( (RADIUS -1 - BIGPENWIDTH - 8) * sin( minutes * twopi / 60 )),
           (int) (-(RADIUS -1 - BIGPENHEIGHT - 8) * cos( minutes * twopi / 60 )));
}

