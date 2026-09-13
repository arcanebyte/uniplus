/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.date.c
 * Function:	DrawDate(), DateWindow(), DateOrigin()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To draw the date in the clock window.
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

extern rectangle winrect;	/* window rectangle in globals */

extern FILE *fs;		/* the window character stream */
extern char *datestring;	/* A string containing the date, 10 long */

extern font thefont;		/* The font currently in use */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.date.c 1.3@(#)";

static rectangle daterect;	/* A rectangle defining date area in locals */
static point dateline;		/* Move pen to here before drawing the date */

/*                                                                *************
                                                                  *           *
                                                                  * DRAWDATE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

DrawDate ()
{
/* Set the text mode */

   TextMode (M_COPY);

/* Draw in blue (in standard screen mode) */

   SetFPat (NULLBITS);

/* Move pen to correct place */

   MoveTo (dateline.x, dateline.y);

/* print it */

   fprintf (fs, "%s", datestring);
}

/*                                                                *************
                                                                  *           *
                                                                  * DATEWINDOW*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Change size of window as much as necessary and draw the date box.           |
-------------------------------------------------------------------------------
*/

DateWindow (grow)
{
   register dh, dw;
   rectangle temprect;

/* Calculate the change in width and height necessary for date */

/* Use our own version of the date box */

   temprect = daterect;

/* Convert to global coordinates */

   RLToG (&temprect.width);
   RLToG (&temprect.x);

/* Calculate dw, allowing some space around box */

   dw = temprect.width + 2 * temprect.x - winrect.width;

/* If it would shrink the width, don't */

   if (dw < 0)
      dw = 0;

/* Calculate dh, allowing some space around box */

   dh = temprect.height * 2;

/* If we are shrinking not growing, then invert the sign */

   if (!grow)
   {
      dh = -dh;
      dw = -dw;
   }

/* Now change size of window */

   ModWindow (0, 0, dw, dh);

/* If growing, set up the date box */

   if (grow)
   {
/* clear the box to white */

      PenMode (M_COPY);
      SetFPat (BITS_0);
      FillRect (&daterect);

/* and then frame it green */

      SetFPat (GREEN);
      OFrameRect (&daterect);
   }
}

/*                                                                *************
                                                                  *           *
                                                                  * DATEORIGIN*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Calculate the origin of the date text.                                      |
-------------------------------------------------------------------------------
*/

DateOrigin ()
{
/* Calculate size of date box allowing space around text */

   daterect.width = 10 * thefont.maxwidth;
   daterect.height = thefont.ascent + thefont.descent;
   RGToL (&daterect.width);

/* now have the size of the text in local coordinates */

   daterect.width += 16;      /* allows 8 units all round */
   daterect.height += 16;

/* Calculate origin of box */

   daterect.x = 16;		/* 16 units in from edge of window */
   daterect.y = daterect.height / 2 + WINHEIGHT; 

/* Calculate origin of text */

   dateline.y = daterect.y + 8;  /* allow 8 units all round */
   RLToG (&dateline);            /* font ascent is in globals */
   dateline.y += thefont.ascent;
   RGToL (&dateline);
   dateline.x = 24;              /* 8 units in from edge of box */
}
