/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.update.c
 * Function:	WUEvent()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To process clock window update events.
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <wlib.h>
#include "clock.h"

/* ---------- Forward declarations: ------------------------------- */

/* ---------- Exported external variables/functions: -------------- */

int inverse;	/* TRUE if clock face should be hilited */

/* ---------- Imported external variables/functions: -------------- */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.update.c 1.2@(#)";

/*                                                                *************
                                                                  *           *
                                                                  *  WUEVENT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Called when an update event is received                                     |
-------------------------------------------------------------------------------
*/

WUEvent (evptr)
register EventRec *evptr;

{
/* The type of update is in the bottom word of the message field */

   if ((evptr->message & UEVMSGMASK) == DEACTMESS && inverse)
   {
      inverse = FALSE;
      InvertFace ();
   }
   if ((evptr->message & UEVMSGMASK) == ACTMESS && !inverse)
   {
      inverse = TRUE;
      InvertFace ();
   }
}


