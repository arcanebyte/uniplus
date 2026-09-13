/******************************************************************************
 *                                                                            *
 * TITLE     : XXX DDA Structure Definitions                                  *
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 * CREATED   : 16th August 1985                                               *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) dda.h 1.1@(#)" */

/*                                                            *****************
                                                              *               *
                                                              * VEDGESTRUCT   *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Defines all the parameters necessary for a 0th order DDA for vertical lines.|
-------------------------------------------------------------------------------
*/

typedef struct
{
   short dummy;
} vedgestruct;

/*                                                            *****************
                                                              *               *
                                                              * LEDGESTRUCT   *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Defines all the parameters necessary for a 1st order DDA for straight lines.|
-------------------------------------------------------------------------------
*/

typedef struct
{
   byte agtb;
   byte rubbish;
   short minxsteps;
   int error;
   int xderror;
   int yderror;
   int ynxderror;
} ledgestruct;

/*                                                            *****************
                                                              *               *
                                                              * AEDGESTRUCT   *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Defines all the parameters necessary for a 2nd order DDA for arc segments.  |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int asq;
   int bsq;
   short x45;
   short y45;
   lint error;
   lint xderror;
   lint yderror;
   lint xdderror;
   lint ydderror;
} aedgestruct;

/*                                                            *****************
                                                              *               *
                                                              * EDGESTRUCT    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Defines all the parameters necessary for a 2nd order DDA for arc segments.  |
-------------------------------------------------------------------------------
*/

typedef struct
{
   point origin;
   short xdir;
   short ydir;
   short a;
   short b;
   union
   {
      vedgestruct vline;
      ledgestruct line;
      aedgestruct arc;
   } dda;
   int (*FindLimitsFn)();
   short (*XCoordFn)();
   short (*MinYCoordFn)();
   short (*MaxYCoordFn)();
   int (*StartFn)();
   short (*StartXCoordFn)();
   int (*StepFn)();
   int (*MskShiftFn)();
} edgestruct;

#define UP            (-2)           /* upward y coordinate step */
#define DOWN          2              /* downward y coordinate step */
#define LEFT          (-2)           /* left x coordinate step */
#define RIGHT         2              /* right x coordinate step */

#define FTODDA(z,e)   {z = (z - e->origin.z) << 1;}
#define DDATOF(z,e)   {z = (z >> 1) + e->origin.z;}
                                     /* NB: >> truncates towards -ve infinity */

#define DDATOFN(z,e,c) {if (e->c) z = 1 - z; DDATOF(z,e)}
                                     /* 1 - z to truncate towards 0 */
#define FTODDAN(z,e,c) {FTODDA(z,e) if (e->c) z = -z;}

#ifndef MIN
#define MIN(x,y)      (((x) > (y)) ? (y) : (x))  /* Minimum of two values */
#endif
#ifndef MAX
#define MAX(x,y)      (((x) < (y)) ? (y) : (x))  /* Maximum of two values */
#endif

#define INRANGE(a, l, h) (((a) < (l)) ? l : MIN(a,h))
                /* returns a if l <= a <= h else l if a < l else h if a > h */

#ifndef ABS
#define ABS(x)        (((x) < 0) ? -(x) : (x))   /* Absolute (+ve) value */
#endif


