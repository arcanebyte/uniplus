/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.imenu.c
 * Function:	InitMenu()
 *
 * Author:	MPA
 * Date:	25 June 86
 *
 * Purpose:	To create the clock's menu & insert in the menu bar.
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

int menu;			/* Global menu identifier */

/* ---------- Imported external variables/functions: -------------- */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.imenu.c 1.2@(#)";

/* Define a dividing line string */

static char divline [] =
{
   DIVITEM, 0
};

/* Define the menu itself */

static UMenuRec clockmenu =
{
   MENUID,					/* menu id */
   "Display",					/* menu title */
   ENABLEDFLAG,					/* No-disabled items */
   CHECKANALOGUE,				/* Analogue ticked */
   "Analogue",
   "Digital",
   divline,					/* a dividing line */
   "Date",					/* ... no more items */
   NULL
};

/*                                                                *************
                                                                  *           *
                                                                  *  INITMENU *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Initialise the menu                                                         |
-------------------------------------------------------------------------------
*/

InitMenu ()
{
/* Down-load the menu */

   menu = MakeMenu (&clockmenu);

/* insert into menu bar of window */

   InsMenu ( menu, 0 );

/* and draw it */

   DrawMBar ();
}

