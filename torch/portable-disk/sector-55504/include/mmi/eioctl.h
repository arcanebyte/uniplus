            /********************************************\
            *                                            *
            *  EDIT IOCTL Definitions file               *
            *                                            *
            *                                            *
            *                                            *
            \********************************************/

/* SCCS identification "@(#) eioctl.h 1.2@(#)" */

/*                                                                *************
                                                                  *           *
                                                                  * CONSTANTS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| constant definitions                                                        |
-------------------------------------------------------------------------------
*/

#define	EGCARSTAT		0     /* get caret state */
#define ERUCLIP                 1     /* Read user clipping area */
#define EGEDSTAT                2     /* Get editarea state */
#define EOICON                  3     /* Open icon */
#define ECICON                  4     /* Close icon */
#define ERIDENT                 5     /* Read icon identifier */
#define ETVEC                   6     /* track icon */
#define ECTVEC                  7     /* cancel icon tracking */
#define EGTSTAT                 8     /* get tracking state */
#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY9                  9
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

#define EOTXFONT	       10     /* open text font */
#define ECTXFONT	       11     /* close text font */ 
#define ERTXFONT	       12     /* read text font structure. */
#define ERFONTD                13     /* Read font data e.g. glyphs */
#define ERSTRW		       14     /* read string width */
#define EGFONTS                15     /* get font state */
#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY16                16
#define DUMMY17                17
#define DUMMY18                18
#define DUMMY19                19
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

#define EOFORM		       20     /* open form */
#define ECFORM		       21     /* close form */
#define EGPENS                 22     /* Get pen state */
#define ERPIXCOL               23     /* rd col of pixel under pen*/	
#define EWPIXCOL               24     /* wr col of pixel under pen*/	
#define ERFORM                 25     /* Read form structure */
#define ERFORMD                26     /* Read form's bitmap */
#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY27                27
#define DUMMY28                28
#define DUMMY29                29
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */


#define EPWIND		       30     /* pull window to front */
#define EINWIND		       31     /* return if point is in window */
#define ECHWIND                32     /* change window by dx,dy,dw,dh */
#define ECLOSEW                33     /* close the window */
#define EFWIND                 34     /* which part of window is point in? */
#define ETDRAG                 35     /* track drag */
#define ETGROW                 36     /* track grow */
#define EDDRAG                 37     /* perform the drag, cancel tracking */
#define EDGROW                 38     /* perform the grow, cancel tracking */
#define ECTRACK                39     /* cancel tracking */

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY40                40
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */
#define EOBMAP		       41     /* open bitmap */
#define ECBMAP		       42     /* close bitmap */
#define EGCOLS		       43     /* get colour state */
#define EGPATS                 44     /* get pattern state */
#define ERPAT		       45     /* pattern read */
#define ELTOG		       46     /* conv pt to global coords */
#define EGTOL                  47     /* conv pt to local coords */
#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY48                48
#define DUMMY49                49
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

#define EOCUR                  50     /* open cursor */
#define ECCUR                  51     /* close cursor  */ 
#define ECHCUR                 52     /* change cursor to.. */

#define EINVBOX                53     /* Invert close box of a window */
#define ECHZPNT                54     /* Change zoom point of pgrp */
#define EGWMODS                55     /* Get window mode state */
#define ESWMODS                56     /* Set window mode state */
#define EDRAWTITLE             57     /* Draw the title bar */
#define EGWAREAS               58     /* Get window areas state */
#define ESWAREAS               59     /* Set window areas state */

#define ESOE		       60     /* signal on event */
#define ESNE		       61     /* signal next event */
#define EEVNTH		       62     /* event handled */
#define EDCLICK                63     /* detect a click event */
#define ETIMER                 64     /* fine timer for user progs */
#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY65                65
#define DUMMY66                66
#define DUMMY67                67
#define DUMMY68                68
#define DUMMY69                69
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

#define ENEWMENU               70     /* new menu allocation */
#define EAPPITEM               71     /* append item to menu */
#define EDISMENU               72     /* dispose of menu */
#define EINSMENU               73     /* insert menu into menu bar */
#define EDELMENU               74     /* delete menu from menu bar */
#define EDRMBAR                75     /* draw menu bar */
#define ECLRMBAR               76     /* clear menu bar */
#define EENITEM                77     /* enable an item */
#define EDISITEM               78     /* disable an item */
#define EMENUSEL               79     /* menuselect event handler */
#define EINITMS                80     /* menuselect initialisation*/
#define EENTICK                81     /* enable tick next to item */
#define EDISTICK               82     /* disable tick */
#define EALTITEM               83     /* alter menu item */
#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define DUMMY84                84
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

#define ELOCK                  85     /* lock a handle and make it public */
#define EULOCK                 86     /* unlock a handle and make it private */

   /* The following ioctl calls do not require an open window, just a tty */

#define ESDCAR                128     /* set default caret state */
#define ESDFONT               129     /* set default font state */

#define EHENQ                 130     /* read handle type from handle */

#define EGFREEW               131     /* Return number of free window */
#define EIWIND                132     /* initialise window */

#define ERSMODE               133     /* read screen mode */
#define EWSMODE               134     /* write screen mode */
#define ERLCPAL               135     /* read logical colour palette  */
#define EWLCPAL               136     /* write logical colour palette */
#define ERPALDEL              137     /* read palette delay */
#define EWPALDEL              138     /* write palette delay */
#define ERVISSCR              139     /* read visible screen # */
#define EWVISSCR              140     /* write visible screen # */
#define ERWORSCR              141     /* read working screen # */
#define EWWORSCR              142     /* write working screen # */
#define ERSCRWID              143     /* read screen width in pixels */
#define EWSCRWID              144     /* write screen width in pixels */

#define ERBEEP                145     /* read beep data */
#define EWBEEP                146     /* write beep data */

#define ERDEFK                147     /* Read soft key definition */
#define EWDEFK                148     /* Write soft key definition */
#define ERREPK                149     /* Read key auto-repeat rate */
#define EWREPK                150     /* Write key auto-repeat rate */
#define ERKMAT                151     /* Read current keyboard matrix */
#define EWKMAT                152     /* Write current keyboard matrix */

#define ERCURH                153     /* read current cursor handle */
#define EHPTR                 154     /* hide pointer */
#define ESPTR                 155     /* show pointer */
#define ERPTR                 156     /* read current pointer position */
#define EWPTR                 157     /* write current pointer position */
#define ERPTRS                158     /* read pointer structure */
#define ERPTRSD               159     /* read pointer data */

#define ERSTHRESH             160     /* read click space threshold */
#define EWSTHRESH             161     /* write click space threshold */
#define ERTTHRESH             162     /* read click time threshold */
#define EWTTHRESH             163     /* write click time threshold */

#define EGSIGDEV              164     /* Get device that wos signalled! */
#define EGETEVNT              165     /* get event */
#define ECHEVNT               166     /* check event */
#define EFLEVNT		      167     /* flush event */
#define EPEVNT 		      168     /* post event */
#define EREMASK		      169     /* read event mask */
#define EWEMASK		      170     /* write event mask */
#define EGDFONT               171     /* read deafult font state */
#define EGDCAR                172     /* read default caret state */
#define EFHENQ                173     /* search for next handle of type */
#define ESHUFFLE              174     /* Shuffle windows */
#define EFRONTDEV             175     /* Find minor dev of front window */

   /* Other constants */

#define BITMAPSIZE             32     /* byte size of 16 x 16 bit bitmap */

#define EINITW                256      /* InitWindow Error */



/*                                                                *************
                                                                  *           *
                                                                  * OSTRUCT   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct  
{  /* Structure passed in ioctl call                 */
   int         route;          /* Where the terminal is       */
   rectangle   boundsrect;     /* Position and size of window */
   int         type;           /* Type of window              */
   point       zoompnt;        /* Point to zoom to/from       */
} 
ostruct;


/*                                                                *************
                                                                  *           *
                                                                  * SSTRUCT   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct  
{
   int  sigtype;
   word sigmask;
} 
sstruct;


/*                                                                *************
                                                                  *           *
                                                                  * ESTRUCT   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct 
{
   word    eventtype;
   Dword   message;
}
epstruct;

/*                                                                *************
                                                                  *           *
                                                                  * PNTARG    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct 
{
      int   res;
      point pnt;
} 
pntarg;

/*                                                                *************
                                                                  *           *
                                                                  *  MWINARG  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct 
{
      int   res;
      point pnt1;
      point pnt2;
} 
mwinarg;

/*                                                                *************
                                                                  *           *
                                                                  * PTRARG    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct 
{
      int     res;
      addrptr ptr;
} 
ptrarg;

/*                                                                *************
                                                                  *           *
                                                                  * PALSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct 
{
      int     no;
      byte    val [16];
} 
palstruct;

/*                                                                *************
                                                                  *           *
                                                                  * DELSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct 
{
      int     no;
      int     val;
} 
delstruct;

#ifdef MENUINCLUDED

/*                                                                *************
                                                                  *           *
                                                                  * MSTRUCT   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| used in NewMenu                                                             |
-------------------------------------------------------------------------------
*/
typedef struct
{
   int 	   res;
   SMenuRec *menuptr;
}
mstruct;


/*                                                                *************
                                                                  *           *
                                                                  * IMSTRUCT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| used in InsMenu                                                             |
-------------------------------------------------------------------------------
*/
typedef struct 
{
   int  res;
   word insbefore;
   int  handle;
}
imstruct;
 
/*                                                                *************
                                                                  *           *
                                                                  * EISTRUCT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| used by EnItem and DisItem                                                  |
-------------------------------------------------------------------------------
*/
typedef struct
{
   int  handle;
   word itemid;
}
eistruct;

/*                                                                *************
                                                                  *           *
                                                                  * AMSTRUCT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| used by AppendItem                                                          |
-------------------------------------------------------------------------------
*/
typedef struct {

   int      res;
   int      handle;   
   char     entry [MAX_CHARS];
} 
aistruct; 

/*                                                                *************
                                                                  *           *
                                                                  * ALTSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| used by AlterItem                                                           |
-------------------------------------------------------------------------------
*/
typedef struct {

   int      res;
   int      handle;
   word     itemid;
   char     entry [MAX_CHARS];
}
altstruct;
#endif MENUINCLUDED

/*                                                                *************
                                                                  *           *
                                                                  *  KDSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Passes a soft key definition                                                |
-------------------------------------------------------------------------------
*/
typedef struct {

   int   res;
   short keyno;
   byte  length;
   char *chptr;
}
kdstruct;


/*                                                                *************
                                                                  *           *
                                                                  *  WTSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Passes a window title string                                                |
-------------------------------------------------------------------------------
*/
typedef struct {

   int   res;
   byte  length;
   char *chptr;
}
wtstruct;

/*                                                                *************
                                                                  *           *
                                                                  * STDSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Structure used to read various things                                       |
-------------------------------------------------------------------------------
*/
typedef struct
{
   int  res;
   int  handle;
   addrptr ptr;
}
stdstruct;

/*                                                                *************
                                                                  *           *
                                                                  * ENQSTRUCT *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int  res;
   int  handle;
}
enqstruct;

typedef struct
{
   int       penhandle;     /* Current pen form handle */
   rectangle penloc;        /* Local pen location and size */
   byte      penvis;        /* Pen up/down */
   byte      penmode;       /* Pen combination mode */
}
upenstate;

typedef struct
{
   int       trackhandle;     /* Current tracking form handle */
   rectsize  tracksize;       /* Local track pen size */
   int       trackphandle;    /* tracking pattern handle */
   byte      trackmode;       /* Tracking pen combination mode */
}
utrackstate;

typedef struct
{
   int       fonthandle;     /* text font handle */
   byte      txface;         /* text style */
   byte      txmode;         /* text mode */
}
utextstate;

typedef struct
{
   int       bkpathandle;     /* background pattern handle */
   int       fpathandle;      /* fill pattern handle */
}
upatstate;


typedef struct
{
   point     caretpos;
   rectangle caretrect;
   short     caretlevel;
   int       cthandle;
}
ucarstate;

typedef struct
{
   rectangle editarea;
   word      edbits;
}
uedstate;

typedef struct
{
   rectangle framearea;
   rectangle visarea;
   rectangle cboundrect;
   point     zoompnt;
   short     winwidmin, winwidmax, winheimin, winheimax;
}
uwareastate;

#ifdef WINDOWINCLUDED
typedef struct
{
   int       type;
   word      open;
   char name [NAMESIZE+1];
   byte      hilited;
}
uwmodstate;
#endif WINDOWINCLUDED


typedef struct
{
   int res;
   int handle;
   addrptr data;
   addrptr mask;
}
pdstruct;

typedef struct
{
   int res;
   int handle;
   addrptr glyphs;
   addrptr strike;
   addrptr ow;
}
fdstruct;

typedef struct
{
   int  carettype;
   int  cttone;
   byte caretmark;
   byte caretspace;
}
defcar;

typedef struct
{
   int res;
   int type;
   int starth;
}
fenqstruct;

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
/*                                                                *************
                                                                  *           *
                                                                  * MMIIOCTL  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|  Union of structures used by eioctl                                         |
-------------------------------------------------------------------------------
*/

typedef union {

   pntarg      argmt1;   /* used by several calls */
   ptrarg      argmt2;   /* used by several calls */
   palstruct   argmt3;   /* used by palette handling calls */
   delstruct   argmt4;   /* used by screen flash handling calls */
   imstruct    argmt5;   /* used by insertmenu call */
   eistruct    argmt6;   /* used by enable/disable menu item call */
   aistruct    argmt7;   /* used by append menu call */
   ostruct     argmt8;   /* used by init window call */
   kdstruct    argmt9;   /* used by define softkey call */
   EventRec    argmt10;  /* used by check/get event calls */
   epstruct    argmt11;  /* used by post event call */
   sstruct     argmt12;  /* used by signal event call */
   rectangle   argmt13;  /* used by chcbounds call */
   mwinarg     argmt14;  /* used by drag and grow calls */
   wtstruct    argmt15;  /* used by the set title ioctl call */
   stdstruct   argmt16;  /* used by various info calls */
   enqstruct   argmt17;  /* used by read type call */
   trckstruct  argmt18;  /* used by track icon call */
   altstruct   argmt19;  /* used by alter menu item call */
   dragstruct  argmt20;  /* used by dragging routines */
   upenstate   argmt21;
   utextstate  argmt22;
   utrackstate argmt23;
   upatstate   argmt24;
   ucarstate   argmt26;
   uedstate    argmt27;
   uwareastate argmt28;
   uwmodstate  argmt29;
   fdstruct    argmt30; /* used in read font data call */
   pdstruct    argmt31; /* used in read cursor data call */
   fenqstruct  argmt32;
   defcar      argmt33; /* used in get/set default caret state */
} mmiioctl ;

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

