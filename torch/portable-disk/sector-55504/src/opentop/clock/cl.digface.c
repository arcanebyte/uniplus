/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.digface.c
 * Function:	DDigClock(), IDigFace()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To draw the digital clock display
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

/* A rectangle defining the area of the clock face in locals */

rectangle digface =	/* Clock face - relative to clock window */
{
   16, 16, RADIUS*2, RADIUS*2	/* dummy values */
};

/* ---------- Imported external variables/functions: -------------- */

extern rectangle disprect;	/* rectangle defining the display area */
extern font thefont;		/* the font currently in use */
extern inverse;			/* if true, draw the face hilited */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.digface.c 1.2@(#)";

/*                                                                *************
                                                                  *           *
                                                                  * DDIGCLOCK *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Draw digital clock display                                                |
-------------------------------------------------------------------------------
*/

DDigClock ()
{
   register font *fptr = &thefont;
   register rectangle *rptr = &digface;

/* Ensure pen is down, prior to drawing with it! */

   DropPen();

/* Calculate the box, giving it a gap of half a character all around.
 * Note that the string is 5 characters long */

   rptr->height = 2*(fptr->ascent + fptr->descent);
   rptr->width  = 6*fptr->maxwidth;
   RGToL (&rptr->width);
   rptr->x = (DISPWIDTH - rptr->width) / 2;
   rptr->y = (WINHEIGHT - rptr->height) / 2;

   PenMode (M_COPY);

/* Clear the display area */

   ClrRect (&disprect);

/* Draw the box */

   SetFPat (GREEN);
   OFrameRect (rptr);

/* Clear the face, allowing for the inverse flag */

   SetFPat (BITS_0);

   if (inverse)
      PenMode (M_NOT);
   else
      PenMode (M_COPY);

   FillRect (rptr);
}

/*                                                                *************
                                                                  *           *
                                                                  *  IDIGFACE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Invert the digital face                                                     |
-------------------------------------------------------------------------------
*/

IDigFace ()
{
   InvRect (&digface);
}

