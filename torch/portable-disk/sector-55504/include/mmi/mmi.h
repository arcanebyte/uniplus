

            /********************************************\
            *                                            *
            *  XXX MMI  - Definitions file               *
            *                                            *
            *  (C) 1984 Paul Andrews                     *
            *                                            *
            \********************************************/

/* SCCS identification "@(#) mmi.h 1.2@(#)" */

/*                                                                *************
                                                                  *           *
                                                                  * CONSTANTS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| constant definitions                                                        |
-------------------------------------------------------------------------------
*/

   /* screen transfer modes */

#define M_ZEROS        0
#define M_SANDD	       1
#define M_SANDND       2
#define M_COPY         3
#define M_NSANDD       4
#define M_DST          5
#define M_XOR          6
#define M_OR           7
#define M_NSANDND      8
#define M_NSXORD       9
#define M_NDST        10
#define M_SORND       11
#define M_NOT         12
#define M_NSORD       13
#define M_NSORND      14
#define M_ONES        15

#define FNAMESIZE   12

   /* Font handle number */

#define   SYSFONT       4

   /* Halftone handle numbers */

#define   NULLBITS      0
#define   BITS_0        5
#define   BITS_8_2      6
#define   BITS_10_5     7
#define   BITS_13_7     8
#define   BITS_15       9

#define   BITS_12_0     10
#define   BITS_12_3     11
#define   BITS_15_3     12
#define   BITS_14_11    13
#define   BITS_4_1      14
#define   BITS_5        15
#define   BITS_6_9      16
#define   BITS_10       17
#define   BITS_15_0     18

#define   TRACKBITS     19

#define   BITS_1        20
#define   BITS_2        21
#define   BITS_3        22
#define   BITS_4        23
#define   BITS_6        24
#define   BITS_7        25
#define   BITS_8        26
#define   BITS_9        27
#define   BITS_11       28
#define   BITS_12       29
#define   BITS_13       30
#define   BITS_14       31

/* constant definitions for form handles (arrows for scroll bars etc) */

#define	FIRSTSYM	32	/* first available handle for symbol forms */

#define	LEFTSYM(n)	((n)*5 + FIRSTSYM + 0)
#define	RIGHTSYM(n)	((n)*5 + FIRSTSYM + 1)
#define	UPSYM(n)	((n)*5 + FIRSTSYM + 2)
#define	DOWNSYM(n)	((n)*5 + FIRSTSYM + 3)
#define	GROWSYM(n)	((n)*5 + FIRSTSYM + 4)

#define	MAXFORMS	((MAXMODES+1)*5)


#define MAXMODES                5     /* number of screen modes */

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

#define MASKINTS spl7		/* Turn off interrupts */

#define ERRBUFFSIZE	512	/* system error message buffer size */

   /* Data Formats */

#define PLANESI      0
#define PLANESS      1 

   /* Pen size */

#define PHEIGHT      1
#define PWIDTH       2

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

/*                                                                *************
                                                                  *           *
                                                                  *  TYPEDEF  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Simple types                                                                |
-------------------------------------------------------------------------------
*/

typedef unsigned char byte;
typedef char boolean;
typedef unsigned short word;
typedef unsigned long  Dword;
typedef word  *addrptr;

/*                                                                *************
                                                                  *           *
                                                                  *   POINT   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Complex types, point                                                        |
-------------------------------------------------------------------------------
*/

typedef struct       /* co-ordinate point */
{
   short x, y;
}
point;

/*                                                                *************
                                                                  *           *
                                                                  * PATTERN   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Halftone                                                                    |
-------------------------------------------------------------------------------
*/

typedef word pattern[16];

/*                                                                *************
                                                                  *           *
                                                                  *  CTRANS   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Foreground & Background colours                                             |
-------------------------------------------------------------------------------
*/

typedef struct      /* ctrans */
{
   byte   foreground, background;
}
ctrans;

/*                                                                *************
                                                                  *           *
                                                                  *   SIZE    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   short width, height;
}
rectsize;

/*                                                                *************
                                                                  *           *
                                                                  * RECTANGLE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct       /* rectangle */
{
   short x, y, width, height;       /* Top left origin and size in pixels */
}
rectangle;

/*                                                                *************
                                                                  *           *
                                                                  * KBDMATRIX *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| The four keyboard transformation matrix tables passed in ioctl calls.       |
-------------------------------------------------------------------------------
*/

typedef short kbdmatrix[4][128];

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

/*                                                                *************
                                                                  *           *
                                                                  *  DIVLIST  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| A list of divided rectangles                                                |
-------------------------------------------------------------------------------
*/
typedef struct Dfudge
{
   addrptr     *bits;           /* A handle to a bitmap */
   short       x, y;
   short       width, height;
   short       rowwords;
   short       format;
   short       planes;
   struct Dfudge **formlist;   /* Should be null, i.e. a bitmap! */
   rectangle   boundsrect;     /* The bit we're interested in */
   short       visflag;        /* 1=visible, 0=obscured */
   struct Dfudge **next;       /* A handle to another DivList */
   struct Dfudge **previous;   /* A handle to another DivList */
   struct Hfudge *obscuring;   /* A POINTER to the obscuring Layer */
}
DivList;

/*                                                                *************
                                                                  *           *
                                                                  *   HFORM   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| This is a subset of a divlist which acts as a header pointer to a form list |
-------------------------------------------------------------------------------
*/

typedef struct Hfudge      /* bitmap */
{
   addrptr     *bits;               /* Pointer to bitmap (form.bits) */
   short       x, y;                /* This is really a rectangle!   */
   short       width, height;       /* Pixel width and height        */
   short       rowwords;            /* Word width of one plane       */
   short       format;              /* Internal representation type  */
   short       planes;              /* No. of colour planes          */
   DivList     **formlist;          /* Handle to list of other forms */
   rectangle   boundsrect;          /* Only used when header to DivList */
}
hform;
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

/*                                                                *************
                                                                  *           *
                                                                  *   FORM    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| This is a subset of a divlist. 'formlist' must always be null, as a form    |
| cannot be further sub-divided.                                              |
-------------------------------------------------------------------------------
*/

typedef struct Ffudge      /* bitmap */
{
   addrptr     *bits;               /* Pointer to bitmap (form.bits) */
   short       x, y;                /* This is really a rectangle!   */
   short       width, height;       /* Pixel width and height        */
   short       rowwords;            /* Word width of one plane       */
   short       format;              /* Internal representation type  */
   short       planes;              /* No. of colour planes          */
#ifdef KERNEL  /* USER PROGRAM DONT NEED TO KNOW ABOUT DIVLIST'S ... */
   DivList     **formlist;          /* Handle to list of other forms */
#else	       /* BUT THEY SHOULD STILL INCLUDE A NULL FORMLIST FIELD */
   int	       *formlist;
#endif KERNEL
}
form;

/*                                                                *************
                                                                  *           *
                                                                  *CopyStruct *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   form        *sourceform;
   form        *destform;
   addrptr     htoneform;
   rectangle   *srcrect;
   rectangle   *destrect;
   rectangle   *cliprect;
   byte        mode;
   byte        rubbish;
   ctrans      *cindex;
}
CopyStruct;

/*                                                                *************
                                                                  *           *
                                                                  *   FONT    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct       /* font */
{
   word         fonttype;          /* Fixed, proportional etc. */
   short        firstchar;         /* Ascii code of first char */
   short        lastchar;          /* Ascii code of last  char */
   short        ascent;            /* Extent above base line   */
   short        descent;           /* Extent below base line   */
   short        maxwidth;          /* Maximum character width  */
   short        leading;           /* Distance to next line    */
   form         glyphs;            /* Form structure           */
   addrptr      *strike;           /* Handle to position in glyphs     */
   addrptr      *ow;               /* Offset/width table (-1 = no char) */
   char         fname[FNAMESIZE];  /* Font name field */
   form         scratform;         /* For the scratch area     */
}
font;

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

/*                                                                *************
                                                                  *           *
                                                                  * GRAFPORT  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int         stream;                 /* e.g. screen, network */
   hform       portbits;               /* A formlist header    */

   rectangle   portframe;              /* Clipping rectangle   */
   rectangle   innerclip;              /* rectangle in portframe to draw to */
   rectangle   originnerclip;          /* Local version of above; unclipped */

   addrptr     *bkpat;                 /* Background half-tone */
   addrptr     *fillpat;               /* Shading half-tone    */

   form        **penform;              /* Handle to pen form   */
   rectangle   penloc;                 /* Pen position & size  */
   byte        penvis;                 /* Pen visibility       */
   byte        penmode;                /* Combination mode     */

   font        **txfont;               /* Handle to  font structure  */
   byte        txface;                 /* Bold, italic etc...  */
   byte        txmode;                 /* Combination mode     */

   form        **trackform;            /* Handle to track form */
   rectsize    tracksize;              /* Track pen size       */
   addrptr     *trackpat;              /* Shading half-tone    */
   byte        trackmode;              /* Combination mode     */

   ctrans      fbcol;                  /* Foreground/Background colour */
}
grafport;
#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

typedef struct
{
   form        **penform;              /* Handle to pen form   */
   rectangle   penloc;                 /* Pen position & size  */
   byte        penvis;                 /* Pen visibility       */
   byte        penmode;                /* Combination mode     */
}
penstate;

typedef struct
{
   font        **txfont;               /* Handle to  font structure  */
   byte        txface;                 /* Bold, italic etc...  */
   byte        txmode;                 /* Combination mode     */
}
textstate;

typedef struct
{
   form        **trackform;            /* Handle to track form */
   rectsize    tracksize;              /* Track pen size       */
   addrptr     *trackpat;              /* Shading half-tone    */
   byte        trackmode;              /* Combination mode     */
}
trackstate;



