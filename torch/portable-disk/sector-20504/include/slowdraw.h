

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

   /* screen transfer modes */

#define SC_COPY      3
#define SC_OR        7
#define SC_XOR       6
#define SC_NOT       12

   /* Data Formats */

#define PLANESI      0
#define PLANESS      1 

   /* Pen size */

#define PHEIGHT      1
#define PWIDTH       2


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
                                                                  * RECTANGLE *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| The rectangle area includes tx & ly but not bx & ry                         |
-------------------------------------------------------------------------------
*/

typedef struct       /* rectangle */
{
   short x, y, width, height;       /* Top left origin and size in pixels */
}
rectangle;

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
   struct Ffudge *obscuring;   /* A POINTER to the obscuring Layer */
}
DivList;

/*                                                                *************
                                                                  *           *
                                                                  *   FORM    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct Ffudge      /* bitmap */
{
   addrptr     *bits;                /* Pointer to bitmap (form.bits) */
   short       x, y;                /* This is really a rectangle!   */
   short       width, height;       /* Pixel width and height        */
   short       rowwords;            /* Word width of one plane       */
   short       format;              /* Internal representation type  */
   short       planes;              /* No. of colour planes          */
   DivList     **formlist;          /* Handle to list of other forms */
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
}
font;

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
   form        portbits;               /* A form               */
   rectangle   portframe;              /* Clipping rectangle   */
   addrptr     *bkpat;                 /* Background half-tone */
   addrptr     *fillpat;               /* Shading half-tone    */

   form        **penform;              /* Handle to pen form   */
   rectangle   penloc;                 /* Pen position & size  */
   byte        penvis;                 /* Pen visibility       */
   byte        penmode;                /* Combination mode     */

   font        **txfont;               /* Handle to  font structure  */
   byte        txface;                 /* Bold, italic etc...  */
   byte        txmode;                 /* Combination mode     */

   ctrans      fbcol;                  /* Foreground/Background colour */
}
grafport;
