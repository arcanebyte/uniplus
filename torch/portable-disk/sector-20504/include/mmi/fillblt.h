/******************************************************************************
 *                                                                            *
 * TITLE     : XXX Definitions for use by cfillblt.c and carcblt.c            *
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 * CREATED   : 16th August 1985                                               *
 *                                                                            *
 * PURPOSE   :                                                                *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

/*                                                            *****************
                                                              *               *
                                                              * LINESTRUCT    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure passed to LineBits() and ArcBits() for line drawing.              |
-------------------------------------------------------------------------------
*/

typedef struct
{
   form       *destform;
   addrptr    htoneform;
   rectangle  *cliprect;
   byte       mode;
   byte       rubbish;
   rectangle  *linebounds;
   short      pwidth;
   short      pheight;
} linestruct;

/*                                                            *****************
                                                              *               *
                                                              * FILLSTRUCT    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure passed to FillBits for area filling.                              |
-------------------------------------------------------------------------------
*/

typedef struct
{
   form       *destform;
   addrptr    htoneform;
   rectangle  *destrect;
   rectangle  *cliprect;
   byte       mode;
   byte       rubbish;
   byte       leftorder;                /* ARC or LINE */
   byte       rightorder;               /* ARC or LINE */
   rectangle  *leftbounds;
   rectangle  *rightbounds;
} fillstruct;

/*                                                            *****************
                                                              *               *
                                                              * SECTSTRUCT    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure passed to SectBits().                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   form       *destform;
   addrptr    htoneform;
   rectangle  *cliprect;
   byte       mode;
   byte       rubbish;
   rectangle  *boundsrect;
   short sang;
   short eang;
} sectstruct;

/*                                                            *****************
                                                              *               *
                                                              * ARCSTRUCT     *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure passed to ArcBits().                                              |
-------------------------------------------------------------------------------
*/

typedef struct
{
   form       *destform;
   addrptr    htoneform;
   rectangle  *cliprect;
   byte       mode;
   byte       rubbish;
   rectangle  *boundsrect;
   short      sang;
   short      eang;
   short      penwidth;
   short      penheight;
} arcstruct;

/*                                                            *****************
                                                              *               *
                                                              * LIMITS        *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure containing a pair of coordinate limits.                           |
-------------------------------------------------------------------------------
*/

typedef struct
{
   short min;
   short max;
} limits;

#define LINEO         1                 /* 'order' of a line */
#define ARC           2                 /* 'order' of an elliptical arc */

#define DEGREES       360               /* number of 'degrees' in a circle */
#define SDEGREES      (DEGREES >> 1)    /*    in a semi-circle */
#define QDEGREES      (DEGREES >> 2)    /*    in a quadrant */

#define XCOORD(a, theta) (Cordic(0, a, theta) >> 1)
#define YCOORD(b, theta) (Cordic(b, 0, theta) >> 1)

#define RECT(r, a, b, w, h) {r->x = a; r->y = b; r->width = w; r->height = h; }

