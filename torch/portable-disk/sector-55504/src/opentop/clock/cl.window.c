/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.window.c
 * Function:	OpenWindow()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To open the clock's window.
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <termio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <wlib.h>
#include "clock.h"

/* ---------- Forward declarations: ------------------------------- */

extern int MSEvent (); 		/* Menu select events */
extern int MLDEvent (); 	/* Left mouse down events */
extern int WUEvent ();		/* Window update events */

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

extern int fd;

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.window.c 1.3@(#)";

/* The action routine vector */

static int (*actions [NUMPROCS]) () =
{
   MSEvent,		/* Menu select event */
   NULL,
   MLDEvent,		/* left mouse button down event */
   NULL,
   NULL,		/* left mouse button up event */
   NULL,
   NULL,
   NULL,
   NULL,
   WUEvent,		/* window update event */
   NULL,		/* over bounds event */
   NULL,		/* ... etc */
};

rectangle winrect =	/* Window position, and size initially in locals */
{
   2512, 160, WINWIDTH, WINHEIGHT
};

rectangle disprect =	/* Clock face area in locals */
{
   0, 0, DISPWIDTH, WINHEIGHT
};

/*                                                                *************
                                                                  *           *
                                                                  *OPENWINDOW *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

OpenWindow ()
{
   int loaded;		/* TRUE if we loaded the window position */

/* Load the background halftone */

   LoadBHTone();

/* Open a TTY structure for the window: Dont open the window itself yet */

   if( WOpen( 0, WOTTYONLY | WOANYUNIQUE, 0, 0, 0, 0 ) < 0 )
       Fatal( "(WOpen)" );

/* Load the old clock position - if any */

   loaded = LoadPosition();

/* Convert the window position and size from local coordinates (window */
/* relative) to global coordinates (screen relative - based on pixels  */

   if( !loaded )
       RLToG( &( winrect.x ) );

   RLToG( &( winrect.width ) );

/* Disable character post-processing and block keyboard input */

   PPOff();
   KBOn();

/* Now open the actual window associated with the TTY structure: */

   if( WShow( fd, &winrect, VANILLA, NULL, actions, 0) < 0 )
       Fatal( "(WShow)" );

/* Prevent the editarea having any effect on character wrapping */

   SetEdBits (NOWRAP);
}
  
