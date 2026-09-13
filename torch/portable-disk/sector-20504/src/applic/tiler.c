/*****************************************************************************
 *
 * Program:	Tiler
 * Filename:	tiler.c
 *
 * Author:	I. Jones and D.M. Gilday
 * Date:	9 Oct 1985
 *
 * Purpose:	To demonstrate the tile graphics. 
 *
 * Amendments:	
 *
 *****************************************************************************/
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 1.10\nTriple X";

/*
 *		Release list.
 * 1.10			Released with Issue 1.10.
*/

/* ---------- Include Files: -------------------------------------- */

#include "tiler.h"

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

IMPORT int errno;				/* Error return code */
IMPORT int fd;					/* Window file descriptor */
IMPORT FILE *fs;				/* Stream of current window */
IMPORT int rand();
IMPORT void srand();

/* ---------- Forward declarations -------------------------------- */

FORWARD int LMouseDown();			/* Event actions */
FORWARD int LMouseUp();
FORWARD int WindowUpdate();
FORWARD int MenuSelect();

FORWARD rectangle TilePosition();

/* ---------- Static variables: ----------------------------------- */

PRIVATE int tilePressed = -1;		/* Which tile was pressed. */
PRIVATE int blankTile;			/* BLank tile number */
PRIVATE int turns;			/* Move number */
PRIVATE int opthandle;			/* Handle to menu */
PRIVATE int starting = TRUE;		/* Initialising semaphore */
PRIVATE int selected = TRUE;		/* Window selected flag */
PRIVATE int tobeselected = TRUE;	/* initial state after initialisation */

PRIVATE int option = NULLOPTION;		/* current option selected */

PRIVATE UMenuRec options = 
{
    OPTIONMENU,				/* Menu ID */
    "Options",				/* Menu title */
    0xFFFFFFFF,				/* Menu options enabled */
    0,					/* Menu options ticked  */
    {					/* Text of menu strings. */ 
         "Quit",
         "Reset",
         "Scramble",
         "Solve"
    }
};

PRIVATE rectangle turnsDisplay =
{
    BASE_X + WIDTH + SPACING,
    BASE_Y - HEIGHT - SPACING,
    (COLS - 2) * DX + WIDTH,
    HEIGHT
};

PRIVATE int compass[ 4 ] = 
{ 
    NORTH, 
    SOUTH, 
    EAST, 
    WEST 
};

PRIVATE struct tile tiles[ NUMTILES + 1 ];

PRIVATE int posx[ NUMTILES ];
PRIVATE int posy[ NUMTILES ];

PRIVATE char chr[] = "ABCDEFGHIJKLMNOPQRSTUVWX ";

PRIVATE rectangle window = 
{
    WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT
};

PRIVATE int ( *EventActions[ NUMPROCS ] ) () =
{
    MenuSelect,		/* Menu select event */
    0,			/* Close window event */
    LMouseDown,		/* Left mouse button down event */
    0,			/* Right mouse button down event */
    LMouseUp,		/* Left mouse button up event */
    0,			/* Right mouse button up event */
    0,			/* Key down event */
    0,			/* Key up event */
    0,			/* Auto-repeat event */
    WindowUpdate,	/* Window update event */
    0,			/* Over bounds event */
    0, 0, 0, 0, 0, 0	/* Presently undefined events */
};

/*                                                                *************
                                                                  *           *
                                                                  * MAIN      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| The entry point                                                             |
-------------------------------------------------------------------------------
*/

main()
{
/* Create the window for the application: */

    if( WOpen(  0,		/* Don't associate device with window */
                WOANYUNIQUE,	/* Any (unique) unopened window device */
	        &window,	/* Window rectangle (Global coords) */
	        VANILLA,	/* Window flavour */
	        0,		/* use DM zoompoint from icon or default */
	        EventActions,	/* Event actions vector */
	        0		/* No automatic dragging */
             ) == ERROR )
    {
	Error( "Failed to open window" );
    }  

/* Create the initial rectangles & initialise the keyboard and cursor states */

    Initialise();

    ShowTiles();

    SetSelect();

    starting = FALSE;

    while( TRUE )
    {
/* Ensures that SetSelected() called once in a while */

	sleep( 2 );	
        switch(option)
        {
            case RESET:    Reset();
			   ShowTiles();
             		   break;

            case SCRAMBLE: Scramble();
             		   break;

            case SOLVE:    Solve();
             	       	   break;

            default:	   break;
       }
       SetSelected();
       option = NULLOPTION;
    }
}


/*                                                                *************
                                                                  *           *
                                                                  * RESET     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Reset - sets the tile configuration to what it originally was.              |
-------------------------------------------------------------------------------
*/

PRIVATE Reset()
{
    ClearDisplay();
    blankTile = NUMTILES - 1;
    InitRect();
}

/*                                                                *************
                                                                  *           *
                                                                  * DISALLOWED*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Returns a bitmask of forbidden directions for a given tile number          |
-------------------------------------------------------------------------------
*/

PRIVATE int Disallowed( tile )
int tile;
{
    int result = 0;

    if( tile < COLS ) result |= N;
    if( tile % COLS == 0 ) result |= W;
    if( tile % COLS == COLS - 1 ) result |= E;
    if( tile >= NUMTILES - COLS ) result |= S;

    return( result );
}

/*                                                                *************
                                                                  *           *
                                                                  * INITRECT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Initialise all the tile bounding rectangles.         		      |
-------------------------------------------------------------------------------
*/

PRIVATE InitRect()
{
    int i;

    for( i = 0; i < NUMTILES; i += 1 )
    {
	tiles[ i ].pattern = FOREGROUND;
        tiles[ i ].current = ( NUMTILES - 1 ) - i;
	tiles[ i ].verboten = Disallowed( i ); 
        tiles[ i ].box = TilePosition( i );
	posx[ i ] = i % COLS;
	posy[ i ] = i / COLS;
    }
    tiles[ blankTile ].pattern = BTILECOL;
    tiles[ GOAWAY ].pattern = FOREGROUND;
    tiles[ GOAWAY ].box = TilePosition( GOAWAY );
}

/*                                                                *************
                                                                  *           *
                                                                  *INITIALISE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Create tile rectangles and inserts menu options into menu bar.              |
-------------------------------------------------------------------------------
*/

PRIVATE Initialise()
{
    imstruct insert;		/* Menu structure to be inserted */
    struct termio termargs;
 
/* Hide the caret (text cursor) */

    HideCursor();

/* The following piece of code blocks all input and turns off character */
/* post-processing. This is to prevent characters typed at the keyboard */
/* from appearing in the puzzle window, and tab expansion etc. from     */
/* disrupting the graphics escape sequences.           			*/

    ioctl( fd, TCGETA, &termargs );
    termargs.c_oflag &= ~OPOST;
    termargs.c_iflag |= KBLOCK;
    ioctl( fd, TCSETA, &termargs );

/* Reset the tile positions */

    Reset();

/* Download the options menu onto the system heap: */

    opthandle = MakeMenu( &options ); 
    if( opthandle == ERROR )
	Error( "Could not create Menu" );

/* Add the menu to the menu bar */

    insert.handle = opthandle;
    insert.insbefore = 0;
    InsertMenu( &insert );

/* Cause the menu bar to be displayed. */

    DrawMBar();
}


/*                                                                *************
                                                                  *           *
                                                                  * SCRAMBLE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Increase the entropy of the tiles array.                                   |
-------------------------------------------------------------------------------
*/

PRIVATE Scramble()
{
    register int i;

    srand( (short) time( (long *) 0 ) );

    for( i = 0; i < SCRAMBLES; i ++ )
    {
	doBk(xc(0), rand() % ROWS);
	doBk(rand() % COLS, yc(0));
    }
    turns = 0;
    DrawTurns();
}

/*                                                                *************
                                                                  *           *
                                                                  * CLEARDISPL*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Clear the puzzle window, the moves display and reset the turns to 0.        |
-------------------------------------------------------------------------------
*/

PRIVATE  ClearDisplay()
{
    rectangle localWindow;		/* Window in local coords */

/* Reset the number of turns */

    turns = 0;

/* Calculate the window position in local coordinates */

    localWindow = window;
    RectGToL( &localWindow );

/* Fill the puzzle window with a lightblue halftone */

    SetFPat( LIGHTBLUE );
    FillRect( &localWindow );

/* Fill the turns dsplay */

    if( selected )
    {
	SetFPat( NULLBITS );
	FillRect( &turnsDisplay );
    }
    else
	ClrRect( &turnsDisplay );

/* Frame the display (on the outside) */

    SetFPat( RED );
    OFrameRect( &turnsDisplay );

    SetFPat( NULLBITS );
    DrawTurns();
    Flush();
}

/*                                                                *************
                                                                  *           *
                                                                  * SHOWTILES *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Draw all the tiles in the puzzle.                                           |
-------------------------------------------------------------------------------
*/

PRIVATE ShowTiles()
{
    int i;				/* Gash loop variable */

/* The close box is also represented as a tile, so draw this first: */

    DrawTile( GOAWAY );

/* Then draw all the ordinary tiles. */

    for( i = 0; i < NUMTILES; i += 1 )
    {
	DrawTile( i );
    }
}


/*                                                                *************
                                                                  *           *
                                                                  * TILEPOSITI*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Returns the position rectangle for a given tile number.                    |
-------------------------------------------------------------------------------
*/

PRIVATE rectangle TilePosition( tile )
int tile;
{
    rectangle winrect;
    int x, y;

    if( tile == GOAWAY )
    {
	x = 0; y = -1;
    }
    else
    {
	x = tile % COLS;
	y = tile / COLS;
    }

    winrect.x = BASE_X + x * WIDTH  + x * SPACING;
    winrect.y = BASE_Y + y * HEIGHT + y * SPACING;
    winrect.width = WIDTH;
    winrect.height = HEIGHT;

    return( winrect );
}


/*                                                                *************
                                                                  *           *
                                                                  * RECTGTOL  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Convert a rectangle from global to local coordinates.                      |
-------------------------------------------------------------------------------
*/

PRIVATE RectGToL( winrect )
rectangle *winrect;
{
/* Convert the global window rectangle into local coordinates. Because */
/* the conversion routine does both a translation and a scaling, it is */
/* necessary to map the width and height of the rectangle onto a point */
/* prior to calling - this ensures that the translation does not affect*/
/* the scaling process. */
    
    winrect->width  += winrect->x;
    winrect->height += winrect->y;
    GToL( winrect );			/* The top left corner point */
    GToL( &( winrect->width ) );	/* the bottom right corner   */
    winrect->width  -= winrect->x;
    winrect->height -= winrect->y;
}

/*                                                                *************
                                                                  *           *
                                                                  * DRAWTILE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Draw a given tile, complete with label.                                     |
-------------------------------------------------------------------------------
*/

PRIVATE DrawTile( tileNumber )
int  tileNumber;
{
    int tile = tiles[ tileNumber ].current; 	/* Tile at this posn */
    char letter = chr[ NUMTILES - tile - 1 ];	/* The label */ 
    char pattern = tiles[ tileNumber ].pattern; /* Halftone fill pattern */
    rectangle position;

    position = tiles[ tileNumber ].box;

/* Set the foreground pattern according to the tile we are drawing as: */

    SetFPat( pattern );

/* Fill the rectangle */

    FillRect( &position );

/* Put a frame just outside the rectangle */

    SetFPat( FRAMECOL );
    OFrameRect( &position ); 

/* Plot the tile's character */

    SetFPat( NULLBITS );
    Inverse( OFF );

/* Use logical OR to combine the text and screen. This allows printing of */
/* text onto non-white */

    TextMode( M_OR );

    if( tileNumber == GOAWAY )
    {
	MoveCursor( XOFFSET, 1 );
	putc( BYE, fs );
    }
    else
    {
	MoveCursor( cx( tileNumber ), cy( tileNumber ) );
	putc( letter, fs );
    }

/* Reset the text mode back to normal (copy) */

    TextMode( M_COPY );
}

/*                                                                *************
                                                                  *           *
                                                                  *  CX       *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Cursor X position for a given tile.                                        |
-------------------------------------------------------------------------------
*/

PRIVATE int cx( tileno )
{
    return( (tileno % COLS) * XGAP + XOFFSET );
}



/*                                                                *************
                                                                  *           *
                                                                  *    CY     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Cursor Y position for a given tile.                                         |
-------------------------------------------------------------------------------
*/

PRIVATE int cy( tileno )
int tileno;
{
    return( (tileno / COLS) * (YGAP + 1) + YOFFSET );
}

/*                                                                *************
                                                                  *           *
                                                                  *SETSELECTED*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| SetSelected - sets the title to either hilighted or not, as appropriate.    |
-------------------------------------------------------------------------------
*/

PRIVATE SetSelected()
{
   while(selected != tobeselected)
   {
      InvRect(&turnsDisplay);
      Flush();
      selected = !selected;
   }
}


/*                                                                *************
                                                                  *           *
                                                                  * DRAWTURNS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  DrawTurns - displays the number of turns taken so far:                     |
-------------------------------------------------------------------------------
*/

PRIVATE DrawTurns()
{
/* Output the number of turns so far: */
  
    MoveCursor( DISPLAY_X, DISPLAY_Y ); 
    Flush();
    if( selected )
	Inverse( ON );
    else
	Inverse( OFF );
    fprintf( fs, "Move = %d  ", turns ); 
    Flush();
}

/*                                                                *************
                                                                  *           *
                                                                  * LEGALMOVE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| LegalMove - returns TRUE if the tile picked is a legal move.                |
-------------------------------------------------------------------------------
*/

PRIVATE int LegalMove( tile )
int tile;
{
    int forbidden = tiles[ tile ].verboten;
    int result = FALSE;

/* A legal move is a tile that is on the same row or column as the */
/* blank tile (Excluding the blank tile itself!)		   */

    if( tile != blankTile && (  cx( tile ) == cx( blankTile ) ||
        			cy( tile ) == cy( blankTile ) ) )
	result = TRUE;
    else
	result = FALSE;
    
    return( result );
}

/*                                                                *************
                                                                  *           *
                                                                  * SWAP      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Exchange a pair of integers
-------------------------------------------------------------------------------
*/

PRIVATE Swap( a, b )
int *a, *b;
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

/*                                                                *************
                                                                  *           *
                                                                  * EXCHANGETI*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Exchanges a given pair of tiles.                                           |
-------------------------------------------------------------------------------
*/

PRIVATE ExchangeTiles( tile1, tile2 )
int tile1, tile2;
{
    int temp;
    int tc1, tc2;

    tc1 = tiles[ tile1 ].current;
    tc2 = tiles[ tile2 ].current;

    Swap( &tiles[ tile1 ].current, &tiles[ tile2 ].current );
    Swap( &tiles[ tile1 ].pattern, &tiles[ tile2 ].pattern );
    Swap( &posx[ tc1 ], &posx[ tc2 ] );
    Swap( &posy[ tc1 ], &posy[ tc2 ] );

    DrawTile( tile1 );
    DrawTile( tile2 ); 
    Flush();

/* Assume that the second tile is the blank tile */

    blankTile = tile2;
}


/*                                                                *************
                                                                  *           *
                                                                  * MOVETILE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  MoveTile - Moves a tile, possibly shunting up others as it goes along.     |
-------------------------------------------------------------------------------
*/

PRIVATE MoveTile( tile )
{
    int bx = cx( blankTile ), by = cy( blankTile );
    int tx = cx( tile ),      ty = cy( tile );
    int x, y, dx, dy;
	
/* If the tiles are in the same column, step vertically */

    if( tx == bx )
    {
	dy = (ty < by) ? NORTH : SOUTH;
	for( y = blankTile + dy; y != tile; y += dy )
	{
	    ExchangeTiles( blankTile, y );
	}
	ExchangeTiles( blankTile, y );
    }
    else /* ty == by */
    {
	dx = (tx < bx) ? WEST : EAST;
	for( x = blankTile + dx; x != tile; x += dx )
	{
	    ExchangeTiles( blankTile, x );
	}
	ExchangeTiles( blankTile, x );
    } 
    turns += 1;
    DrawTurns();
}


/*                                                                *************
                                                                  *           *
                                                                  * CHECKTILE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| CheckTile - Returns the tile number (or -1) that the given coordinate is in.|
-------------------------------------------------------------------------------
*/

PRIVATE int CheckTile( where )
point *where;
{
    int result = -1;
    int i;

    for( i = 0; i <= NUMTILES; i += 1 )
    {
	if( PtInRect( where, &tiles[ i ].box ) )
	{
	    result = i;
	    break;
	}
    }
    return( result );
}

/*                                                                *************
                                                                  *           *
                                                                  *   SGN     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Return:   (-1 if x < 0), (0 if x = 0), (or +1 if x > 0)                     |
-------------------------------------------------------------------------------
*/

PRIVATE int Sgn( x )
int x;
{
    return( (x == 0) ? 0 : ( (x > 0) ? 1 : -1 ) );
}

/*                                                                *************
                                                                  *           *
                                                                  *   PC      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Return the piece that should be at position x, y.                           |
-------------------------------------------------------------------------------
*/

PRIVATE int pc( x, y )
int x, y;
{
    return( x + y * COLS );
}

/*                                                                *************
                                                                  *           *
                                                                  *   AT      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Return the piece actually AT position x, y                                  |
-------------------------------------------------------------------------------
*/

PRIVATE int at( x, y )
int x, y;
{
    return( tiles[ ( COLS - x - 1 ) + ( ROWS - y - 1 ) * COLS ].current );
}

/*                                                                *************
                                                                  *           *
                                                                  *    XC     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Return the X coordinate of a given piece                                    |
-------------------------------------------------------------------------------
*/

PRIVATE int xc( p )
int p;
{
    return( posx[ p ] );
}

/*                                                                *************
                                                                  *           *
                                                                  *   YC      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Return the Y coordinate of a given piece                                    |
-------------------------------------------------------------------------------
*/

PRIVATE int yc( p )
int p;
{
    return( posy[ p ] );
}

/*                                                                *************
                                                                  *           *
                                                                  *   MV      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Moves the piece specified by it's tile number where 0 is the blank.	      |
-------------------------------------------------------------------------------
*/

PRIVATE mv( tile )
register int tile;
{
    register int i;

    SetSelected();	/* This ensures that the turns rectangle is hilighted */
			/* correctly even during scrambling or solving */

    for (i = 0; tiles[i].current != tile; i++);
	MoveTile( i );
}


/*                                                                *************
                                                                  *           *
                                                                  * SOLVE     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Solve the tile puzzle automatically                                        |
|                   							      |
|    +-+-+-+-+-+-+-+							      |
|    |*|*|*|*|*|*|*|    =  tile correctly in place and not disturbed	      |
|    +-+-+-+-+-+-+-+							      |
|    |*|*|*|*|*|*|*|    *  tile placed in correct position by this function   |
|    +-+-+-+-+-+-+-+							      |
|    |*|*|*|*|*|*|*|    B  position of blank on exit                          |
|    +-+-+-+-+-+-+-+							      |
|    |*|*|*|*|*|*|*|   ' ' undefined contents, possibly disturbed during call |
|    +-+-+-+-+-+-+-+							      |
|  0 |*|*|*|*|*|*|B|							      |
|    +-+-+-+-+-+-+-+							      |
|                 0             					      |
-------------------------------------------------------------------------------
*/

PRIVATE Solve()
{
    int row;
    
    turns = 0;
    DrawTurns();

/* Solve all the rows apart from the last two */

    for( row = ROWS - 1; row >= 2; row -= 1 )
    {
	SolveRow( row );
    }

/* Solve the last two rows. */

    do2Rows();
}

/*                                                                *************
                                                                  *           *
                                                                  * SOLVEROW  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Places all of the pieces in a given row in the correct position without     |
| moving any of the pieces in rows above the one to be solved.                |
|                   							      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    =  tile correctly in place and not disturbed	      |
|    +-+-+-+-+-+-+-+							      |
|  Y |*|*|*|*|*|*|*|    *  tile placed in correct position by this function   |
|    +-+-+-+-+-+-+-+							      |
|    | | | | | | | |    B  position of blank on exit - undefined              |
|    +-+-+-+-+-+-+-+							      |
|    | | | | | | | |   ' ' undefined contents, possibly disturbed during call |
|    +-+-+-+-+-+-+-+							      |
|  0 | | | | | | | |							      |
|    +-+-+-+-+-+-+-+							      |
|                 0             					      |
-------------------------------------------------------------------------------
*/

PRIVATE SolveRow( row )
int row;
{
    int X;

/* Solve N-1 columns in this row */

    for( X = COLS - 1; X >= 1; X -= 1 )
    {
	Do1( X, row );
    }

/* Do the last column in this row */

    doLast1( row );
}


/*                                                                *************
                                                                  *           *
                                                                  *   DO1     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Given a position, moves the tile which belongs in this position to this     |
| place without disturbing any tiles in higher rows or in the same row and    |
| and in higher column numbers. The blank's final position is undefined.      |
|                   							      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    =  tile correctly in place and not disturbed	      |
|    +-+-+-+-+-+-+-+							      |
|  Y |=|=|=|*| | | |    *  tile placed in correct position by this function   |
|    +-+-+-+-+-+-+-+							      |
|    | | | | | | | |    B  position of blank on exit                          |
|    +-+-+-+-+-+-+-+							      |
|    | | | | | | | |   ' ' undefined contents, possibly disturbed during call |
|    +-+-+-+-+-+-+-+							      |
|  0 | | | | | | | |							      |
|    +-+-+-+-+-+-+-+							      |
|           X     0             					      |
-------------------------------------------------------------------------------
*/

PRIVATE Do1( X, Y )
int X, Y;
{
/* Find the piece that should be at X, Y */

    int P = pc( X, Y );

/* If that piece is already in position, we need do no more. */

    if( P == at( X, Y ) ) return;

/* If the X coordinate of the piece is greater than its target: */

    if( xc( P ) > X )
    {
	if( yc( 0 ) == Y )
	    doBk( xc( 0 ), Y - 1 );
	doMv( P, X, Y - 1, X - 1, Y - 1, COLS - 1, Y - 1 );
    }
    else if( xc( 0 ) > X )
    {
	if( yc( P ) == Y )
	   doBk( xc( P ), Y - 1 );
	else
	    doMv( P, X, Y - 1, X - 1, Y - 1, COLS - 1, Y - 1 );
    }
    doMv( P, X, Y, -1, -1, X, Y );
}

/*                                                                *************
                                                                  *           *
                                                                  * DOBK      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Moves the blank to the given position by sliding only tiles in the same    |
|  column and/or row. This is the basic primitive used throughout the solve   |
|  functions.                                                                 |
-------------------------------------------------------------------------------
*/

PRIVATE doBk( X, Y )
int X, Y;
{
    if( xc( 0 ) != X )
	mv( at( X, yc( 0 ) ) );
    if( yc( 0 ) != Y )
	mv( at( X, Y ) );
}


/*                                                                *************
                                                                  *           *
                                                                  *   DOMV    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Moves the tile number P to position (X, Y) and the blank to (x, y) moving   |
| only tiles within the smallest rectangle bounding (X, Y), (x, y), (ux, uy)  |
| and the current positions of tile P and the blank. Unless x<0 or y<0 when   |
| the point (x, y) is ignored and the blank may be left anywhere within the   |
| bounding rectangle. 							      |
-------------------------------------------------------------------------------
*/

PRIVATE doMv( P, X, Y, x, y, ux, uy )
int P, X, Y, x, y, ux, uy;
{
    if( xc( P ) != X )
	doMvX( P, X, ux, uy );
    if( yc( P ) != Y )
	doMvY( P, Y, ux, uy );
    if( x >= 0 && y >= 0 )
	doMvB( X, Y, x, y, ux, uy );
}

/*                                                                *************
                                                                  *           *
                                                                  *  DOMVX    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Moves the tile P horizontally to the appropriate column X, only disturbing  |
| tiles in the smallest rectangle bounding the initial and final positions of |
| the tile P, (ux, uy) and the initial blank position.                        |
-------------------------------------------------------------------------------
*/

PRIVATE doMvX( P, X, ux, uy )
int P, X, ux, uy;
{
    int x, y, s;

    x = xc( P );
    y = yc( P );
    s = Sgn( X - x );
    do
    {
	doMvB( x, y, x + s, y, ux, uy );
	mv( P );
	s = Sgn( X - x );
	x += s;
    } while( x != X );
}

/*                                                                *************
                                                                  *           *
                                                                  *  DOMVY    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Moves the tile P vertically to the appropriate row Y, only disturbing       |
| tiles in the smallest rectangle bounding the initial and final positions of |
| the tile P, (ux, uy) and the initial blank position.                        |
-------------------------------------------------------------------------------
*/

PRIVATE doMvY( P, Y, ux, uy )
int P, Y, ux, uy;
{
    int x, y, s;

    x = xc( P );
    y = yc( P );
    s = Sgn( Y - y );

    do
    {
	doMvB( x, y, x, y + s, ux, uy );
	mv( P );
	y += s;
    } while( y != Y );
}

/*                                                                *************
                                                                  *           *
                                                                  *  DOMVB    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Moves the blank to position (x, y) only disturbing tiles in the smallest    |
| rectangle bounding the points (x, y), (X, Y), (ux, uy) and the initial      |
| position of the blank. The tile at position (X, Y) is also left in place.   |
-------------------------------------------------------------------------------
*/
PRIVATE doMvB( X, Y, x, y, ux, uy )
int X, Y, x, y, ux, uy;
{
    if( at( x, y ) == 0 ) return;

    if( X == x )
    {
	if( xc( 0 ) == x ) 
    	{
	    if( xc( 0 ) == ux )
	        doBk( xc( 0 ) - 1, yc( 0 ) );
	    else
	        doBk( xc( 0 ) + 1, yc( 0 ) );
        }
    	doBk( xc( 0 ), y );
        doBk( x, y );
        return;
    }

    if( Y == y )
    {
	if( yc( 0 ) == y ) 
    	{
	    if( yc( 0 ) == uy )
	        doBk( xc( 0 ), yc( 0 ) - 1 );
	    else
	        doBk( xc( 0 ), yc( 0 ) + 1 );
        }
        doBk( x, yc( 0 ) );
        doBk( x, y );
        return;
    }
    if( xc( 0 ) == X )
	doBk( xc( 0 ), y );

    doBk( x, y );
}

/*                                                                *************
                                                                  *           *
                                                                  *  DOLAST1  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Moves the tile which belongs at (0, Y) into position, assuming that the tile|
| at (1, Y) is correct before the function is called. Tiles in the rows above |
| the given row and tiles to the left of the last two tiles in this row are   |
| undisturbed.                                                                |
|                   							      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    =  tile correctly in place and not disturbed	      |
|    +-+-+-+-+-+-+-+							      |
|  Y |=|=|=|=|=|*|*|    *  tile placed in correct position by this function   |
|    +-+-+-+-+-+-+-+							      |
|    | | | | | | | |    B  position of blank on exit                          |
|    +-+-+-+-+-+-+-+							      |
|    | | | | | | | |   ' ' undefined contents, possibly disturbed during call |
|    +-+-+-+-+-+-+-+							      |
|  0 | | | | | | | |							      |
|    +-+-+-+-+-+-+-+							      |
|               1 0             					      |
-------------------------------------------------------------------------------
*/

PRIVATE doLast1( Y )
int Y;
{
    int P = pc( 0, Y );

    if( at( 0, Y ) == 0 )
	doBk( 0, Y - 1 );
    if( at( 0, Y ) == P ) return;

    doMv( P, 0, Y - 2, 0, Y - 1, COLS - 1, Y - 1 );
    doMv( pc( 1, Y ), 0, Y, 0, Y - 1, 1, Y );
    doMv( P, 0, Y - 1, 1, Y - 1, COLS - 1, Y - 1 );
    doBk( 1, Y );
    doBk( 0, Y );
    doBk( 0, Y - 1 );
}

/*                                                                *************
                                                                  *           *
                                                                  *  DO2ROWS  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Solves the last two rows of the puzzle without disturbing any tiles above   |
| these rows.                                                                 |
|                   							      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    =  tile correctly in place and not disturbed	      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    *  tile placed in correct position by this function   |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    B  position of blank on exit                          |
|    +-+-+-+-+-+-+-+							      |
|    |*|*|*|*|*|*|*|   ' ' undefined contents, possibly disturbed during call |
|    +-+-+-+-+-+-+-+							      |
|  0 |*|*|*|*|*|*|B|							      |
|    +-+-+-+-+-+-+-+							      |
|                 0             					      |
-------------------------------------------------------------------------------
*/

PRIVATE do2Rows()
{
    int X;
    for( X = COLS - 1; X >= 2; X -= 1 )
    {
	do2( X );
    }
    doMv( pc( 0, 1 ), 0, 1, 0, 0, 1, 1 );	/* Once the blank is correct */
						/* the last 3 tiles must be !*/
}

/*                                                                *************
                                                                  *           *
                                                                  * DO2       *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Correctly positions the bottom two tiles in the given column, only          |
| disturbing tiles in the rectangle bounding (0, 0) and (X, 1).               |
|                   							      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    =  tile correctly in place and not disturbed	      |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    *  tile placed in correct position by this function   |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|=|=|=|=|    B  position of blank on exit                          |
|    +-+-+-+-+-+-+-+							      |
|    |=|=|=|*| | | |   ' ' undefined contents, possibly disturbed during call |
|    +-+-+-+-+-+-+-+							      |
|  0 |=|=|=|*| | | |							      |
|    +-+-+-+-+-+-+-+							      |
|           X     0             					      |
-------------------------------------------------------------------------------
*/

PRIVATE do2( X )
int X;
{
    int P = pc( X, 0 );
    int P1 = pc( X, 1 );

    if( at( X, 0 ) == P && at( X, 1 ) == P1 ) return;

    doMv( P, X, 0, X - 1, 0, X, 1 );
    if( at( X, 1 ) == P1 ) return;

    doMv( P1, X - 2, 1, X - 1, 1, X - 1, 1 );
    doMv( P, X, 1, X - 1, 1, X, 1 );
    doMv( P1, X - 1, 1, X - 1, 0, X - 1, 1 );
    doBk( X, 0 );
    doBk( X, 1 );
    doBk( X - 1, 1 );
}

/*                                                                *************
                                                                  *           *
                                                                  *  DIE      *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Close the window and exit the program.                                     |
-------------------------------------------------------------------------------
*/

PRIVATE Die()
{
    WClose( fd );
    exit( 0 );
}

/*                                                                *************
                                                                  *           *
                                                                  *  ERROR    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Error - issues the error message and shuffles off this mortal coil.         |
-------------------------------------------------------------------------------
*/

PRIVATE Error( message )
char *message;
{
    fprintf( stderr, "Tiler: %s\n", message );
    exit( 1 );
}

/*********************** Event handlers *************************************/

/*                                                                *************
                                                                  *           *
                                                                  *MENUSELECT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| MenuSelect - Act upon a menu selection event.                               |
-------------------------------------------------------------------------------
*/

PRIVATE MenuSelect( eventptr )
EventRec *eventptr;
{
    byte menunum = eventptr->message >> 8;
    byte itemnum = eventptr->message;

    if( starting || option != NULLOPTION )
	return;

    if( menunum == OPTIONMENU )
    {
	switch( itemnum )
	{
	    case 0: /* Quit */
		    Die();	/* Hello Charon... */
		    break;

	    case 1: /* Reset */
		    option = RESET;
		    break;

	    case 2: /* Scramble */
		    option = SCRAMBLE;
		    break;

	    case 3: /* Solve */
		    option = SOLVE;
		    break;
	}
    }
    else
	Error( "Unknown menu" );
}


/*                                                                *************
                                                                  *           *
                                                                  * LMOUSEDOWN*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| LMouseDown - Action taken when left mouse key is pressed.                   |
-------------------------------------------------------------------------------
*/

PRIVATE LMouseDown( eventptr )
EventRec *eventptr;
{
    point mouseposn;

    if( starting || option != NULLOPTION )
	return;

    mouseposn = eventptr->where;

/* Convert the mouse position into local coordinates */

    GToL( &mouseposn );

/* Find which tile (-1 if none) the mouse was pressed over. */

    tilePressed = CheckTiles( &mouseposn );

/* If we pressed the mouse while over a tile, highlight it. */

    if( tilePressed != ERROR ) 
    {
	if( tilePressed == GOAWAY  || LegalMove( tilePressed ) )
	{
	    InvRect( &tiles[ tilePressed ].box );
	    Flush();
	}
	else
	    tilePressed = -1;
    }

/* Otherwise, if the mouse was pressed inside the turnsdisplay, drag window */

    else if( PtInRect( &mouseposn, &turnsDisplay ) ) 
	DoDrag();
}

/*                                                                *************
                                                                  *           *
                                                                  * LMOUSEUP  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| LMouseUp - Action taken when left mouse key up event occurs.                |
-------------------------------------------------------------------------------
*/

PRIVATE LMouseUp( eventptr )
EventRec *eventptr;
{
    point mouseposn;
    int blankPosn;		/* Tile where blank currently is */
    int tileReleased;		/* Tile mouse released over */

    if( !starting && option == NULLOPTION )
    {

	mouseposn = eventptr->where;

/* Convert the mouse position into local coordinates */

        GToL( &mouseposn );

        if( tilePressed != -1 )
        {

/* If we released the mouse while over a tile, unhighlight it. */

	    InvRect( &tiles[ tilePressed ].box );
	    Flush();

/* Find which tile (-1 if none) the mouse was released over. */

	    tileReleased = CheckTiles( &mouseposn );

	    if( tileReleased == tilePressed )
	    {

/* If we clicked the go away box, die */

	        if( tilePressed == GOAWAY )
	        {
	            Die();
	        }
	        else
		    MoveTile( tilePressed );
	    }
        }

        tilePressed = -1;
    }
}

/*                                                                *************
                                                                  *           *
                                                                  *WINDOWUPDAT*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| WindowUpdate - this action taken when a window update event occurs.         |
-------------------------------------------------------------------------------
*/

PRIVATE WindowUpdate( eventptr )
EventRec *eventptr;
{
    byte updatetype = eventptr->message;
    
/* If the window was obscured or UpFronted, then toggle the display. */

    if( updatetype == ACTMESS )
    {
	tobeselected = TRUE;
    }
    else if( updatetype == DEACTMESS )
    {
	tobeselected = FALSE;
    }			
    Flush();
}
