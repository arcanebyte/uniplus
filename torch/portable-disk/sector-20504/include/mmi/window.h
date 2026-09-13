#define WINDOWINCLUDED


            /********************************************\
            *                                            *
            *  SLOWDRAW - Definitions file               *
            *                                            *
            *  (C) 1984 Paul Andrews                     *
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
#define NAMESIZE 20     /* Max length of a windows title */

   /* streams */

#define O_SCREEN6    1
#define O_PRINTER    2
#define I_KB         1
#define I_MOUSE      2

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

/* Other stuff */

#define MAX_WINDOWS 12
#define MAX_SYS_WIN 14     /* 2 greater than MAX_WINDOWS  */
   /* + 1 for desktop gives 15. +1 (sign) bit = 16 = 1 word!!!! */
#define HIDDEN      2      /* Not shown on screen ever    */
#define VISIBLE     1      /* Always on screen            */
#define OBSCURED    0      /* Shown on screen if at front */


#define SCALE      4
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

      /* area constants */

#define INDESK     0
#define INMENUBAR  1
#define INCONTENT  3
#define INDRAG     4
#define INGROW     5
#define INCLOSE    6
#define INSELECT   7
#define INFRAME    8

      /* Window type constants */

#define STRAWBERRY 0
#define PLAIN      1
#define VANILLA    2
#define NEOPOLITAN 3
#define WTMASK     0xf         /* part of type that gives window format */

#define CLOSEBOX   0x10        /* Draw it with a close box              */
#define MENUWIN    0x20        /* This is in fact a pull down menu      */
#define GROWBOX    0x40        /* Has a grow area                       */
#define NOZOOM	   0x80	       /* Do not zoom the window on openning */
#define PSMASK     0x0100      /* part of type that indicates a signal  */

#define SHOWDEV    0x1000      /* Show device in title bar */
#define SHOWNAME   0x2000      /* Show name in title bar */
#define SHOWPID    0x4000      /* Show pid in title bar */
#define TITLEMASK  0x7000

/* Values that wtimer may have to indicate why the timer was cancelled */

#define BADCLICK -1
#define WTIMEOUT  0
#define WSPACEOUT 1
#define WMCLICK   2



#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

#define SWMODMASK  0x7000      /* Part of type the user can modify */

/* wflags bit meanings */

#define WTIMERON  1            /* Window fine timer is running */
#define RESERVED  2            /* Reserved by find free window */

   /* Some window part dimensions */

#define TITLEHEIGHT   14
#define CBOXWIDTH     20
#define GBOXWIDTH     16
#define GBOXHEIGHT    8

/*                                                                *************
                                                                  *           *
                                                                  *  V_term   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Structure of a virtual terminal                                             |
-------------------------------------------------------------------------------
*/
typedef struct Vfudge      /* Screen drawing port */
{
   struct Vfudge *back;                /* Adjacent window in back    */
   struct Vfudge *front;               /* Adjacent window in front   */

   rectangle   visarea;                /* Displayed area of window */
   rectangle   framearea;              /* Obscuring area of window */
   rectangle   cboundrect;             /* Cursor boundsrect */

   point       zoompnt;                /* Point to zoom to/from */

   short       winwidmin;              /* Max and min size of window */
   short       winwidmax;
   short       winheimin;
   short       winheimax;

   int         type;                   /* Window type */
   word        open;                   /* open flag   */
   word        id;                     /* a unique window id */

   struct proc *uproc;                 /* Pointer to process table of opener */
   word        hsigmask;               /* Events to signal heap */
   word        sigmask;                /* Events to signal user */
   int         signo;                  /* Signal number to send */

   byte	       wflags;                 /* flags; used by window timers */
   int	       wtimer;

   char        name [NAMESIZE + 1];    /* The name of this window */
   byte        hilited;                /* Whether the title bar is hilited */
   MenuBar     **mbar;   	       /* handle to the menu bar structure */


   edit        editrec;                /* structure containing grafport, caret
					  and editing data for v_term  */ 


   struct tty  *ttyptr;                /* Unix terminal structure    */
}
V_term;


/*                                                                *************
                                                                  *           *
                                                                  *           *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Some field constants                                                        |
-------------------------------------------------------------------------------
*/

#define TTW         ttw
#define TBITS       editrec.gp.portbits
#define TLIST       editrec.gp.portbits.formlist
#define TFRAME      editrec.gp.portframe
#define TPORT       editrec.gp

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */


