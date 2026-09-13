/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.digtime.c
 * Function:	DDigTIme(), TimeOrigin()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To draw the digital time
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

extern font thefont;		/* The font we are using */

extern FILE *fs;		/* The window character stream */
extern double hours, minutes;	/* Time in double precision! */
extern rectangle digface;	/* Size of digital face in locals */

extern int inverse;		/* If true, draw time hilited */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.digtime.c 1.2@(#)";

static point origin;		/* Coords of string in locals */

/*                                                                *************
                                                                  *           *
                                                                  * DDIGTIME  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| 'mode' is ERASE or DRAWTIME                                                 |
-------------------------------------------------------------------------------
*/

DDigTime (mode)
int mode;
{
/* If we are supposed to be erasing the old time, don't bother */

   if (mode != ERASE)
   {
/* Do it in black */

      SetFPat (NULLBITS);

      if (!inverse)
         TextMode (M_COPY);
      else
         TextMode (M_NOT);

/* Move pen to digital face text origin */

      MoveTo (origin.x, origin.y);

/* Write out the time */

      fprintf (fs,"%2d:%02d", (int) hours == 0 ? 12 : (int) hours, (int) minutes);
   }
}

/*                                                                *************
                                                                  *           *
                                                                  * TIMEORIGIN*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Calculate the origin for the text.                                          |
| Put the result in origin.x & origin.y in local coordinates                  |
-------------------------------------------------------------------------------
*/

TimeOrigin ()
{
   register font *fptr = &thefont;
   point temp;

   temp.x = 5*fptr->maxwidth;
   temp.y = fptr->ascent;
   RGToL (&temp);

   origin.x = (DISPWIDTH - temp.x)/2;
   origin.y = (WINHEIGHT + temp.y)/2;
}
