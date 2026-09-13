/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	cl.res.c
 * Function:	LoadPosition, SavePosition, LoadBHTone, DefResFile,
 *		FindResFile,  MakeResFile, ClearClock
 *
 * Author:	MPA, IJ
 * Date:	25 June 86
 *
 * Purpose:	To load the clock background halftone and position from a 
 * 		resource file.
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <wlib.h>
#include <resource.h>
#include "clock.h"

/* ---------- Forward declarations: ------------------------------- */

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

extern int backhtone;			/* Background pattern handle no. */
extern rectangle winrect;		/* Clock position rectangle */

/* ---------- Static variables: ----------------------------------- */

/* SCCS identification string. */

static char SCCSid[] = "@(#) cl.res.c 1.4@(#)";

static char resfn[ 256 ] = "";	/* Clock resource filename */
pattern htonestore;		/* Halftone data */
word *htadr = htonestore;	/* -> halftone */

/*                                                                *************
                                                                  *           *
                                                                  * LOADBHTONE*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Read the background halftone from the clock resource file                  |
-------------------------------------------------------------------------------
*/

LoadBHTone ()
{
   resid resfd;			/* Resource file identifier */
   objid resobj;		/* Object identifier */

/* Find the clock resource file */

   FindResFile( resfn, "clock.res" );

/* Open the resource file for reading */

   if ( (resfd = ResOpen (resfn, "r")) != ERROR)
   {
/* Find the first halftone in the file */

      if ( (resobj = ResFirst (resfd, RESHTONE)) != ERROR)
      {
/* Read the halftone */

         if ( ResRead (resfd, resobj, &htadr ) == ERROR)
	    Fatal( "(ResRead htone)" );
      }
      if( ResClose (resfd) == ERROR )
	 Fatal( "(ResClose)" );
   }
}

/*                                                                *************
                                                                  *           *
                                                                  * CLEARCLOCK*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Clear the clock window.                                                    |
-------------------------------------------------------------------------------
*/

ClearClock()
{
    backhtone = OpPat (htonestore);

/* If we failed to load the halftone, use one we KNOW is on the heap: */

    if( backhtone == ERROR )
	backhtone = DARKGREY;

    SetBPat( backhtone );
    ClearWindow();
}

/*                                                                *************
                                                                  *           *
                                                                  * SAVEPOSIT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Save the clock's window position                                           |
-------------------------------------------------------------------------------
*/

SavePosition()
{
    resid resfile;			/* Resource file id */
    objid obj;				/* Object identifier */
    pattern backpat;			/* Background halftone */

/* Read the window's position */

    RVisArea( &winrect );

/* Make the resource file to hold this information */

    MakeResFile( resfn, "clock.res" );

/* If the file exists, overwrite the previous window position */

    resfile = ResOpen( resfn, "w" );
    if( resfile != ERROR )
    {
	obj = ResFirst( resfile, RESWINPOS );
	if( obj == ERROR )
	{
	    if( ResAdd( resfile, RESWINPOS, "clock-posn", 0,
						    &winrect ) == ERROR )
		Fatal( "(ResAdd posn)" );
	}
	else
	{
	    if( ResWrite( resfile, obj, RESWINPOS, "clock-posn", 0,
						    &winrect ) == ERROR )
		Fatal( "(ResWrite posn)" );
	}
    }
    else

/* Otherwise, add the window position and background halftone */

    {
	resfile = ResCreate( resfn, "Clock" );
	if( ResAdd( resfile, RESWINPOS, "clock-posn", 
		    0, &winrect ) == ERROR )
	    Fatal( "(ResAdd posn)" );

	RdPat( backhtone, backpat );
	if( ResAdd( resfile, RESHTONE, "clock-backpat",
		    0, backpat ) == ERROR )
	    Fatal( "(ResAdd htone)" );
    }
    if( ResClose( resfile ) == ERROR )
	Fatal( "(ResClose)" );;
}

/*                                                                *************
                                                                  *           *
                                                                  * LOADPOSIT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Load the clock's window position                                           |
-------------------------------------------------------------------------------
*/

LoadPosition()
{
    resid resfile;			/* Resource file id */
    objid obj;				/* Object identifier */
    rectangle *rectptr = &winrect;	/* -> -> rectangle */
    int result;				/* TRUE if we found the resource file */

/* Try finding the clock resource file */

    FindResFile( resfn, "clock.res" );

/* Try opening the resource file for reading */

    resfile = ResOpen( resfn, "r" );
    if( resfile != ERROR )
    {
	obj = ResFirst( resfile, RESWINPOS );
	if( obj != ERROR )
	{
	    if( ResRead( resfile, obj, &rectptr ) == ERROR )
		Fatal( "(ResRead posn)" );;

/* We only want the position, not the size! */

	   winrect.width = WINWIDTH;
	   winrect.height = WINHEIGHT;
	   result = TRUE;
	}
	if( ResClose( resfile ) == ERROR )
	    Fatal( "(ResClose)" );;
    }
    else
	result = FALSE;

    return( result );
}

/*                                                                *************
                                                                  *           *
                                                                  *FINDRESFILE*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Look in the standard places for the user's resource file:                  |
-------------------------------------------------------------------------------
*/

FindResFile( pathname, tailpart )
char *pathname;			/* Pathname buffer */
char *tailpart;			/* The tail portion of the pathname */
{
    extern char *getenv();	/* environment enquiries */
    char *dir;			/* -> Environment variable */
    char command[ 256 ];	/* Command string passed to System */

/* Try the user's home directory $HOME/resources/file first: */

    dir = getenv( "HOME" );
    if( dir != NULL )
    {
	strcpy( pathname, dir );
	strcat( pathname, "/resources/" );
	strcat( pathname, tailpart );
    }

/* If we failed to find the user's resource file, check the defaults */

    if( dir == NULL || access( pathname, 0 ) == ERROR )
	DefResFile( pathname, tailpart );
}


/*                                                                *************
                                                                  *           *
                                                                  *MAKERESFILE*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Return the pathname of the user's configured resource file.                |
-------------------------------------------------------------------------------
*/

MakeResFile( pathname, tailpart )
char *pathname;			/* Pathname buffer */
char *tailpart;			/* The tail portion of the pathname */
{
    extern char *getenv();	/* environment enquiries */
    char *dir;			/* -> Environment variable */
    char command[ 256 ];	/* Command string passed to System */

/* Try the user's home directory $HOME/resources/file first: */

    dir = getenv( "HOME" );
    if( dir != NULL )
    {

/* Create the subdirectory if necessary */

	strcpy( pathname, dir );
	strcat( pathname, "/resources" );
	if( access( pathname, 0 ) == ERROR )
	{
	    sprintf( command, "mkdir %s", pathname );
	    System( command );
	}
	strcat( pathname, "/" );
	strcat( pathname, tailpart );
    }
}

/*                                                                *************
                                                                  *           *
                                                                  * DEFRESFILE*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Find the default resource file.                                            |
-------------------------------------------------------------------------------
*/

DefResFile( pathname, tailpart )
char *pathname;			/* Pathname buffer */
char *tailpart;			/* The tail portion of the pathname */
{
    strcpy( pathname, "/usr/lib/resources/" );
    strcat( pathname, tailpart );
}

