


            /********************************************\
            *                                            *
            *  XXX MMI  - Definitions file               *
            *                                            *
            *                                            *
            *                                            *
            \********************************************/


/*                                                                *************
                                                                  *           *
                                                                  * CONSTANTS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| constant definitions                                                        |
-------------------------------------------------------------------------------
*/
   /* caret style values */

#define BLOCKCAR       0
#define VLINECAR       1
#define ULINECAR       2

   /* text style bits */

#define NORMAL      00
#define BOLD        01
#define ULINE       02
#define ITALIC      04
#define INVERSE	    08

#define FACEBITS    0x0f     /* A mask for all the text face bits */
 
   /* font types */

#define FIXED       00
#define PROP        01

   /* Output codes */

#define RSPORT      01       /* Read from screen port to pen */

#define TOGBOLD	    02	     /* Toggle bold mode */

#define DEFEDAREA   03       /* Default edit area */

#define HIDEC       04
#define SHOWC       05
#define RESETC      06
#define BELL        07
#define CURLEFT     08
#define CURRIGHT    09
#define CURDOWN     10
#define CURUP       11
#define CLRSCR      12
#define CR          13
#define CLREOL	    14
#define CLREOW      15

#define EBITBLT     16      /* Does the reverse of plot */

#define TOGITAL     17      /* Toggle italic mode    */
#define TOGINV      18      /* Toggle inverse mode   */
#define TOGUL       19      /* Toggle underline mode */
#define DROPPEN     20
#define RAISEPEN    21
#define PLOT        22

#define INSCHR      23
#define INSLN       24
#define DELCHR      25
#define DELLN       26

#define ESCAPE      27

#define DEFPEN      28       /* Set default pen state */
#define NULLC1      29       /* Code should be unused */
#define HOMEC       30
#define NULLC2      31       /* Code should be unused */

#define BACKSPACE  127


#define CM          ' '      /* Cursor movement escape sequence */
#define CMCS	    '!'      /* Set cursor mark and space escape sequence */
#define CBLOCK      '"'      /* Set cursor size escape sequence */

#define TFACE       '#'      /* Set text   face escape sequence */
#define TMODE       '$'	     /* Set text   mode escape sequence */
#define TFONT       '%'      /* Set text   font escape sequence */

#define SCROLL      '&'      /* Scroll window escape sequence */

#define PMODE       '\''     /* Set pen mode escape sequence */
#define PM          '('      /* Pen movement escape sequence */
#define PMR         ')'      /* Relative pen movement escape sequence */

#define DRAW        '*'      /* Draw a line from current pen position to x, y
				and change pen coordinates to x, y */
#define DRAWR	    '+'	     /* Draw a line from current pen position to pos-   
				ition dx, dy and change pen coordinates to 
				x + dx, y + dy */

#define FRRECT      ','      /* Frame a rectangle */
#define OFRRECT     '-'      /* Frame rectangle outside */
#define FIRECT      '.'      /* Fill a rectangle with current foreground pat */
#define CLRECT      '/'      /* Clear a rectangle */
#define INVRECT     '0'      /* Invert a rectangle */

#define FRELL       '1'      /* Draw a frame inside an ellipse */
#define OFRELL      '2'      /* Draw a frame outside an ellipse */
#define FIELL       '3'      /* Fill an ellipse with current foreground */
#define CLELL       '4'      /* Clear an ellipse to the current background */
#define INVELL      '5'      /* Invert an ellipse */

#define FRARC       '6'      /* Draw a frame inside an arc */
#define OFRARC      '7'      /* Draw a frame outside an arc */
#define FIARC       '8'      /* Fill an arc with current foreground */
#define CLARC       '9'      /* Clear an arc to the current background */
#define INVARC      ':'      /* Invert an arc */

#define FRTRI       ';'      /* Frame a triangle */
#define FITRI       '<'      /* Fill a triangle with current foreground */
#define CLTRI       '='      /* Clear a triangle to the current background */
#define INVTRI      '>'      /* Invert a triangle */

#define PCHANGE     '?'      /* Change pen to .. escape sequence */

#define BPAT        '@'      /* Set background pattern escape sequence */
#define FPAT        'A'      /* Set foreground pattern escape sequence */
#define BCOL        'B'      /* Set background colour  escape sequence */
#define FCOL        'C'      /* Set foreground colour  escape sequence */
#define PENSIZE	    'D'	     /* Set pen size escape sequence */

#define SETUWIN     'E'      /* Set up the users clipping rectangle */
#define SETEDAREA   'F'      /* Set edit area */

#define GSCROLL     'G'      /* Scroll in pixels */

#define DICON       'H'      /* Draw icon n at x, y */
#define EICON       'I'      /* Erase icon n at x, y */
#define SICON       'J'      /* Shade icon n at x, y*/
#define IICON	    'K'	     /* Invert icon n at x, y*/

#define SETEDBITS   'L'      /* Set edit area */

#define TRACKFORM   'M'      /* Set Tracking form */
#define TRACKPAT    'N'      /* Set tracking half-tone */
#define TRACKMODE   'O'      /* Set tracking mode */
#define TRACKSIZE   'P'      /* Set tracking pen size */
#define DEFUWIN     'Q'      /* Set default window */
#define DEFTEXT     'R'      /* Set default text state */
#define CCOLOUR     'S'      /* Set caret colour */

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
/*
 * DONT FORGET TO CHANGE LASTESC UNLESS YOU DONT WANT IT TO BE SEEN !!
 */

   /* Other stuff */

#define FIRSTESC    ' ' 
#define LASTESC     'S' 


#define CMl          2       /* Expect 2 more characters after CM code */
#define CMCSl        2       /* Expect 2 more characters after CMCS code */
#define CBLOCKl      8       /* Expect 8 more characters after CBLOCK code */
#define TFACEl       1       /* Expect 1 more character  after TFACE code */
#define TMODEl       1       /* Expect 1 more character  after TMODE code */
#define TFONTl       2       /* Expect 2 more character  after TFONT code */
#define SCROLLl      1       /* Expect 1 more character  after SCROLL code */
#define PMODEl       1       /* Expect 1 more character  after PMODE code */
#define PMl          4       /* Expect 4 more characters after PM code */
#define PMRl         4       /* Expect 4 more characters after PMR code */
#define DRAWl        4       /* Expect 4 more characters after DRAW code */
#define DRAWRl       4       /* Expect 4 more characters after DRAWR code */

#define FRRECTl	     8       /* Expect 8 more characters after FRRECT code */
#define OFRRECTl     8       /* Expect 8 more characters after OFRRECT code */
#define FIRECTl	     8       /* Expect 8 more characters after FIRECT code */
#define CLRECTl	     8       /* Expect 8 more characters after CLRECT code */
#define INVRECTl     8       /* Expect 8 more characters after INVRECT code */

#define FRELLl       8
#define OFRELLl      8
#define FIELLl       8
#define CLELLl       8
#define INVELLl      8

#define FRARCl      12
#define OFRARCl     12
#define FIARCl      12
#define CLARCl      12
#define INVARCl     12

#define FRTRIl      12
#define FITRIl      12
#define CLTRIl      12
#define INVTRIl     12

#define PCHANGEl     2       /* Expect 2 more character  after PCHANGE code */
#define BPATl        2       /* Expect 2 more character  after BPAT code */
#define FPATl        2       /* Expect 2 more character  after FPAT code */
#define BCOLl        1       /* Expect 1 more character  after BCOL code */
#define FCOLl        1       /* Expect 1 more character  after FCOL code */

#define PENSIZEl     4       /* Args to PENSIZE are its new size */
#define SETUWINl     8       /* A rectangle follows this one */
#define EDAREAl      8

#define GSCROLLl     4

#define DICONl       6
#define EICONl       6
#define SICONl       6
#define IICONl       6

#define EDBITSl      1

#define TRFORMl      2       /* Expect 2 more character after TRACKFORM code */
#define TRPATl       2       /* Expect 2 more character after TRACKPAT code */
#define TRMODEl      1       /* Expect 1 more character after TRACKMODE code */
#define TRSIZEl      4       /* Args to TRACKSIZE are its new size */
#define DEFUWINl     0       /* Set default window */
#define DEFTEXTl     0       /* Set default text state */

#define CCOLOURl     2       /* Expect 2 more characters after CCOLOUR code */

   /* A few miscellaneous constants */

#define TRUE	     1
#define FALSE	     0

#define MAX_ESC     12

#define INSERTC	     0
#define INSERTL      1
#define DELETEC      2
#define DELETEL      3
#define LINE         1
#define SCROLLUP     2
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */
   /* constants for edbits */

#define NORMMASK     0
#define NOWRAP       1             /* Don't wrap to nexrt line at RH edge */
#define PAGE         2             /* When move off bottom, page don't scroll */

/*                                                                *************
                                                                  *           *
                                                                  * CARSTATE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   point     caretpos;
   rectangle caretrect;
   short     caretlevel;
   addrptr   *carettone;
}
carstate;

/*                                                                *************
                                                                  *           *
                                                                  *  EDSTATE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   rectangle editarea;
   word      edbits;
}
edstate;

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
/*                                                                *************
                                                                  *           *
                                                                  *    TQ     *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Type of escape sequence and number of chars left to fill it                 |
-------------------------------------------------------------------------------
*/

typedef struct
{
   byte   type;       /* e.g. cursor movement */
   byte   sema;       /* semaphore, i.e. # of characters left before done */
   byte	  escq [MAX_ESC];	       /* Control queue */
}
tq;


/*                                                                *************
                                                                  *           *
                                                                  *  EDITREC  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Part of V_term tht defines the structures used by the editing functions     |
-------------------------------------------------------------------------------
*/

typedef struct
{
   rectangle   caret;                  /* Area of caret */
   short       caretlevel;             /* Caret visible semaphore */
   word        caretonscreen;          /* Caret actually on the screen */
   addrptr     *carettone;             /* Halftone used for caret */

   rectangle   editarea;               /* realeditarea clipped to usercliparea*/
   word        edbits;                 /* define how editarea is used */

   tq          qstat;		       /* Control queue status */

   grafport    gp;                     /* The grafport structure */
}
edit;


#define		CARET		curd->caret
#define		CURPORT		curd->gp
#define         CLIPAREA        CURPORT.innerclip
#define         EDITAREA        curd->editarea
#define         LCOORDAREA      CURPORT.portframe

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

