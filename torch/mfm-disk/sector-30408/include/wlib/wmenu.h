/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Menu User Include File                    *
 *                                                                            *
 * COPYRIGHT : (c) 1986 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) wmenu.h 1.1@(#)" */

#include <mmi/menu.h>

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

