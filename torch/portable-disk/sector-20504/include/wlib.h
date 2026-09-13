/* Include file for programs which wish to use XXX Window Library
 *
 * (C) C.Manning, TORCH Computers Ltd. 1985 
 */

#include <mmi/mmi.h>
#include <mmi/icon.h>
#include <stdio.h>
#include <mmi/event.h>
#include <mmi/edit.h>
#include <mmi/menu.h>
#include <mmi/zoom.h>
#include <mmi/window.h>
#include <mmi/eioctl.h>
#include <mmi/mouse.h>


#define WERR_INVAL 1     /* Invalid call to WOpen or WClose */
#define WERR_OPFAIL 2    /* Open failed in WOpen */
#define WERR_IWINFAIL 3  /* InitWindow () failed in WOpen */
#define WERR_FOPFAIL 4   /* fopen () faile in WOpen */
#define WERR_ALLOC   5   /* malloc failed in WOpen */
#define WERR_NODEV   6   /* failed to find free window in WOpen */

#define WAEDRAG     1    /* perform auto dragging */
#define WAEGROW     2    /* perform auto growwing */

/* values for WOpens parameter wo_flags    */

#define WONONUNIQUE  0   /* This window - regardless of it's current state */
#define WOANYUNIQUE  1   /* any window - mus be unique    */
#define WOTHISUNIQUE 2   /* this window - fail of already open */
#define WOTTYONLY    (1<<15)	/* bit to signify only open the tty */

#define NUMPROCS   17     /* number of event procedures */

#define STDFD      1     /* fd for standard output */
   /* Parameters and return codes for GetClick */

#define SCLICKONLY   1   /* Single click only */
#define DCLICKONLY   2   /* double click only */
#define DORSCLICK    3   /* either */

#define NOCLICK      0   /* no click of required type */
#define LSINGLE      1   /* single click of left button */
#define RSINGLE      2   /* single click of right button */
#define LDOUBLE      3   /* double click of left button */
#define RDOUBLE      4   /* work this one out for yourself */
#define LCLICKANDDOWN 5  /* single click + mouse down - no second click */
#define RCLICKANDDOWN 6  /* right button version of above */

   /* Parameters for Bold(), Italic() etc.  */

#define OFF     0        /* turn style off */
#define ON      1        /* turn style on */
#define TOGGLE  2        /* toggle */

/*                                                                *************
                                                                  *           *
                                                                  * UMENUREC  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| User Menu Record                                                            |
-------------------------------------------------------------------------------
*/
typedef struct {

   word     menuid;
   char     menuname [MAX_CHARS];
   unsigned enableflags;
   unsigned checkflags;
   char     *items [MAX_ENTRIES];

} UMenuRec;

/*                                                            *****************
                                                              *               *
                                                              * ULIMSTATE     *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| used for reading and writing the limits on width and height of a window.    |
-------------------------------------------------------------------------------
*/

typedef struct
{
   short winwidmin;
   short winwidmax;
   short winheimin;
   short winheimax;
} ulimstate;

#include <mmi/WL.CTRL.h>


