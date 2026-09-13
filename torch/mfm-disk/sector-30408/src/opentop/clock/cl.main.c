/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.main.c
 * Function:	main(), Update, DrawClock(), GetTime, DrawTime(),
 *		InvertFace(), GetFInfo(), MLDEvent(), ClEvent(), Fatal()
 *
 * Purpose:	To display an analogue or digital clock, with an optional
 *		date.
 *
 * Author:	MPA & DMG 
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <math.h>
#include <time.h>
#include <termio.h>
#include <fcntl.h>
#include <stdio.h>
#include <wlib.h>
#include <resource.h>
#include "clock.h"

/* ---------- Imported external variables/functions: -------------- */

extern int errno;   		/* System error code */
extern int fd;			/* Current window file descriptor */
extern int inverse;		/* TRUE if we invert the display */

/* ---------- Exported external variables/functions: -------------- */

font thefont;			/* The font we happen to be using */

char *datestring;		/* The date (from asctime() see time(3C) */
double  hours;			/* Hours, mins and secs!!!! */
double minutes;
double seconds;		
double twopi = 2 * 3.141592653;	/* Used for circle calculations */
int analogue = TRUE;		/* TRUE if using analogue face */
int date = FALSE;		/* TRUE if displaying the date */
int backhtone;			/* The background halftone handle number */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.main.c 1.4@(#)";


				/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 0.10\nTriple X";

				/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.20
 *	     26/9/85	face drawing routine altered DMG 
 *           March 86   extensively altered by MPA
 *
 */

static int secs;	/* Time since 1970 (See time(2)) in seconds */
static int tosleep;	/* Time to sleep before next update */

/*                                                                *************
                                                                  *           *
                                                                  *   MAIN    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| The entry point                                                             |
-------------------------------------------------------------------------------
*/

main (argc, argv)
int argc;
char *argv[];
{
/* we are the front window to start */

   inverse = TRUE;

/* Disable events whilst we set ourselves up */

   DisEvents ();

/* Open a window in correct place and of correct size */

   OpenWindow ();

/* Clear to a nice pattern. This will always be the background pattern */

   ClearClock ();

/* Initialise the menu */

   InitMenu ();

/* Read all about our font */

   GetFInfo ();

/* Calculate origin of date string */

   DateOrigin ();

/* Calculate origin of time string */

   TimeOrigin ();

/* Don't display the text cursor */
 
   HideCursor ();

/* Draw the clock face */

   DrawClock ();

/* Draw the close control */

   InitClose ();

/* Draw time etc */

   GetTime ();
   DrawTime (DRAWTIME);

/* Ok, can re-enable events now */

   EnEvents ();

/* Enter an infinite loop, waiting for a minute before updating the display: */

   do
   {
      sleep (60 - (tosleep % 60));
      Update ();
   } while ( TRUE );
}

/*                                                                *************
                                                                  *           *
                                                                  *  UPDATE   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Update the clock display                                                  |
-------------------------------------------------------------------------------
*/

Update()
{
   register drawdate = FALSE;

/* read current time in secs. Used for a variety of purposes */

   tosleep = time (0);

/* Has the date changed and should we display it if so? */

   if ((secs/SECSPERDAY != tosleep/SECSPERDAY) && date)
      drawdate = TRUE;

/* Has the time changed? */

   if (secs/60 != tosleep/60)
   {
/* Dont let any odd events come in while we are drawing */

      DisEvents ();

/* Erase the old positions of the clock hands */

      DrawTime(ERASE);

/* Get the new time: */

      GetTime();

/* Should we change the date? */

      if (drawdate)
         DrawDate ();

/* ... and draw the new clock hands */

      DrawTime(DRAWTIME);

/* Re-enable events */

      EnEvents ();
   }

}

/*                                                                *************
                                                                  *           *
                                                                  * DRAWCLOCK *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Draw the clock face either in analogue or digital                           |
-------------------------------------------------------------------------------
*/

DrawClock ()
{
   if (analogue)
      DAnClock ();
   else
      DDigClock ();

   Flush ();	/* Flush the buffers */
}

/*                                                                *************
                                                                  *           *
                                                                  *  GETTIME  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Read the current system time                                              |
-------------------------------------------------------------------------------
*/

GetTime()
{
   struct tm *mytime;

   secs = time( 0 );

/* convert to the local time i.e. GMT or BST in England */

   mytime = localtime (&secs);

/* Make the datestring */

   datestring = asctime (mytime);

/* Format is like "Sun Sep 16 01:03:52 1973\n", Just take the bit we need  */

   *(datestring+10) = '\0';

/* store the hours etc........ */

   seconds = mytime->tm_sec;
   minutes = mytime->tm_min;
   hours   = mytime->tm_hour % 12;
}

/*                                                                *************
                                                                  *           *
                                                                  * DRAWTIME  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Draw the time in either analogue or digital                                 |
-------------------------------------------------------------------------------
*/

DrawTime (mode)
register mode;

{
   if (analogue)
      DAnTime (mode);
   else
      DDigTime (mode);

   Flush ();	/* Flush the buffers */
}

/*                                                                *************
                                                                  *           *
                                                                  *INVERTFACE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Invert the face of the clock                                                |
-------------------------------------------------------------------------------
*/

InvertFace (mode)
register mode;

{
   if (analogue)
      IAnFace ();
   else
      IDigFace ();

   Flush ();	/* Flush the buffers */
}

/*                                                                *************
                                                                  *           *
                                                                  * GETFINFO  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Read all about the font in use                                              |
-------------------------------------------------------------------------------
*/

GetFInfo ()
{
   utextstate state;
   struct mystruct
   {
      int res;
      int handle;
      font *fptr;
   } infostruct;
   register utextstate *tptr = &state;
   register struct mystruct *iptr = &infostruct;

/* Need to find out what font we are using */

   GetFontState ( &state );

/* Now need the width and height of a character. */

    RdFontInfo( state.fonthandle, &thefont );
}

/*                                                                *************
                                                                  *           *
                                                                  *  MLDEVENT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Called when a left mouse down occurs in our window but not in a control     |
| Note that this routine is only called when we are the front window          |
-------------------------------------------------------------------------------
*/

MLDEvent (evptr)
register EventRec *evptr;

{
   DoDrag ();
}

/*                                                                *************
                                                                  *           *
                                                                  *  CLEVENT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Called when the close box control has been activated.                       |
-------------------------------------------------------------------------------
*/

ClEvent (evptr)
register EventRec *evptr;

{
/* Save the clock's window position for next time */

   SavePosition();

   SetBPat ( NULLBITS );
   exit (0);
}

/*                                                                *************
                                                                  *           *
                                                                  * FATAL     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Issue a fatal error message and exit the program.                         |
-------------------------------------------------------------------------------
*/

Fatal( fun )
char *fun;
{
    fprintf( stderr, "Clock: %s %s\n", fun, WError() );
    exit( 1 );
}
