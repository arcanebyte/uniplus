h59962
s 00007/00007/01769
d D 1.10 86/11/05 16:48:34 ian 11 10
c All change! restool is dead, long live resshow!
e
s 00028/00007/01748
d D 1.9 86/11/05 12:04:26 ian 10 9
c Made rename alter internal name too.
e
s 00096/00043/01659
d D 1.8 86/11/05 10:03:13 ian 9 8
c Misc fixes for rename, filename for title etc.
e
s 00059/00015/01643
d D 1.7 86/09/08 13:47:23 ian 8 7
c Fixed read-only filing system bug
e
s 00006/00003/01652
d D 1.6 86/08/22 11:38:16 ian 7 6
c Made RESSTRING == RESTEXT, added a free() to Paste() and improved efficiency of text display.
e
s 00622/00279/01033
d D 1.5 86/08/13 15:23:08 root 6 5
c Lots of fixes by Paul and David
e
s 00348/00105/00964
d D 1.4 86/07/29 11:17:03 ian 5 4
c Modified to cope with any resource type.
e
s 00634/00049/00435
d D 1.3 86/07/29 08:49:00 ian 4 2
c modified to use argv to specify a file (MPA)
e
s 00000/00000/00484
d R 1.3 86/05/30 11:27:17 ian 3 2
c Nothing changed
e
s 00001/00002/00483
d D 1.2 86/05/29 17:36:28 ian 2 1
c Changed neopoliton window to strawberry
e
s 00485/00000/00000
d D 1.1 86/05/29 17:28:26 ian 1 0
c date and time created 86/05/29 17:28:26 by ian
e
u
U
t
T
I 1
/*****************************************************************************
 *
D 6
 * Program:	Slideshow
 * Filename:	slideshow.c
E 6
I 6
D 11
 * Program:	Restool
 * Filename:	restool.c
E 11
I 11
 * Program:	Resshow
 * Filename:	resshow.c
E 11
E 6
 *
D 6
 * Author:	IJ
E 6
I 6
 * Author:	IJ & MPA
E 6
 * Date:	27 May 86
 *
D 6
 * Purpose:	To provide 'rolling demo' for Unix ix show.
E 6
I 6
 * Purpose:	Provides a means of 'viewing' and manipulating a resource file.
E 6
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <stdio.h>
#include <termio.h>
#include <wlib.h>
D 6
#include "std.h"
#include "resource.h"
E 6
I 6
#include <string.h>
#include <errno.h>
#include <mmi/mmierror.h>
#include <resource.h>
E 6

/* ---------- Forward declarations: ------------------------------- */

#define MIN( x, y )	((x) < (y) ? (x) : (y))
#define MAX( x, y )	((x) > (y) ? (x) : (y))

D 6
#define BPWD		16
#define ROWWORDS( w )	(((w) + BPWD - 1 ) / BPWD)
D 4
#define EDITMENU	0
E 4

I 4
#define EDITMENU	1
#define PICMENU		2
E 6
I 6
#define EDITMENU	1	/* Menu id's */
#define OBJMENU		2
E 6
#define DEMOMENU	3

E 4
D 6
#define SCREENWIDTH	736
E 6
I 6
#define SCREENWIDTH	736	/* Some screen constants */
E 6
#define SCREENHEIGHT	256
I 4
D 6
#define SCREENPLANES	2
E 6
E 4

D 6
#define MINWINWIDTH	76
E 6
I 6
#define MINWINWIDTH	76	/* Window size constraints in globals */
E 6
#define MINWINHEIGHT	38
#define MAXWINWIDTH	SCREENWIDTH
#define MAXWINHEIGHT	(SCREENHEIGHT - MBARHEIGHT)
#define STEPS		10

D 6
#define XSCALE		4
E 6
I 6
#define XSCALE		4	/* Scaling constants */
E 6
#define YSCALE		8
D 6
#define PLANES		2
#define OFFSET 		10000
E 6

D 4
#define SELITEM		0
#define HIDESHOW	2
E 4
I 4
#define CUTITEM		0	/* Edit menu */
#define COPYITEM	1
#define PASTEITEM	2
I 6
#define RENAMEITEM	4
E 6
E 4

I 4
#define DEMOITEM	0	/* Demo menu */

D 6
#define MOREITEM	0	/* Picture menu */
E 6
I 6
#define MOREITEM	0	/* Object menu */
E 6
#define LESSITEM	1

I 6
/* Below is because of bug in kernel versions < 1.3 */

#define JUSTSTRING	"\35\3"
#define EDBITSON	"\33L\1"
#define EDBITSOFF	"\33L\200"	/* Cant have \0 in a string */

E 6
D 9
#define SLIDEFILE	"/usr/lib/resources/slideshow.res"
E 9
I 9
D 11
#define SLIDEFILE	"/resources/restool.res"
E 11
I 11
#define SLIDEFILE	"/resources/resshow.res"
E 11
E 9

I 9
extern char *BaseName();

E 9
extern int MenuSEvent();
E 4
extern int CloseEvent();
extern int WUPDEvent();
extern int ScrollEvent();
extern int LMDEvent();

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

D 6
extern char *getenv();		/* FROM C library */
extern int werrno;		/* FROM wlib */
E 6
I 6
extern char *malloc();		/* FROM C library */
I 9
extern char *getenv();          /* FROM C library */
extern int optind;              /* FROM C library */
extern char *optarg;            /* FROM C library */

E 9
E 6
extern int fd;			/* FROM wlib */
extern FILE *fs;		/* FROM wlib */
I 6
extern int rerrno;		/* Resource manager error number */
E 6

/* ---------- Static variables: ----------------------------------- */

I 10
static char version [] =
"(C) Copyright 1986 TORCH Computers Ltd Version 1.30\nTriple X";

E 10
/* SCCS identification string. */

static char SCCSid[] = "%A%";

D 6
static word *nullbitmap = NULL;
static form screenform = 	/* SCREEN FORM */
{
    &nullbitmap,
    0, 0,			/* x, y (filled in later) */
    0, 0,  			/* Width and height (filled in later) */
    0,				/* Number of words horizontally */
    0,				/* Unused */
    PLANES,			/* No. bit planes */
    NULL
};
D 5
static int screenhan = ERROR;	/* Init. value is sentinel for 1st time thru */
E 5
I 5
static int s_han1 = ERROR;	/* Init. value is sentinel for 1st time thru */
E 6
I 6
/* Global handle numbers. Initial value is sentinel for 1st time thru */
E 6
E 5

I 6
static int s_han1 = ERROR, s_han2 = ERROR;

E 6
static int finished = FALSE;	/* TRUE when window close event arrives */
static int nextpiccie = FALSE;	/* TRUE if left mouse down - jump to next */
I 4
static int doingdemo = FALSE;	/* TRUE while performing demo */
static int emptyfile = TRUE;	/* TRUE if the file is empty */
static int readonly = FALSE;	/* TRUE if we only opened slideshow file r/o */
static int needscomp = FALSE;	/* TRUE if needs compression */
I 6
static int getnewname = FALSE;  /* TRUE if we need to get a new name */
E 6

E 4
static int window;		/* Window file descriptor */

D 6
static w_scrollpad padinfo;	/* The clipboard's pad */
E 6
I 6
D 11
static w_scrollpad padinfo;	/* The restool's pad */
E 11
I 11
static w_scrollpad padinfo;	/* The resshow's pad */
E 11
E 6

static rectangle winrect = 	/* Window rectangle in globals */
{
    50, 50, 150, 75
};

D 6
static rectangle selrect;  	/* Selection rectangle (globals) */
E 6
I 6
static rectangle objrect;  	/* Object boundary rectangle (globals) */
E 6

I 4
D 6
static int delay = 5;		/* Default delay between pictures */
E 6
I 6
static int delay = 5;		/* Default delay between pictures (secs) */
E 6

D 6
static resid resfile;		/* Slideshow resource file */
E 6
I 6
static resid resfile;		/* Restool resource file */
E 6
static objid objects[ 500 ];	/* Max number of objects in file */
static int numobj = 0;		/* Current number of objects in file */
static int curobj = -1;		/* The currently selected object */

D 6
static char slidefile[] = SLIDEFILE;
E 6
I 6
D 9
static char slidefile[] = SLIDEFILE;	/* default resource file */
E 9
I 9
static char slidefile[ 80 ];	/* default resource file */
E 9
E 6

/* The menu structures */

I 6
static char divline[] = { DIVITEM, 0 };

E 6
static UMenuRec demomenu = 
{
    DEMOMENU,			/* Menu ID */
    "Demo",			/* Menu title */
    0xffffffff,			/* Menu options enabled */
    0,				/* Menu options ticked  */
    {				/* Text of menu strings. */ 
         "Running demo",
	 NULL
    }
};

static UMenuRec editmenu = 
{
    EDITMENU,			/* Menu ID */
    "Edit",			/* Menu title */
D 6
    0xFFFFFFFF,			/* Menu options enabled */
E 6
I 6
D 8
    0xFFFFFFEF,			/* Menu options enabled */
E 8
I 8
    0xFFFFFFFF,			/* Menu options enabled */
E 8
E 6
    0,				/* Menu options ticked  */
    {				/* Text of menu strings. */ 
	 "Cut",
	 "Copy",
	 "Paste",
I 6
	 divline,
	 "Rename",
E 6
	 NULL
    }
};

D 6
static UMenuRec picmenu = 
E 6
I 6
static UMenuRec objmenu = 
E 6
{
D 6
    PICMENU,			/* Menu ID */
    "Picture",			/* Menu title */
E 6
I 6
    OBJMENU,			/* Menu ID */
    "Object",			/* Menu title */
E 6
    0xFFFFFFFF,			/* Menu options enabled */
    0,				/* Menu options ticked  */
    {				/* Text of menu strings. */ 
	NULL
    }
};

int demohan;			/* Menu handles */
int edithan;
D 6
int pichan;
E 6
I 6
int objhan;
E 6

E 4
/* Event processing routines */

static int ( *eventactions[ NUMPROCS ] ) () =
{
D 4
    0,			/* Menu select event */
E 4
I 4
    MenuSEvent,		/* Menu select event */
E 4
    CloseEvent,		/* Close window event */
    LMDEvent,		/* Left mouse button down event */
    0,			/* Right mouse button down event */
    0,			/* Left mouse button up event */
    0,			/* Right mouse button up event */
    0,			/* Key down event */
    0,			/* Key up event */
    0,			/* Auto-repeat event */
    WUPDEvent,		/* Window update event */
    0,			/* Over bounds event */
    0,			/* Desktop manager event */
    ScrollEvent, 	/* Scroll event */
    0, 0, 0		/* Presently undefined events */
};

I 5
D 9
/* the program's name for error reporting */

D 6
static char *progname;
E 6
I 6
static char *progname, *sf;
E 9
I 9
static char deffile[ 80 ];          /* Default filename */
static char *progname;              /* The program's name */
static char *sf;                    /* Current source file */
static char *curtitle;              /* Window's current title */
E 9
E 6
static char *cantcope = "Can not display this type of resource.";
static char *examplestring = "The Quick Brown Fox Jumps Over The Lazy Dog.";
I 6

D 9
/* Global pointer to objects data */
E 9
I 9
static char *objptr = 0;            /* Global pointer to objects data */
E 9

D 9
static char *objptr = 0;
E 9
I 9
static int deffont;                 /* Default font */
E 9

D 9
/* Default font */

E 6
static int deffont;

E 9
E 5
/*                                                                *************
                                                                  *           *
                                                                  *  MAIN     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|    Slideshow program.                                                       |
E 6
I 6
|    Restool program.                                                         |
E 6
-------------------------------------------------------------------------------
*/

main( argc, argv )
int argc;
char *argv[];
{
D 4
    int delay = 5;
    int t;
    resid resfile;
E 4
I 4
D 6
    char *sf;
E 6
E 4
D 9
    objid object;
E 9
I 9
    char *homedir;                              /* Value of $HOME */
    int opt;                                    /* Current option flag */
    char *optargs = "d:";                       /* getopt args */
    register int i;                             /* Current argv[] element */
E 9
D 5
    form *formptr = NULL;;
E 5
I 5
D 6
    char *objptr = NULL;
E 6
E 5

I 5
/* determine the program's name for error reporting */

    progname = strrchr( argv[0], '/' );		/* find the last '/' */
    if( progname == NULL )
	progname = argv[0];			/* no '/' so use full name */
    else
D 9
	++ progname;				/* step past last '/' */
E 9
I 9
	progname += 1;				/* step past last '/' */
E 9

E 5
D 4
    if( argc == 2 )
	delay = atoi( argv[ 1 ] );
E 4
I 4
/* Interpret the arguments */
E 4

I 6
D 9
    sf = slidefile;	/* set source file to default */
E 9
I 9
    homedir = getenv( "HOME" );
    if( homedir == NULL )
        Fatal( "$HOME is undefined" );
    else
    {
        strcpy( deffile, homedir );
        strcat( deffile, SLIDEFILE );
    }
E 9

E 6
D 4
    PreRelease( "Slideshow 1.00" );
E 4
I 4
D 9
    switch (argc)
E 9
I 9
    for( opt = getopt( argc, argv, optargs );
         opt != EOF;
         opt = getopt( argc, argv, optargs ) )
E 9
    {
D 9
	case 1 :		/* If none, resource file = slidefile */
D 6
	    sf = slidefile;
E 6
	    break;
        
	case 2 :		/* If one, it must be a resource file name */
D 5
	    sf = argv[1];
E 5
I 5
D 6
	    sf = argv[ 1 ];
E 6
I 6
	    if (strcmp (argv[ 1 ], "-d") == 0)
	       goto badargs;
	    else		/* If -d found we have bad arguments */
	       sf = argv[ 1 ];
E 6
E 5
	    break;
E 9
I 9
        switch( opt )
        {
            case 'd':   sscanf( optarg, "%d", &delay ); break;
            default:    Fatal( "Usage: %s [-d <delay>] [<fn>]\n", progname );
        }
    }
    
/* Process all resource files on the command line */
E 9
E 4

D 4
    resfile = ResOpen( "/usr/lib/resources/slideshow.res", "r" );
E 4
I 4
D 9
	case 4 :		/* If four, it is an optional delay + filename*/
D 5
	    sf = argv[3];
E 5
I 5
	    sf = argv[ 3 ];
E 5
	case 3 :		/* If three, it is optional delay only */
D 5
	    if (strcmp (argv[1], "-d") == 0)
E 5
I 5
	    if (strcmp (argv[ 1 ], "-d") == 0)
E 5
	       delay = atoi( argv[ 2 ] );
	    else		/* If -d not found we have bad arguments */
		goto badargs;
	    break;

        default:		/* Anything else is wrong. */
badargs:
D 5
	    Error ("%s <-d delay> <fn>\n", argv[0]);
E 5
I 5
	    Fatal( "usage: %s [-d <delay>] [<fn>]\n", progname );
E 5
	    break;
E 9
I 9
    if( optind == argc )
        Process( deffile );
    else
    {
        for( i = optind; i < argc; i++ )
            Process( argv[ i ] );
E 9
    }

I 9
    exit( 0 );
}
/*                                                                *************
                                                                  *           *
                                                                  * PROCESS   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Process one resource file                                                  |
-------------------------------------------------------------------------------
*/

Process( sf )
char *sf;               /* Current source file */
{        
    objid object;

    curtitle = BaseName( sf );
    
E 9
D 6
/* Try opening the slideshow file for writing. If that fails, try read-only */
E 6
I 6
/* Try opening the resource file for writing. */
E 6

I 5
    resfile = ResOpen( sf, "w" );
E 5
E 4
    if( resfile == ERROR )
I 4
    {
D 5
	readonly = TRUE;
	resfile = ResOpen( rs, "r" );
E 5
I 5
D 6
	readonlt = TRUE;
	resfile = ResOpen( sf, "r" );
E 6
I 6
/* If we don't have write permission, try and read it */

D 8
        if (MMIERRTYPE(rerrno) == RESSYSERR && MMIERRNO(rerrno) == EACCES)
E 8
I 8
        if ( MMIERRTYPE(rerrno) == RESSYSERR && 
	     (MMIERRNO(rerrno) == EACCES) || (MMIERRNO(rerrno) == EROFS) )
E 8
	{
	    readonly = TRUE;
	    resfile = ResOpen( sf, "r" );
	}

/* If file does not exist, try and create it */

        else if (MMIERRTYPE(rerrno) == RESSYSERR && MMIERRNO(rerrno) == ENOENT)
	    resfile = ResCreate (sf, "A Resource File");
E 6
E 5
    }
    if( resfile == ERROR )
E 4
D 5
	Error( "(demo forms) %s", ResError() );
E 5
I 5
	Fatal( "file '%s': %s", sf, ResError() );
E 5

I 4
D 5
/* Determine the size of the window from the first form in it */
E 5
I 5
/* Read the default font handle number */
E 5

E 4
D 5
    object = ResFirst( resfile, RESFORM );
    ResRead( resfile, object, &formptr );
D 4
    winrect.width = formptr->width;
E 4
I 4
    winrect.width = formptr->width * formptr->planes / SCREENPLANES;
E 4
    winrect.height = formptr->height;
    free( formptr );
E 5
I 5
    deffont = FontHan ();
E 5

I 5
/* Determine the size of the window from the first object in it */

D 6
    object = ResFirst( resfile, RESANY );
    ResRead( resfile, object, &objptr );
    CalcWSize (&winrect, object, ResObjType (object));
    free( objptr );
E 6
I 6
    objptr = NULL;		/* This will make manager allocate space */
    if ((object = ResFirst( resfile, RESANY )) != ERROR)
    {
        ResRead( resfile, object, &objptr );
        CalcWSize (&winrect, objptr, ResObjType (object));
        free( objptr );
        objptr = NULL;
    }
/* If there were no objects in the file, thats ok, otherwise abort */
I 9

E 9
    else if ( !(MMIERRTYPE(rerrno) == RESERR && MMIERRNO(rerrno) == ERESNOOBJ))
    {
	Fatal ("file '%s': %s", sf, ResError ());
    }
E 6

E 5
/* Disable events until we have finished initialising: */

    DisEvents();
I 5

E 5
D 6
    OpenWindow();
I 4
    InitMenu();
D 5
    ShowPic( 0 );
E 5
I 5
    ShowObj( 0 );
E 5
E 4
    EnEvents();
E 6
I 6
    OpenWindow();	/* Open the window */
    InitMenu();		/* Initialise the menus */
    ShowObj( 0 );	/* Display the first object in the file */
    EnEvents();		/* Can re-enable events now */
E 6

D 4
/* Loop around, reading and displaying all the forms in the resource file: */
E 4
I 4
/* Wait for events: */
E 4

D 5
    do
E 5
I 5
    while( !finished )
E 5
    {
D 4
	for( object = ResFirst( resfile, RESFORM );
	     object != ERROR && !finished;
	     object = ResNext( resfile, RESFORM ) )
	{
	    formptr = NULL;
	    ResRead( resfile, object, &formptr );
	    selrect.width = formptr->width;
	    selrect.height = formptr->height;
	    screenhan = OpForm( formptr );
	    if( screenhan == ERROR )
		Error( "(form) %s", WError() );
	    free( formptr );
	    AdjustWindow();
	    PenSize( winrect.width * XSCALE, winrect.height * YSCALE );
	    PlotClip();

	    t = delay;
	    do
		t = sleep( t );
	    while( t > 0 && !nextpiccie && !finished );
	    ClForm( screenhan );
	    nextpiccie = FALSE;
	}
E 4
I 4
	pause();
I 6
	if (getnewname)
	    Rename ();
E 6
	if( doingdemo )
	    RollingDemo();
E 4
D 5
    } while( !finished );
E 5
I 5
    }
E 5
I 4

I 6
/*** At this point we are exiting ***/

E 6
/* Deallocate menus */

    TidyMenus();

D 6
/* Close the slideshow resource file */
E 6
I 6
/* Close the resource file */
E 6

E 4
    ResClose( resfile );
D 4
    
    PPOn();
    KBOff();
E 4
I 4

I 6
/* Close the window */

E 6
    WClose( fd );

/* Remove dead space from the resource file */

    if( needscomp ) 
D 6
	ResCompress( slidefile, RESTOTAL );
E 6
I 6
	ResCompress( sf, RESTOTAL );

/* and terminate */

E 6
E 4
    exit( 0 );
}
I 9
/*                                                                *************
                                                                  *           *
                                                                  * BASENAME  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Return base portion of pathname                                           |
-------------------------------------------------------------------------------
*/
E 9

I 9
char *BaseName( pathname )
char *pathname;
{
    char *result;
    
    result = strrchr( pathname, '/' );
    if( result == NULL )
        result = pathname;
    else
        result += 1;
        
    return( result );
}
    
E 9
/*                                                                *************
                                                                  *           *
                                                                  * OPENWINDOW*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 9
|   Open the clipboard window.                                                |
E 9
I 9
D 11
|   Open the restool window.                                                  |
E 11
I 11
|   Open the resshow window.                                                  |
E 11
E 9
I 6
| Note that the size must already have been calculated by calling CalcWSize.  |
E 6
-------------------------------------------------------------------------------
*/

OpenWindow()
{
D 6
    selrect = winrect;
E 6
I 6
/* Set the contents of objrect to be equal to those of winrect */
E 6

I 6
    objrect = winrect;

E 6
/* Window size must be between min & max size limits: */

    winrect.width = MIN( winrect.width, MAXWINWIDTH );
    winrect.width = MAX( winrect.width, MINWINWIDTH );
    winrect.height = MIN( winrect.height, MAXWINHEIGHT );
    winrect.height = MAX( winrect.height, MINWINHEIGHT );

					/* (Only relevent for WOTHISUNIQUE) */
    window = WOpen( 0, 			/* Minor device number of window */
		    WOANYUNIQUE, 
		    &winrect, 
D 2
		    NEOPOLITAN | CLOSEBOX | GROWBUT  |
		    VSCRLBAR | HSCRLBAR,
E 2
I 2
D 4
		    STRAWBERRY,
E 4
I 4
		    NEOPOLITAN | CLOSEBOX | GROWBUT  |
		    VSCRLBAR | HSCRLBAR,
E 4
E 2
D 5
		    NULL,			/* Null zoom point */
E 5
I 5
		    NULL,			/* use default zoom point */
E 5
		    eventactions,
D 4
		    WAEDRAG | WAEGROW );	/* Allow dragging */
E 4
I 4
		    WAEDRAG | WAEGROW );	/* Allow dragging, & growing */
E 4

I 6
/* Did we fail? */

E 6
    if( window == ERROR )
D 5
	Error( "(Window) %s", WError() );
E 5
I 5
D 6
	Error( WError() );
E 6
I 6
	Fatal( WError() );
E 6
E 5

I 4
/* remove text cursor */

E 4
    HideCursor();

D 4
/* Turn off post-processing */
E 4
I 4
/* Turn off post-processing, and block the keyboard */
E 4

    PPOff();
    KBOn();

I 5
/* clear the window to remove any keyboard characters echoed before KBOn() */

D 6
    ClearWindow();
E 6
I 6
    ClearVis();
E 6

E 5
D 4
/* Set window title and remove caret */
E 4
I 4
/* Set window title */
E 4

D 5
    TitleBits( CLOSEBOX | SHOWNAME );
E 5
I 5
    TitleBits( SHOWNAME );
E 5
D 6
    SetTitle( "Slideshow" );
E 6
I 6
D 9
    SetTitle( "Restool" );
E 9
I 9
    SetTitle( curtitle );
E 9
E 6
    ShowTitle();
I 6

/* Flush the ClearVis */

E 6
    Flush();

D 4
/* Set up the pad: */
E 4
I 4
D 6
/* Set up the Neopolitan window pad to size of form: */
E 6
I 6
/* Set up the Neopolitan window pad to size of object: */
E 6
E 4

    padinfo.pad.x = 0;
    padinfo.pad.y = 0;
D 6
    padinfo.pad.width = selrect.width;
    padinfo.pad.height = selrect.height;
E 6
I 6
    padinfo.pad.width = objrect.width;
    padinfo.pad.height = objrect.height;
E 6

I 4
/* Set the visible part to be equal to the window rectangle */

E 4
    padinfo.window.x = 0;
    padinfo.window.y = 0;
    padinfo.window.width = winrect.width;
    padinfo.window.height = winrect.height;

I 4
/* Set the step size of the scroll buttons */

E 4
    padinfo.horizstep = ( padinfo.pad.width - padinfo.window.width ) / STEPS;
    padinfo.vertstep = ( padinfo.pad.height - padinfo.window.height ) / STEPS;

D 4
/* Update the display: */
E 4
I 4
/* Update the pad */
E 4

    PutPad( &padinfo );
I 4

/* Update the display: */

E 4
    DrawScroll();
}

/*                                                                *************
                                                                  *           *
I 4
                                                                  *ROLLINGDEMO*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|   Display each form in a resource file in turn.                             |
E 6
I 6
|   Display each object in a resource file in turn.                           |
E 6
-------------------------------------------------------------------------------
*/

RollingDemo()
{
    int i, t;

D 6
/* Repeat going through all the pictures until the close box is clicked */
E 6
I 6
/* Repeat going through all the objects until the close box is clicked */
E 6
/* (finished == TRUE) or user stopped demo via the menu (doingdemo == FALSE) */

    do
    {
	for( i = 0; i < numobj; i++ )
	{
	    if( objects[ i ] == 0 )
D 6
		continue;
E 6
I 6
		continue;		/* Object has been deleted */
E 6
		
D 5
	    ShowPic( i );
E 5
I 5
/* disable events while we are redrawing the picture */

	    DisEvents();
	    ShowObj( i );
	    EnEvents();

E 5
	    t = delay;

/* Loop round until we've slept the required amount, OR clicked the mouse */
/* indicating move to next OR clicked the closebox OR terminated rolling  */
/* demo using the pull-down menu option */

	    do
		t = sleep( t );
	    while( t > 0 && !nextpiccie && !finished && doingdemo );

/* If the close box has been clicked, or the rolling demo unselected, stop */

	    if( finished || !doingdemo )
		break;

D 6
/* Free the form handle number currently in use */

E 6
D 5
	    ClForm( screenhan );
E 5
	    nextpiccie = FALSE;
	}
    } while( !finished && doingdemo );
}

/*                                                                *************
                                                                  *           *
D 5
                                                                  *  SHOWPIC  *
E 5
I 5
                                                                  *  SHOWOBJ  *
E 5
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 5
|  Show a particular picture                                                  |
E 5
I 5
|  Show a particular object if we can.                                        |
E 5
-------------------------------------------------------------------------------
*/

D 5
ShowPic( which )
E 5
I 5
ShowObj( which )
E 5
int which;
{
D 5
    form *formptr;		/* -> the object's data */
E 5
I 5
D 6
    char *objptr;		/* -> the object's data */
E 6
E 5
    objid object;		/* Object id to display */

D 6
/* Only do this if we select a new option */
E 6
I 6
/* Only do this if we select a new option and there is something to display */
E 6

D 6
    if( which == curobj )
E 6
I 6
    if( which == curobj || emptyfile)
E 6
	return;

I 5
D 6
    ClObject (curobj);
E 6
I 6
/* Free space both in kernel and in process occupied by last object */
E 6

I 6
    ClObject (curobj);	/* will set objptr to be null */

/* Get the resource manager object identifier */

E 6
E 5
    object = objects[ which ];

/* Read the resource's data into memory (auto allocate) */

D 5
    formptr = NULL;	/* Will force Resource Manager to allocate space */
E 5
I 5
D 6
    objptr = NULL;
E 5

E 6
D 5
    if( ResRead( resfile, object, &formptr ) == ERROR )
	Error( "(ResRead) %s", ResError() );
E 5
I 5
    if( ResRead( resfile, object, &objptr ) == ERROR )
	Fatal( ResError() );
E 5

D 5
/* Open the form */
E 5
I 5
D 6
/* Set the width & height of the pad */
E 6
I 6
/* Untick the previously selected form and tick the new one. */
E 6
E 5

D 5
    screenhan = OpForm( formptr );
    if( screenhan == ERROR )
	Error( "(form) %s", WError() );
E 5
I 5
D 6
    CalcWSize (&selrect, objptr, ResObjType (object));
E 6
I 6
    DisTick( objhan, curobj );
    curobj = which;		/* Change the current object to new one */
    EnableTick( objhan, which );
E 6
E 5

D 5
/* Set the width & height of the pad */
E 5
I 5
D 6
    Switch (ResObjType (object))
E 6
I 6
/* Set up some object type specific stuff */

    switch (ResObjType (object))
E 6
    {
I 6
/* Object is a mouse cursor, this has to be drawn in two stages */

E 6
	case RESCURSOR:
            s_han1 = OpForm( &((cursor*) objptr)->data );
            if( s_han1 == ERROR )
	        Error( WError() );
E 5

D 5
    selrect.width = formptr->width * formptr->planes / SCREENPLANES;
    selrect.height = formptr->height;
E 5
I 5
            s_han2 = OpForm( &((cursor*) objptr)->mask );
            if( s_han2 == ERROR )
	        Error( WError() );
D 6
/* Set the pen size and plot the form in the window */
E 5

D 5
/* Deallocate user address space form data (form now on heap) */
E 5
I 5
            PenSize( selrect.width * XSCALE, selrect.height * YSCALE );
E 6
	    break;
E 5

I 6
/* Object is a font, so open it for use later */

E 6
D 5
    free( formptr );
E 5
I 5
	case RESFONT:
	    s_han1 = OpFont ( objptr );
            if( s_han1 == ERROR )
	        Error( WError() );
	    break;
E 5

I 6
/* Object is a halftone, so open it for use later */

E 6
D 5
/* Adjust the window to fit the new form */
E 5
I 5
	case RESHTONE:
	    s_han1 = OpPat ( objptr );
            if( s_han1 == ERROR )
	        Error( WError() );
	    break;
E 5

I 6
/* Object is an icon, so open it for use later */

E 6
D 5
    AdjustWindow();
E 5
I 5
	case RESICON:
D 6
            s_han1 = OpForm( &((icon*) objptr)->data );
E 6
I 6
            s_han1 = OpIcon( objptr );
E 6
            if( s_han1 == ERROR )
	        Error( WError() );
E 5
D 6

I 5
            s_han2 = OpForm( &((icon*) objptr)->mask );
            if( s_han2 == ERROR )
	        Error( WError() );
E 5
/* Set the pen size and plot the form in the window */

D 5
    PenSize( selrect.width * XSCALE, selrect.height * YSCALE );
    PlotClip();
E 5
I 5
            PenSize( selrect.width * XSCALE, selrect.height * YSCALE );
E 6
	    break;
E 5

I 5
D 6
	case RESFORM:
/* Open the form */
E 6
I 6
/* Object is a form, so open it for use later */
E 6

I 6
	case RESFORM:
E 6
            s_han1 = OpForm( objptr );
            if( s_han1 == ERROR )
	        Error( WError() );
D 6
/* Set the pen size and plot the form in the window */

            PenSize( selrect.width * XSCALE, selrect.height * YSCALE );
E 6
            break;

I 6
/* Object cannot be displayed, don't do anything */

E 6
	default:
	    break;
    }

D 6
/* Deallocate user address space object data (object now on heap) */
E 6
I 6
/* Set the size of the bounding rectangle of the object */
E 6

D 6
    free( objptr );
E 6
I 6
    CalcWSize (&objrect, objptr, ResObjType (object));
E 6

D 6
/* Adjust the window to fit the new object */
E 6
I 6
/* Set the pen size (in locals) in case needed (must be done after CalcWSize) */
E 6

D 6
    AdjustWindow();
E 6
I 6
    PenSize( objrect.width * XSCALE, objrect.height * YSCALE );
E 6

D 6
    DrawClip();
E 6
I 6
/* Adjust the window to fit the new object. This will redraw it too. */
E 6

E 5
D 6
/* Untick the previously selected form and tick the new one. */
E 6
I 6
    AdjustWindow();
E 6

D 6
    DisTick( pichan, curobj );
    curobj = which;
    EnableTick( pichan, which );
E 6
}

/*                                                                *************
                                                                  *           *
I 5
                                                                  *  CLOBJECT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
| Close down any necessary handle numbers for this object.                    |
E 6
I 6
| Close down any necessary handle numbers for this object, and free the space |
| that it takes up.                                                           |
E 6
-------------------------------------------------------------------------------
*/

ClObject (object)
int object;

{
   objid obj = objects [object];

D 6
   switch (ResObjType (obj)
E 6
I 6
/* Deallocate user address space object data if it is allocated */

D 7
    if (objptr)
E 7
I 7
    if (objptr != NULL)
E 7
    {
       free( objptr );
       objptr = 0;	/* Indicate that nothing is now allocated */
    }

/* If s_han1 has the value ERROR, then there are no objects on the heap */

   if (s_han1 == ERROR)
      return;

/* Objects on the heap have a type, so we must use different calls for
   different objects.
   */

   switch (ResObjType (obj))
E 6
   {
D 6
	case RESICON:
	case RESCURSOR:
	    ClForm (s_han2);
	case RESFORM:
E 6
I 6
	case RESCURSOR:		/* Object is a mouse pointer */
	    if (s_han2 != ERROR)
	        ClForm (s_han2);
	case RESFORM:		/* Object is a form */
E 6
	    ClForm (s_han1);
	    break;

D 6
	case RESFONT:
E 6
I 6
	case RESICON:		/* Object is an icon */
	    ClIcon (s_han1);
	    break;

	case RESFONT:		/* Object is a font */
E 6
	    ClFont (s_han1);
	    break;

D 6
	case RESHTONE:
E 6
I 6
	case RESHTONE:		/* Object is a halftone */
E 6
	    ClPat (s_han1);
	    break;

D 6
	default:
E 6
I 6
	default:		/* Object cannot be in kernel */
E 6
	    break;
    }
I 6

/* Indicate that there are no objects currently open */

    s_han1 = s_han2 = ERROR;
E 6
}

/*                                                                *************
                                                                  *           *
E 5
                                                                  * INITMENUS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|  Create the cut & paste menu.                                               |
E 6
I 6
|  Create the all the menus.                                                  |
E 6
-------------------------------------------------------------------------------
*/

InitMenus()
{
D 6
    char name[ 80 ];
E 6
I 6
    char name[ 80 ];	/* A temporary buffer */
E 6
    objid obj;
    extern char *Type();
D 9

E 9
I 9
    int type;           /* Type of this resource */
    
E 9
D 6
/* The edit menu */
E 6
I 6
/*** Download all the menus before drawing the menu bar ***/
E 6

D 6
    edithan = MakeMenu( &editmenu );
E 6
I 6
/* The object menu */
/*
 * Note that because there are a variable number of objects, the contents
 * of this menu can vary a lot too. Normally we would use AppItem and
 * AlterItem to do this, but to be compatible with kernel versions of less
 * than 1.3, we cannot do this. Therefore each time we append an item or alter
 * it, we must down load a new menu. Note that this does not apply to ticking
 * and disabling menu items.
 */

/* Go through every object in the resource file */

    for( obj = ResFirst( resfile, RESANY ), numobj = 0;
	 obj != ERROR && numobj < MAX_ENTRIES;
	 obj = ResNext( resfile, RESANY ),  numobj++ )
    {
I 9
        type = ResObjType( obj );

E 9
	emptyfile = FALSE;		/* File isn't empty */
	objects[ numobj ] = obj;

/* Make the string for the menu item */

	sprintf( name, "%s%s%s", ResObjName( obj ), JUSTSTRING,
D 9
		 Type( ResObjType( obj ) ) );
E 9
I 9
		 Type( type ) );
E 9
	objmenu.items[numobj] = malloc (strlen (name) + 1);
	strcpy (objmenu.items[numobj], name);

/* If object is a deleted one, disable it. */

	if (ResObjType (obj) == RESNULL)
D 9
	   objmenu.enableflags &= ~(1 << numobj);
E 9
I 9
	   objmenu.enableflags &= ~(1 << numobj); 
E 9
    }

/*
 * Note that there are items on this menu that are right-justified. This
 * would normally be obtained by embedding the value RJUST in the item
 * string just before the right-justified part. However to maintain
 * compatibility with kernel versions < 1.3 we must embed the two values
 * embodied in JUSTSTRING. However (!) on the LAST item this is not enough.
 * On this item we must first turn off cursor wrapping using the SETEDBITS
 * escape sequence. Then, after the last character in the item, we must turn
 * it back on again. If you are curious to see the effects of not doing
 * these two things, try it. NOTE that in release 1.3 or later of the kernel
 * none of this is necessary.
 */

    SetFudge (numobj-1);
    
    objhan = MakeMenu( &objmenu );	/* Can now down-load the object menu */
    if( objhan == ERROR )
         Error( "obj menu : %s", WError() );
    if( InsMenu( objhan, 0 ) < 0 )	/* and insert it into the bar */
         Error( "obj menu : %s", WError() );
   
/*** The edit menu ***/

    edithan = MakeMenu( &editmenu );	/* Down load it */
E 6
    if( edithan == ERROR )
D 5
	Error( "Failed to download menu" );
E 5
I 5
	Error( "edit menu : %s", WError() );
E 5
D 6
    if( InsMenu( edithan, 0 ) < 0 )
E 6
I 6
    if( InsMenu( edithan, OBJMENU ) < 0 )	/* Insert into bar */
E 6
D 5
	Error( "Failed to Insert menu" );
E 5
I 5
	Error( "edit menu : %s", WError() );
E 5

D 6
/* If we only have the file open read-only, disable cut + paste */
E 6
I 6
/* If there are no objects in the file, disable cut and copy */
E 6

D 6
    if( readonly )
E 6
I 6
    if (emptyfile)
E 6
    {
	DisItem( edithan, CUTITEM );
D 6
	DisItem( edithan, PASTEITEM );
E 6
I 6
	DisItem( edithan, COPYITEM );
D 8
	/*
E 8
	DisItem( edithan, RENAMEITEM );
D 8
	*/
E 8
E 6
    }

D 6
/* The picture menu */
E 6
I 6
/* If we only have the file open read-only, disable cut + paste */
E 6

D 6
    pichan = MakeMenu( &picmenu );
    if( pichan == ERROR )
D 5
	Error( "Failed to download menu" );
E 5
I 5
	Error( "pic menu : %s", WError() );
E 5
    if( InsMenu( pichan, 0 ) < 0 )
D 5
	Error( "Failed to Insert menu" );
E 5
I 5
	Error( "pic menu : %s", WError() );
E 5
   
D 5
    for( obj = ResFirst( resfile, RESFORM ), numobj = 0;
E 5
I 5
    for( obj = ResFirst( resfile, RESANY ), numobj = 0;
E 5
	 obj != ERROR;
D 5
	 obj = ResNext( resfile, RESFORM ),  numobj++ )
E 5
I 5
	 obj = ResNext( resfile, RESANY ),  numobj++ )
E 6
I 6
    if( readonly )
E 6
E 5
    {
D 6
	emptyfile = FALSE;
	objects[ numobj ] = obj;
	sprintf( name, "%s%c%s", ResObjName( obj ), RJUST,
		 Type( ResObjType( obj ) ) );
	AppItem( pichan, name );
E 6
I 6
	DisItem( edithan, CUTITEM );
	DisItem( edithan, PASTEITEM );
D 8
	/*
E 8
	DisItem( edithan, RENAMEITEM );
D 8
	*/
E 8
E 6
    }

D 6
/* The demo menu */
E 6
I 6
/*** The demo menu ***/
E 6

    demohan = MakeMenu( &demomenu );
    if( demohan == ERROR )
D 5
	Error( "Failed to download menu" );
E 5
I 5
	Error( "demo menu : %s", WError() );
E 5
    if( InsMenu( demohan, 0 ) < 0 )
D 5
	Error( "Failed to Insert menu" );
E 5
I 5
	Error( "demo menu : %s", WError() );
E 5
   
D 6
/* Draw the menu bar */
E 6
I 6
/* Now that everything is down-loaded and inserted we can draw the menu bar */
E 6

    DrawMBar();
}

/*                                                                *************
                                                                  *           *
                                                                  * TYPE      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|  Return a mnemonic for a resource type.                                     |
E 6
I 6
|  Return a mnemonic for a resource type. These strings are put in the menu   |
| item corresponding to the object and are right-justified.                   |
E 6
-------------------------------------------------------------------------------
*/

char *Type( x )
int x;
{
    char *result;
    switch( x )
    {
	case  RESNULL:   result = "DELETED"; break;
	case  RESFORM:   result = "FORM"; break;
	case  RESICON:   result = "ICON"; break;
	case  RESFONT:   result = "FONT"; break;
	case  RESCURSOR: result = "CURSOR"; break;
	case  RESHTONE:  result = "HTONE"; break;
	case  RESPALETTE:result = "PALETTE"; break;
	case  RESCONFIG: result = "CONFIG"; break;
	case  XXXMAGIC:  result = "*HEADER*"; break;
	case  RESMENU:   result = "MENU"; break;
	case  RESTEXT:   result = "TEXT"; break;
	case  RESPOINT:  result = "POINT"; break;
	case  RESRECT:   result = "RECT"; break;
	case  RESINT:    result = "INT-VEC"; break;
	case  RESDOUBLE: result = "DUB-VEC"; break;
	case  RESSTRING: result = "STRING"; break;
	case  RESWINPOS: result = "WINPOS"; break;
	case  RESPADPOS: result = "PADPOS"; break;
	case  RESPICTURE:result = "PICTURE"; break;
	case  RESPRESTEL:result = "PRESTEL"; break;
	case  RESFAXT3:	 result = "FAX-T3"; break;
	case  RESFAXT4:  result = "FAX-T4"; break;
	case  RESNAPLPS: result = "NAPLPS"; break;
	case  RESCEPT:	 result = "CEPT3"; break;
	case  RESNEWPAL: result = "NEW-PAL"; break;
	case  RESFILENAME:result = "FILENAME"; break;
	case  RESENVIRONMENT:result = "ENVIRON"; break;
	case  RESFIRSTWORD:result = "1STWORD"; break;
	case  RESSOUND:    result = "SOUND"; break;
	case  RESKBDMATRIX:result = "KBD-MATRIX"; break;
	case  RESTERMIO:   result = "TTY-SETTINGS"; break;
	case  RESDIALOGUE: result = "DIALOGUE"; break;

	default:        result = "UNKNOWN"; break;
    }
    return( result );
}

/*                                                                *************
                                                                  *           *
                                                                  * TIDYMENUS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Deallocate the menus                                                       |
-------------------------------------------------------------------------------
*/

TidyMenus()
{
I 6
/* Have to first delete the menu from the bar and then close it down */

E 6
    if( DelMenu( edithan ) == ERROR )
D 5
	Error( "DelMenu edithan" ); 
E 5
I 5
	Error( "edit menu : %s", WError() );
E 5
    if( DispMenu( edithan ) == ERROR )
D 5
	Error( "DispMenu" ); 
E 5
I 5
	Error( "edit menu : %s", WError() );
E 5

    if( DelMenu( demohan ) == ERROR )
D 5
	Error( "DelMenu demohan" ); 
E 5
I 5
	Error( "demo menu : %s", WError() );
E 5
    if( DispMenu( demohan ) == ERROR )
D 5
	Error( "DispMenu" ); 
E 5
I 5
	Error( "demo menu : %s", WError() );
E 5

D 6
    if( DelMenu( pichan ) == ERROR )
D 5
	Error( "DelMenu pichan" ); 
E 5
I 5
	Error( "pic menu : %s", WError() );
E 5
    if( DispMenu( pichan ) == ERROR )
D 5
	Error( "DispMenu" ); 
E 5
I 5
	Error( "pic menu : %s", WError() );
E 6
I 6
    if( DelMenu( objhan ) == ERROR )
	Error( "obj menu : %s", WError() );
    if( DispMenu( objhan ) == ERROR )
	Error( "obj menu : %s", WError() );
E 6
E 5
}

/*                                                                *************
                                                                  *           *
E 4
                                                                  * ADJUSTWIND*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|   Adjust the size of the clipboard window to fit the selection.             |
E 6
I 6
D 11
|   Adjust the size of the restool window to fit the selection.               |
E 11
I 11
|   Adjust the size of the resshow window to fit the selection.               |
E 11
E 6
-------------------------------------------------------------------------------
*/

AdjustWindow()
{
    rectangle winrect;		/* The visarea */
    rectangle framerect;	/* The frame area */
    ulimstate winlimits;	/* The window limits (max only used) */
    int newwidth, newheight;

I 6
/* Read the windows vital statistics */

E 6
    RFrameArea( &framerect );	/* The window's frame area */
    RVisArea( &winrect );	/* The window's current size */
    RWinLimits( &winlimits );	/* The windows min & max size limits */

D 4
/* Set the new dimensions from the difference between the visarea and */
/* frame area (the title bar) and the selection rectangle */
E 4
I 4
D 6
/* Calculate the new dimensions of the framearea of the window */
E 6
I 6
/* Calculate the new dimensions of the FRAMEAREA of the window */
/* (be optimistic) */
E 6
E 4

D 6
    newwidth = selrect.width + framerect.width - winrect.width;
    newheight = selrect.height + framerect.height - winrect.height;
E 6
I 6
    newwidth = objrect.width + (framerect.width - winrect.width);
    newheight = objrect.height + (framerect.height - winrect.height);
E 6

D 6
/* But the new dimensions must be at least the minimum window size: */
E 6
I 6
/* Check whether the optimism was founded. The new dimensions must be
   at least the minimum window size: */
E 6

    newwidth = MAX( newwidth, winlimits.winwidmin );
    newheight = MAX( newheight, winlimits.winheimin );

/* ... and must be less than the screen size, so we can reach the controls: */

    newwidth = MIN( newwidth, MAXWINWIDTH );
    newheight = MIN( newheight, MAXWINHEIGHT );

I 4
D 6
/* Limit the maximum size of the framearea */
E 6
I 6
/* Write back the new maximum size of the framearea */
E 6

E 4
    winlimits.winwidmax = newwidth;
    winlimits.winheimax = newheight;

D 6
/* Set the new window maximum size: */

E 6
    WWinLimits( &winlimits );

/* Set the new pad value: */

    padinfo.pad.x = 0;
    padinfo.pad.y = 0;
D 6
    padinfo.pad.width = selrect.width;
    padinfo.pad.height = selrect.height;
E 6
I 6
    padinfo.pad.width = objrect.width;
    padinfo.pad.height = objrect.height;
E 6

I 6
/* and the visible part of that pad */

E 6
    padinfo.window.x = 0;
    padinfo.window.y = 0;
    padinfo.window.width = newwidth - (framerect.width - winrect.width);
    padinfo.window.height = newheight - (framerect.height - winrect.height);

I 6
/* Set the pad step size */

E 6
    padinfo.horizstep = (padinfo.pad.width - padinfo.window.width ) / STEPS;
    padinfo.vertstep = (padinfo.pad.height - padinfo.window.height ) / STEPS;

I 6
/* And update the pad */

E 6
    PutPad( &padinfo );

/* Change the size of the window: (ModWindow expects relative size changes) */
/* Note that this will cause an automatic window update event */

    if( newwidth - framerect.width != 0 ||
	newheight - framerect.height != 0 )
    {
	ModWindow( 0, 0, newwidth - framerect.width, 
			 newheight - framerect.height );
I 6

/* Set the edit area to be equal to the object bounding rectangle */

	framerect.x = 0;
	framerect.y = 0;
	framerect.width = padinfo.pad.width * XSCALE;
	framerect.height = padinfo.pad.height * YSCALE;

/* This should be in locals */

	WEditArea (&framerect);		/* Re-use framerect */
        Flush ();			/* (Because WEditArea is escape seq.) */
E 6
    }
D 6
    else
E 6
I 6
    else		/* Window size hasn't changed */
E 6
	ChangePad();
}

/*                                                                *************
                                                                  *           *
D 5
                                                                  * PLOTCLIP  *
E 5
I 5
                                                                  * CALCWSIZE *
E 5
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 5
|  Plot the selection rectangle in the clipboard window.                      |
E 5
I 5
|  Calculate the size of the window according to the object type.             |
E 5
-------------------------------------------------------------------------------
*/

D 5
PlotClip()
E 5
I 5
D 6
CalcWSize (rptr, objptr, type)
register rectangle *rptr;
register char *objptr;
register type;
E 6
I 6
CalcWSize (rptr, loc_obj, type)
register rectangle *rptr;	/* Rectangle for result */
register char *loc_obj;		/* Pointer to object data */
register type;			/* Type of object */
E 6

E 5
{
I 5
    switch (type)
    {
I 6
/* Object is a mouse pointer, these are fixed size */

E 6
	case RESCURSOR:
D 6
	    rptr->width = 16 * ((cursor*) objptr)->data.planes / SCREENPLANES;
E 6
I 6
	    rptr->width = 16;
E 6
	    rptr->height = 16;
	    break;

I 6
/* Object is a font. Allow space for example string to print */
/* Note that cannot SelFont cannot be called before window is open. */

E 6
	case RESFONT:
D 6
	    rptr->width = StringWidth (examplestring) + 8;
	    rptr->height = ((font*) objptr)->leading;
E 6
I 6
	    if (s_han1 != ERROR)	/* s_han1 MUST be set to the font */
	       SelFont (s_han1);
	    rptr->width = RdStrWidth (examplestring) +
			  ((font*) loc_obj)->maxwidth;
	    rptr->height = ((font*) loc_obj)->leading;
	    if (s_han1 != ERROR)
	       SelFont (deffont);
E 6
	    break;

I 6
/* Object is a halftone */

E 6
	case RESHTONE:
D 6
	    rptr->width = 256 / XSCALE;
E 6
I 6
	    rptr->width = 256 / XSCALE;	/* Allow a 'decent' sized window */
E 6
	    rptr->height = 256 / YSCALE;
	    break;

D 6
	case RESTEXT:
E 6
I 6
/* Object is text */

I 7
	case RESSTRING:
E 7
	case RESTEXT:			/* Display 40x20 characters */
E 6
	    rptr->width = 320;
	    rptr->height = 160;
	    break;

D 6
	case RESICON:
	    rptr->width = ((icon*) objptr)->mask.width *
			  ((icon*) objptr)->mask.planes / SCREENPLANES;
	    rptr->height = ((icon*) objptr)->mask.height;
E 6
I 6
/* Object is an icon */

	case RESICON:			/* Allow the space it will occupy */
	    rptr->width = ((icon*) loc_obj)->mask.width;
	    rptr->height = ((icon*) loc_obj)->mask.height;
E 6
	    break;

D 6
	case RESFORM:
            rptr->width = ((form*) objptr)->width *
		            ((form*) objptr)->planes / SCREENPLANES;
            rptr->height = ((form*) objptr)->height;
E 6
I 6
/* Object is a form */

	case RESFORM:			/* Allow the space it will occupy */
            rptr->width = ((form*) loc_obj)->width;
            rptr->height = ((form*) loc_obj)->height;
E 6
	    break;

D 6
	default:
	    rptr->width = StringWidth (cantcope) + 8;
E 6
I 6
/* Object can't be displayed */
/* Note that RdStrWidth could fail if window is not yet open. */

	default:			/* Allow space for message */
	    rptr->width = RdStrWidth (cantcope) + 8;
	    if (rptr->width == ERROR)
		rptr->width = 0;	/* Minimum window size will cope */
E 6
	    rptr->height = 20;
	    break;
    }
}

/*                                                                *************
                                                                  *           *
                                                                  * DRAWCLIP  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Draw the current object in the window if we can.                           |
-------------------------------------------------------------------------------
*/

DrawClip()
{
E 5
D 6
    rectangle visarea;
    int xoffset, yoffset;
E 6
I 6
    rectangle editrect;
    int xoffset, yoffset, i;
E 6
    
I 6
/* We may have scrolled through the window, so read the current position */

E 6
    GetPad( &padinfo );
D 6
    RVisArea( &visarea );
E 6

D 6
/* If the selected portion of the screen is smaller than the visarea, then */
/* we need to clear the window before displaying the selected area: */
E 6
I 6
/* this is in 'globals' */
E 6

D 6
    if( visarea.width > selrect.width || visarea.height > selrect.height )
    	ClearWindow(); 

E 6
    xoffset =   -padinfo.window.x;
    yoffset =   -padinfo.window.y;
D 5
    MoveTo( xoffset * XSCALE, yoffset * YSCALE );
    PenForm( screenhan );
E 5

I 6
/* Change the edit area to reflect the current offset. */

    REditArea (&editrect);
    RectGToL (&editrect);
    editrect.x = xoffset * XSCALE;
    editrect.y = yoffset * YSCALE;
    WEditArea (&editrect);

/* Reset the background pattern in case it should be */

    SetBPat (BITS_0);

E 6
D 5
    PlotPoint();
    PenForm( 0 ); 
E 5
I 5
    switch (ResObjType (objects [curobj]))
    {
I 6
/* Object is a mouse pointer. This must be written in to stages. */

E 6
	case RESCURSOR:
I 6
	    SetBPat (BITS_12_3);	/* Provide a background so see mask */
	    ClearVis ();		/* Clear window to this pattern */
E 6
            MoveTo( xoffset * XSCALE, yoffset * YSCALE );
D 6
	    PenMode (M_SANDD);
E 6
I 6
	    PenMode (M_SANDD);		/* mask is anded onto destination */
E 6
            PenForm( s_han2 );
            PlotPoint();
D 6
	    PenMode (M_SXORD);
E 6
I 6
	    PenMode (M_XOR);		/* data is xor'ed onto result */
E 6
            PenForm( s_han1 );
            PlotPoint();
D 6
	    PenMode (M_COPY);
            PenForm( 0 ); 
E 6
I 6
	    PenMode (M_COPY);		/* Set pen mode to default */
            PenForm( 0 ); 		/* set pen to no form */
E 6
	    break;

I 6
/* Object is a font. Print a string using it that contains all letters */

E 6
	case RESFONT:
D 6
	    HomeCursor ();
	    SelFont (s_han1);
	    fprintf (fs, examplestring);
	    SelFont (deffont);
E 6
I 6
	    SelFont (s_han1);		/* set the current font */
    	    ClearVis(); 		/* Clear window */
	    fprintf (fs, examplestring);/* Print the string */
	    SelFont (deffont);		/* make sure font is not in use */
E 6
	    break;

I 6
/* Object is a halftone, clear the window to it. */

E 6
	case RESHTONE:
	    SetBPat (s_han1);
D 6
	    ClearWindow ();
	    SetBPat (BITS_0);
E 6
I 6
	    ClearVis ();
E 6
	    break;

I 6
/* Object is text. Draw at most 799 characters of it (40x20 - 1). */

I 7
	case RESSTRING:
E 7
E 6
	case RESTEXT:
D 6
	    HomeCursor ();
	    for (i=0; i<ResObjSize (objects [curobj]) && i < 800; i++)
E 6
I 6
    	    ClearVis(); 
	    for (i=0; i<ResObjSize (objects [curobj]) && i < 799; i++)
E 6
D 7
	       fprintf (fs, "%c", *(objptr+i));
E 7
I 7
	       putc ( *(objptr+i), fs );
E 7
	    break;

I 6
/* Object is an icon. Draw it */

E 6
	case RESICON:
D 6
            MoveTo( xoffset * XSCALE, yoffset * YSCALE );
	    PenMode (M_NSANDD);
            PenForm( s_han2 );
            PlotPoint();
	    PenMode (M_SXORD);
            PenForm( s_han1 );
            PlotPoint();
	    PenMode (M_COPY);
            PenForm( 0 ); 
E 6
I 6
	    SetBPat (BITS_12_3);	/* provide a background */
	    ClearVis ();		/* clear window to it */
            DrawIcon ( s_han1, xoffset * XSCALE, yoffset * YSCALE );
E 6
	    break;

I 6
/* Object is a form. Plot it */

E 6
	case RESFORM:
I 6
    	    ClearVis(); 
E 6
            MoveTo( xoffset * XSCALE, yoffset * YSCALE );
            PenForm( s_han1 );
            PlotPoint();
D 6
            PenForm( 0 ); 
E 6
I 6
            PenForm( 0 ); 		/* set pen to no form */
E 6
	    break;

I 6
/* Can't display this object, print an apology */

E 6
	default:
I 6
    	    ClearVis(); 
	    fprintf (fs, cantcope);
E 6
	    break;
    }

I 6
/* Make sure the results of all this are seen */

E 6
E 5
    Flush();
}

D 4
/************** EVENT ROUTINES ********************************************/
E 4
I 4
/*                                                                *************
                                                                  *           *
                                                                  *  CUT      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|   Move the current object to the clipboard                                  |
E 6
I 6
|   Move the current object to the scrap.                                     |
E 6
-------------------------------------------------------------------------------
*/
E 4

I 4
Cut()
{
    objid obj;			/* Object identifier to free */
    char name[ 80 ];		/* Object name */
    register int i;			

I 6
/* Copy the object to the scrap, and then... */

E 6
    Copy();

D 6
/* Delete the object from the resource file and modify menu entry */
E 6
I 6
/* ...delete the object from the resource file and modify menu entry */
E 6

    needscomp = TRUE;
    obj = objects[ curobj ];
D 6
    objects[ curobj ] = 0;
    sprintf( name, "%s%c%s", ResObjName( obj ), RJUST, Type( RESNULL ) );
E 6
I 6
    objects[ curobj ] = 0;	/* Indicate object is deleted */
    sprintf( name, "%s%s%s", ResObjName( obj ), JUSTSTRING, Type( RESNULL ) );

/* We should set NOWRAP on the last item, refer to comments in InitMenu */

    if (curobj == numobj-1)
       SetFudge (curobj);

E 6
    if( ResFree( resfile, obj ) == ERROR )
D 5
	Error( "(ResFree) %s", ResError() );
E 5
I 5
	Error( ResError() );
E 5

D 6
    AlterItem( pichan, curobj, name );
    DisItem( pichan, curobj );
E 6
I 6
/* Note, can't use AlterItem in kernels < 1.3 because of bug */
E 6

I 6
    free (objmenu.items[curobj]);
    objmenu.items[curobj] = malloc (strlen (name) + 1);
    strcpy (objmenu.items[curobj], name);

/* Disable the item */

    objmenu.enableflags &= ~(1 << curobj);

/* Delete the old menu */

    DelMenu (objhan);

/* And dispose of it */

    if (DispMenu (objhan) == ERROR)
	Error ("DispMenu: %s", WError());
    objhan = MakeMenu (&objmenu);

/* Insert new menu */

    InsMenu (objhan, DEMOMENU);

/* Don't need to draw menu bar as its contents haven't changed!!! */

E 6
/* Change the display to show the first non-deleted object: */

    for( i = numobj - 1; i >= 0; i-- )
    {
	if( objects[ i ] > 0 )
	    break;
    }

/* If there are no valid objects left, disable cut & copy */

    if( i < 0 )
    {
	DisItem( edithan, CUTITEM );
	DisItem( edithan, COPYITEM );
I 6
D 8
	/*
E 8
	DisItem( edithan, RENAMEITEM );
D 8
	*/
E 8
E 6
	emptyfile = TRUE;
D 6
	ClearWindow();
E 6
I 6
	ClearVis();
E 6
	Flush();
    }
D 6
    else
D 5
	ShowPic( i );
E 5
I 5
	ShowObj( i );
E 6
I 6

    ShowObj( i );	/* Draw the new current object */
E 6
E 5
}

E 4
/*                                                                *************
                                                                  *           *
I 4
                                                                  *  COPY     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Copy the current object to the clipboard                                  |
-------------------------------------------------------------------------------
*/

Copy()
{
    char *resdata = NULL;	/* -> resource data */
    objid obj;			/* Object identifier */

I 6

/* Check that there is actually some data to copy */

   if (!objptr)
      return;

E 6
/* Find the currently selected object */

    obj = objects[ curobj ];

/* Open the scrap for writing */

    if( OpenScrap( "w" ) == ERROR )
D 5
	Error( "(OpenScrap) %s", ResError() );;
E 5
I 5
	Error( ResError() );
    else
    {
E 5

D 6
/* Read the object's data */
E 6
I 6
/* Write the object to the scrap, maintaining its name */
E 6

D 5
    if( ResRead( resfile, obj, &resdata ) == ERROR )
	Error( "(ResRead) %s", ResError() );;
E 5
I 5
D 6
	resdata = NULL;
	if( ResRead( resfile, obj, &resdata ) == ERROR )
	    Error( ResError() );
E 6
I 6
	if( WriteLScrap( ResObjType( obj ), ResObjName ( obj ),
			 ResObjSize( obj ), objptr ) == ERROR )
	    Error( "(WriteLScrap) %s", ResError() );
E 6
E 5

D 6
/* Write the object to the scrap */

D 5
    if( WriteScrap( ResObjType( obj ), ResObjSize( obj ), resdata ) == ERROR )
	Error( "(WriteScrap) %s", ResError() );
E 5
I 5
	if( WriteScrap( ResObjType( obj ), ResObjSize( obj ), resdata ) == ERROR )
	    Error( "(WriteScrap) %s", ResError() );
E 5

E 6
/* Deallocate temporary store and close the resource file */

D 5
    free( resdata );
    CloseScrap();
E 5
I 5
D 6
	free( resdata );
E 6
	CloseScrap();
    }
E 5
}

/*                                                                *************
                                                                  *           *
                                                                  *  PASTE    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 6
|   Paste the contents of the scrap onto the slideshow.                       |
E 6
I 6
|   Paste the contents of the scrap into the resource file.                   |
E 6
-------------------------------------------------------------------------------
*/

Paste()
{
    resid scrapfile;		/* Scrap resource file identifier */
    objid scrapobj;		/* The scrap object identifier */
    char *resdata = NULL;	/* -> resource data */
    int type;			/* The type of the current resource */
    objid newobj;		/* Object added from scrap */
    char name[ 80 ];		/* Object name */

D 6
/* If the file was empty, enable the copy item, and also the cut item if */
E 6
I 6
/* If the file WAS empty, enable the copy item, and also the cut item if */
E 6
/* we have the file open for read/write */

    if( emptyfile )
    {
	EnableItem ( edithan, COPYITEM );
	if( !readonly )
I 6
	{
E 6
	    EnableItem( edithan, CUTITEM );
I 6
D 8
	    /*
E 8
	    EnableItem( edithan, RENAMEITEM );
D 8
	    */
E 8
	}
E 6
	emptyfile = FALSE;
    }

/* Open the scrap */

    scrapfile = OpenScrap( "r" );
    if( scrapfile == ERROR )
D 5
	Error( "(OpenScrap) %s", ResError() );
E 5
I 5
	Error( ResError() );
    else
    {
E 5
D 7

E 7
D 5
/* Find the first object in it */
E 5
I 5
D 6
/* Find the first form in it */
E 6
I 6
/* Find the first object in it */
E 6
E 5

D 5
    scrapobj = ResFirst( scrapfile, RESANY );
    if( scrapobj == ERROR )
	Error( "(No object in scrap) %s", ResError() );
E 5
I 5
	scrapobj = ResFirst( scrapfile, RESANY );
	if( scrapobj == ERROR )
	    Error( "searching for object in deskscrap : %s", ResError() );
E 5

D 5
/* Find what type it is */
E 5
I 5
/* Try and read the objects data in */
E 5

D 5
    type = ResObjType( scrapobj );
E 5
I 5
	if( ReadScrap( RESANY, &resdata ) == ERROR )
	    Error( "(ReadScrap) %s", ResError() );
E 5

D 5
/* Read the object's data in */
E 5
I 5
D 6
/* Add the form to the slideshow file */
E 6
I 6
/* Add the object to the resource file */
E 6
E 5

D 5
    if( ReadScrap( type, &resdata ) == ERROR )
	Error( "(ReadScrap) %s", ResError() );;
E 5
I 5
	newobj = ResAdd( resfile, ResObjType (scrapobj),
				  ResObjName (scrapobj), 
			          ResObjSize (scrapobj), resdata );
	if( newobj == ERROR )
	    Error( "(ResAdd newobj) %s", ResError() );
	else
	{
I 7
	    free( resdata );

E 7
D 6
	    sprintf( name, "%s%c%s", ResObjName (scrapobj), RJUST,
		     Type (ResObjType (scrapobj)) );
	    AppItem( pichan, name );
E 6
I 6
/* Must set NOWRAP on last item (and ONLY the last item) because of
   kernel bug (see comments in InitMenu) */

	    RemFudge (numobj-1);	/* Remove it from previous last item */

	    sprintf( name, "%s%s%s%s%s", EDBITSON, ResObjName (scrapobj),
		     JUSTSTRING, Type (ResObjType (scrapobj)), EDBITSOFF );

/* Note, can't use AppItem in kernels < 1.3 because of bug */

	    objmenu.items[numobj] = malloc (strlen (name) + 1);
	    strcpy (objmenu.items[numobj], name);

/* Remove the current version of the menu */

	    DelMenu (objhan);
            if (DispMenu (objhan) == ERROR)
	        Error ("DispMenu: %s", WError());

/* And down-load the new one */

	    objhan = MakeMenu (&objmenu);
	    InsMenu (objhan, DEMOMENU);		/* Goes before demo menu */
E 6
	    objects[ numobj++ ] = newobj;
I 6

/* Don't need to redraw the menu bar has it hasn't changed!!! */

E 6
	}
E 5

D 5
/* Add the object to the slideshow file */

    newobj = ResAdd( resfile, type, ResObjName( scrapobj ), 
		     ResObjSize( scrapobj ), resdata );
    if( newobj == ERROR )
	Error( "(ResAdd newobj) %s", ResError() );
    else
    {
	sprintf( name, "%s%c%s", ResObjName( scrapobj ), RJUST,
		 Type( type ) );
	AppItem( pichan, name );
	objects[ numobj++ ] = newobj;
    }

E 5
/* Close the scrap */

D 5
    CloseScrap();
E 5
I 5
	CloseScrap();
    }
E 5
}


/****************** EVENT ROUTINES ********************************************/

/*                                                                *************
                                                                  *           *
                                                                  * MENUSEVENT*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Menu select event.                                                        |
-------------------------------------------------------------------------------
*/

int MenuSEvent( eventptr )
EventRec *eventptr;
{
D 6
    byte menunum = eventptr->message >> 8;
    byte itemnum = eventptr->message;
E 6
I 6
    byte menunum = eventptr->message >> 8;	/* menu id (NOT handle) */
    byte itemnum = eventptr->message;		/* item number */
E 6

    switch( menunum )
    {
D 6
	case EDITMENU:	switch( itemnum )
			{
	    		    case CUTITEM:   Cut();   break;
	    		    case COPYITEM:  Copy();  break;
	    		    case PASTEITEM: Paste(); 
D 5
					    ShowPic( numobj - 1 );
E 5
I 5
					    ShowObj( numobj - 1 );
E 5
					    break;
E 6
I 6
	case EDITMENU:
	    switch( itemnum )
	    {
	       case CUTITEM:   Cut();   break;
	       case COPYITEM:  Copy();  break;
	       case PASTEITEM: Paste(); 
		    ShowObj( numobj - 1 );
		    break;
E 6

D 6
	    		    default:Error( "Bad switch" );
			}
			break;
E 6
I 6
	       case RENAMEITEM:
D 8
		    /*	This case should get a new name for an object
E 8
I 8
		    /*	This case should get a new name for an object */
E 8
		    DisEvents ();
		    getnewname = TRUE;
D 8
		    */
E 8
		    break;
E 6

D 6
	case DEMOMENU:  switch( itemnum )
			{
			    case DEMOITEM:  if( doingdemo )
					    {
						DisTick( demohan, DEMOITEM );
						EnableMenu( edithan );
						EnableMenu( pichan );
						doingdemo = FALSE;
					    }
					    else
					    {
						EnableTick( demohan, DEMOITEM );
						DisMenu( edithan );
						DisMenu( pichan );
						doingdemo = TRUE;
					    }
					    DrawMBar();
					    break;
E 6
I 6
	       default:
	            Error( "No such item: %d in menu: %d",itemnum, menunum );
	    }
	    break;
E 6

D 6
	    		    default:Error( "Bad switch" );
			}
			break;
E 6
I 6
	case DEMOMENU:
	    switch( itemnum )
	    {
	        case DEMOITEM:		/* Toggle the demo state */
		    if( doingdemo )
	            {
		        DisTick( demohan, DEMOITEM );
		        EnableMenu( edithan );	/* Can now cut/paste */
		        EnableMenu( objhan );	/* Can now select also */
		        doingdemo = FALSE;
		    }
		    else
		    {
			EnableTick( demohan, DEMOITEM );
			DisMenu( edithan );	/* Cant cut/paste while demo */
			DisMenu( objhan );	/* Cant select while demo */
			doingdemo = TRUE;
		    }
		    DrawMBar();		/* State of menu bar has changed */
		    break;
E 6

D 6
	case PICMENU:	switch( itemnum )
			{
D 5
			    default: ShowPic( itemnum );
E 5
I 5
			    default: ShowObj( itemnum );
E 5
				     break;
			}
			break;
E 6
I 6
	       default:
	            Error( "No such item: %d in menu: %d",itemnum, menunum );
	    }
	    break;
E 6

I 6
	case OBJMENU:
	    if (itemnum < numobj)
	        ShowObj (itemnum);
	    else
	        Error( "No such item: %d in menu: %d",itemnum, menunum );
	    break;

E 6
	default: Error( "Unknown menu" );
    }
    Flush();
}

/*                                                                *************
                                                                  *           *
E 4
                                                                  * LMDEVENT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|   Left mouse down event handler                                             |
-------------------------------------------------------------------------------
*/

LMDEvent( eventptr )
EventRec *eventptr;
{
D 6
    nextpiccie = TRUE;
E 6
I 6
    nextpiccie = TRUE;	/* Step demo by one */
E 6
    return( 0 );
}

/*                                                                *************
                                                                  *           *
                                                                  * CLOSEEVENT*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Window close event handler.                                                |
-------------------------------------------------------------------------------
*/

int CloseEvent( eventptr )
EventRec *eventptr;
{
D 6
    finished = TRUE;
E 6
I 6
    finished = TRUE;	/* Flag for exit */
E 6
    return( 0 );
}

/*                                                                *************
                                                                  *           *
                                                                  * WUPDEVENT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Window update event handler.                                               |
-------------------------------------------------------------------------------
*/

int WUPDEvent( eventptr )
EventRec *eventptr;
{
    int message = eventptr->message & 0xffff;
 
    if( message == SIZEMESS )
D 6
	ChangePad();
E 6
I 6
	ChangePad();		/* Update the window */
E 6
}

/*                                                                *************
                                                                  *           *
                                                                  * CHANGEPAD *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Update the pad when the window size changes                                |
-------------------------------------------------------------------------------
*/

ChangePad()
{

    rectangle visarea;

    RVisArea( &visarea );
    GetPad( &padinfo );
    padinfo.window.x = 0;
    padinfo.window.y = 0;
    padinfo.window.width = visarea.width;
    padinfo.window.height = visarea.height;
    padinfo.horizstep = (padinfo.pad.width - padinfo.window.width) / STEPS;
    padinfo.vertstep = (padinfo.pad.height - padinfo.window.height) / STEPS;
    PutPad( &padinfo );
D 5
    PlotClip();
E 5
I 5
    DrawClip();
E 5
}

/*                                                                *************
                                                                  *           *
                                                                  * SCROLLEVEN*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Scroll event handler.                                                      |
-------------------------------------------------------------------------------
*/

int ScrollEvent( eventptr )
EventRec *eventptr;
{
    if( eventptr->what != SCROLLEVENT )
	Error( "Unknown scroll event" );
    else
D 5
    	PlotClip();
E 5
I 5
D 6
    	DrawClip();
E 6
I 6
    	DrawClip();	/* Re-draw window */
E 6
E 5
}

/*                                                                *************
                                                                  *           *
                                                                  *   ERROR   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
D 5
|   Shuffle off this mortal coil.                                             |
E 5
I 5
| Report an error                                                             |
E 5
-------------------------------------------------------------------------------
*/

Error( message, a, b, c, d, e, f, g )
char *message;
{
D 6
    char buffer[ 80 ];

    sprintf( buffer, message, a, b, c, d, e, f, g );
D 5
    fprintf( stderr, "Slideshow: %s\n", buffer);
D 4
    exit( 1 );
E 4
I 4
    /* exit( 1 ); */
E 5
I 5
    fprintf( stderr, "%s: %s\n", progname, buffer);
E 6
I 6
    fprintf( stderr, "%s: ",progname);
    fprintf( stderr, message, a, b, c, d, e, f, g );
    fprintf( stderr, "\n");
E 6
E 5
E 4
}
I 5

/*                                                                *************
                                                                  *           *
                                                                  *   FATAL   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Report a fatal error and exit                                               |
-------------------------------------------------------------------------------
*/

Fatal( message, a, b, c, d, e, f, g )
char *message;
{
    Error( message, a, b, c, d, e, f, g);
    exit(1);
I 6
}

/*                                                                *************
                                                                  *           *
                                                                  * SETFUDGE  *
                                                                  * REMFUDGE  *
                                                                  *************
-------------------------------------------------------------------------------
| The next two routines set/remove the EDBITSON and EDBITS off escapes in the |
| passed item. This is only necessary in kernels < 1.3. See InitMenu for more |
| details.                                                                    |
-------------------------------------------------------------------------------
*/

SetFudge (index)
register index;

{
   char *temp;

   if (index >= 0)
   {
       temp = malloc (strlen (objmenu.items[index]) + 7);
       strcpy (temp, EDBITSON);
       strcat (temp, objmenu.items[index]);
       strcat (temp, EDBITSOFF);
       free (objmenu.items[index]);
       objmenu.items[index] = temp;
   }
}

RemFudge (index)
register index;

{
   char *temp;
   int len;

   if (index >= 0)
   {
       len = strlen (objmenu.items[index]) + 1;	/* Include null terminator */
       temp = malloc (len - 6);
       strncpy (temp, objmenu.items[index]+3, len - 7);
       *(temp + len - 7) = 0;
       free (objmenu.items[index]);
       objmenu.items[index] = temp;
   }
}

/*                                                                *************
                                                                  *           *
                                                                  * CLEARVIS  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| This routine clears the visarea of the window to the current background.    |
-------------------------------------------------------------------------------
*/

ClearVis ()
{
D 11
   w_scrollpad padinfo;	/* The restool's pad */
E 11
I 11
   w_scrollpad padinfo;	/* The resshow's pad */
E 11

   GetPad (&padinfo);
   RGToL (&padinfo.window);
   RGToL (&padinfo.window.width);
   ClrRect (&padinfo.window);
}

/*                                                                *************
                                                                  *           *
                                                                  *  RENAME   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| This code is supposed to get a new name from the user and rename the current|
| object to that name...                                                      |
-------------------------------------------------------------------------------
*/

Rename ()
{
D 8
    /*
    re-name code...
E 8
I 8
    FILE *gsstream;
    char newname[ 80 ];
    objid obj;
    extern int errno;
    int gs_ok;
D 10

E 10
I 10
    int type;                   /* Type of this object */
    
E 10
/* Read the output of the 'gs' command */

    gsstream = POpen( "gs -r 15 1 -n Restool Please enter new name", "r" );

    do
    {
	clearerr( gsstream );
	fgets( newname, sizeof( newname ), gsstream );
    } while( ferror( gsstream ) && errno == EINTR );

    newname[ strlen( newname ) - 1 ] = '\0';
    gs_ok = PClose( gsstream );
    getnewname = FALSE;
E 8
    EnEvents ();
D 8
    */
E 8
I 8

D 9
    if( gs_ok == 0)
E 9
I 9
    if( gs_ok != 2 )    /* 2 is error return */
E 9
    {
/* Rename the object in the file */

	obj = objects[ curobj ];
	ResObjRename( resfile, obj, newname );

D 10
    /* Note, can't use AlterItem in kernels < 1.3 because of bug */
E 10
I 10
/* Now alter the internal name of the object */
E 10

I 10
        type = ResObjType( obj );
        if( type == RESFONT || type == RESICON )
        {
            char *objptr = NULL;
            
            if( ResRead( resfile, obj, &objptr ) == ERROR )
                Fatal( "Failed to read object %s", ResError() );
            if( type == RESFONT )
                strncpy( ((font *) objptr)->fname, newname, FNAMESIZE );
            else if( type == RESICON )
                strncpy( ((icon *) objptr)->ident, newname, 8 );
            if( ResWrite( resfile, obj, type, ResObjName( obj ), objptr ) )
                Fatal( "Failed to write object" );
        }
        
/* Note, can't use AlterItem in kernels < 1.3 because of bug */

E 10
	free (objmenu.items[curobj]);
	sprintf( newname, "%s%s%s", ResObjName( obj ), JUSTSTRING,
		 Type( ResObjType( obj ) ) );
	objmenu.items[curobj] = malloc (strlen (newname) + 1);
	strcpy (objmenu.items[curobj], newname);

	if (curobj == numobj-1)
	   SetFudge (curobj);

D 10
    /* Delete the old menu */
E 10
I 10
/* Delete the old menu */
E 10

	DelMenu (objhan);

D 10
    /* And dispose of it */
E 10
I 10
/* And dispose of it */
E 10

	if (DispMenu (objhan) == ERROR)
	    Error ("DispMenu: %s", WError());
	objhan = MakeMenu (&objmenu);

D 10
    /* Insert new menu */
E 10
I 10
/* Insert new menu */
E 10

	InsMenu (objhan, DEMOMENU);

D 10
    /* Don't need to draw menu bar as its contents haven't changed!!! */
	}
E 10
I 10
/* Don't need to draw menu bar as its contents haven't changed!!! */
    }
E 10
E 8
E 6
}
I 8

E 8

E 5
E 1
