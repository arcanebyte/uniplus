/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.mevent.c
 * Function:	MSEvent()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To process the clock menu events
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

extern int analogue;	
extern int date;	
extern int menu;		/* The menu id */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.mevent.c 1.2@(#)";

/*                                                                *************
                                                                  *           *
                                                                  *  MSEVENT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| An item has been selected from our menu.                                    |
-------------------------------------------------------------------------------
*/

MSEvent (evptr)
register EventRec *evptr;

{
/* the item selected is in the bottom byte */

   switch (evptr->message & 0xff)
   {
/* Toggle to analogue display */

      case ANALOGUE :
         analogue = TRUE;
         DisTick (menu, DIGITAL);		/* remove the tick */
         EnableTick (menu, ANALOGUE);		/* tick the other item */
         DrawClock ();				/* draw the clock face */
         PenMode (M_COPY);			/* set pen mode right */
         DrawTime (DRAWTIME);			/* draw time - don't erase */
         break;

/* Toggle to digital display */

      case DIGITAL :
         analogue = FALSE;
         DisTick (menu, ANALOGUE);
         EnableTick (menu, DIGITAL);
         DrawClock ();
         PenMode (M_COPY);
         DrawTime (DRAWTIME);
         break;

/* Toggle date display */

      case DATE :
         date ^= 1;
         if (date)
         {
            EnableTick (menu, DATE);
            DateWindow (TRUE);		/* Grow the window */
            DrawDate ();
         }
         else
         {
            DisTick (menu, DATE);
            DateWindow (FALSE);		/* Shrink the window */
         }
         break;

/* Should never get here */

      default :
         break;
   }
}

