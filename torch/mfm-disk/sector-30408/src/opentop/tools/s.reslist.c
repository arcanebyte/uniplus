h49366
s 00036/00034/00145
d D 1.6 86/11/06 10:05:33 ian 6 5
c All change! rs -> reslist!
e
s 00003/00000/00176
d D 1.5 86/11/05 11:03:39 ian 5 4
c Added 1.3 version string
e
s 00044/00008/00132
d D 1.4 86/07/11 12:50:13 ian 4 3
c Added comments
e
s 00001/00001/00139
d D 1.3 86/07/11 09:37:02 ian 3 2
c mmi.h should ONLY be in /usr/include/mmi so use <mmi/mmi.h> DMG
e
s 00001/00000/00139
d D 1.2 86/07/08 09:55:03 ian 2 1
c Fixed syntax error
e
s 00139/00000/00000
d D 1.1 86/07/08 09:47:03 ian 1 0
c date and time created 86/07/08 09:47:03 by ian
e
u
U
t
T
I 4
/*****************************************************************************
 *
D 6
 * Program:	rs
 * Filename:	rs.c
E 6
I 6
 * Program:	reslist
 * Filename:	reslist.c
E 6
 * Function:	main, ProcessFile, Type, Error 
 *
D 6
 * Author:	I. Jones
E 6
I 6
 * Author:	IJ
E 6
 * Date:	November 1985
 *
 * Purpose:	To list the contents of resource files.
 *
D 6
 * Syntax:	rs [files...]
E 6
I 6
 * Syntax:	reslist [files...]
E 6
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

E 4
I 1
#include <stdio.h>
#include "std.h"
D 3
#include <mmi.h>
E 3
I 3
#include <mmi/mmi.h>
E 3
#include <resource.h>
I 4
#define STDIN		0
E 4

I 4
/* ---------- Forward declarations: ------------------------------- */

E 4
extern char *Type();

D 4
/* SCCS identification string. */
E 4
I 4
/* ---------- Exported external variables/functions: -------------- */
E 4

D 4
static char SCCSid[] = "%A%";
E 4
I 4
/* ---------- Imported external variables/functions: -------------- */
E 4

D 4
char filename[ 80 ];
E 4
I 4
/* ---------- Static variables: ----------------------------------- */
E 4

I 5
static char version [] =
"(C) Copyright 1986 TORCH Computers Ltd Version 1.30\nTriple X";

E 5
D 4
#define STDIN		0
E 4
I 4
/* SCCS identification string. */
E 4

I 4
static char SCCSid[] = "%A%";
static char filename[ 80 ];
D 6

E 6
I 6
static char *progname;
E 6
/*                                                                *************
                                                                  *           *
                                                                  *  MAIN     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  The entry point                                                            |
-------------------------------------------------------------------------------
*/
E 4
main( argc, argv )
int argc;
char *argv[];
{
D 4
    resid resfileid;
I 2
    int i;
E 4
I 4
    resid resfileid;	/* Resource file identifier */
    register int i;
E 4
E 2

I 6
    progname = argv[ 0 ];
    
E 6
I 4
/* If we have no command line arguments, read from standard input */

E 4
    if( argc == 1 )
    {
	strcpy( filename, "*standard-input*" );
	resfileid = ResFDOpen( STDIN, "r" );
	ProcessFile( resfileid );
    }
    else
I 4

/* Otherwise, process all the files on the command line */
E 4
    {
	for( i = 1; i < argc; i++ )
	{
	    strcpy( filename, argv[ i ] );
	    resfileid = ResOpen( filename, "r" );
	    ProcessFile( resfileid );
	}
    }
}

/*                                                                *************
                                                                  *           *
                                                                  *PROCESSFILE*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   List all objects in a resource file.                                      |
-------------------------------------------------------------------------------
*/

ProcessFile( resfile )
resid resfile;
{
D 4
    objid objectid;
E 4
I 4
    objid objectid;		/* Current object identifier */
E 4

    if( resfile == ERROR )
    {
	Error();
	return;
    }
    else
	printf( "Resource file title = '%s'\n\n", ResTitle( resfile ) );

    for( objectid = ResFirst( resfile, RESANY ); objectid != ERROR;
	 objectid = ResNext( resfile, RESANY ) )
    {
	printf( "%-15s %15s %6d/%-6d %s\n", Type( ResObjType( objectid ) ),
		 ResObjName( objectid ), ResObjSize( objectid ),
		 ResObjOffset( objectid ), filename );
    }
    ResClose( resfile );
    putchar( '\n' );
}

/*                                                                *************
                                                                  *           *
                                                                  * TYPE      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Return a mnemonic for a resource type.                                     |
-------------------------------------------------------------------------------
*/
char *Type( x )
int x;
{
    char *result;
    switch( x )
    {
D 6
	case  RESNULL:   result = "deleted"; break;
	case  RESFORM:   result = "form"; break;
	case  RESICON:   result = "icon"; break;
	case  RESFONT:   result = "font"; break;
	case  RESCURSOR: result = "cursor"; break;
	case  RESHTONE:  result = "halftone"; break;
	case  RESPALETTE:result = "palette"; break;
	case  RESCONFIG: result = "configuration"; break;
E 6
I 6
	case  RESNULL:   result = "*Deleted*"; break;
	case  RESFORM:   result = "Form"; break;
	case  RESICON:   result = "Icon"; break;
	case  RESFONT:   result = "Font"; break;
	case  RESCURSOR: result = "Cursor"; break;
	case  RESHTONE:  result = "Halftone"; break;
	case  RESPALETTE:result = "Palette"; break;
	case  RESCONFIG: result = "Configuration"; break;
E 6
	case  XXXMAGIC:  result = "*Header*"; break;
D 6
	case  RESMENU:   result = "menu"; break;
	case  RESTEXT:   result = "text"; break;
	case  RESPOINT:  result = "point"; break;
	case  RESRECT:   result = "rectangle"; break;
	case  RESINT:    result = "int vector"; break;
	case  RESDOUBLE: result = "double vector"; break;
	case  RESSTRING: result = "string"; break;
	case  RESWINPOS: result = "window position"; break;
	case  RESPADPOS: result = "pad position"; break;
	case  RESPICTURE:result = "picture"; break;
	case  RESPRESTEL:result = "prestel page"; break;
	case  RESFAXT3:	 result = "fax T3"; break;
	case  RESFAXT4:  result = "fax T4"; break;
E 6
I 6
	case  RESMENU:   result = "Menu"; break;
	case  RESTEXT:   result = "Text"; break;
	case  RESPOINT:  result = "Point"; break;
	case  RESRECT:   result = "Rectangle"; break;
	case  RESINT:    result = "Int vector"; break;
	case  RESDOUBLE: result = "Double vector"; break;
	case  RESSTRING: result = "String"; break;
	case  RESWINPOS: result = "Window position"; break;
	case  RESPADPOS: result = "Pad position"; break;
	case  RESPICTURE:result = "Picture"; break;
	case  RESPRESTEL:result = "Prestel page"; break;
	case  RESFAXT3:	 result = "Fax T3"; break;
	case  RESFAXT4:  result = "Fax T4"; break;
E 6
	case  RESNAPLPS: result = "NAPLPS"; break;
	case  RESCEPT:	 result = "Cept3"; break;
	case  RESNEWPAL: result = "New palette"; break;
D 6
	case  RESFILENAME:result = "filename"; break;
	case  RESENVIRONMENT:result = "environment"; break;
E 6
I 6
	case  RESFILENAME:result = "Filename"; break;
	case  RESENVIRONMENT:result = "Environment"; break;
E 6
	case  RESFIRSTWORD:result = "1stword"; break;
D 6
	case  RESSOUND:    result = "sound"; break;
	case  RESKBDMATRIX:result = "keyboard matrix"; break;
	case  RESTERMIO:   result = "tty settings"; break;
	case  RESDIALOGUE: result = "dialogue template"; break;
E 6
I 6
	case  RESSOUND:    result = "Sound"; break;
	case  RESKBDMATRIX:result = "Keyboard matrix"; break;
	case  RESTERMIO:   result = "TTY settings"; break;
	case  RESDIALOGUE: result = "Dialogue template"; break;
E 6

D 6
	default:        result = "unknown"; break;
E 6
I 6
	default:        result = "Unknown"; break;
E 6
    }
    return( result );
}

/*                                                                *************
                                                                  *           *
                                                                  *  ERROR    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 4
|  Snuff it.                                                                  |
E 4
I 4
|  Something is amiss. Tell the user and press on regardless.                 |
E 4
-------------------------------------------------------------------------------
*/

Error()
{
D 6
    fprintf( stderr, "rs: %s\n", ResError() );
E 6
I 6
    fprintf( stderr, "%s: %s\n", progname, ResError() );
E 6
}
E 1
