/******************************************************************************
 *                                                                            *
 * An MMI definitions file for handle tables                                  *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) handle.h 1.1@(#)" */

      /* type definitions */

#define FONTHANDLE       1
#define FORMHANDLE       2
#define BITMAPHANDLE     3
#define POINTERHANDLE    4
#define MENUHANDLE       5
#define ICONHANDLE       6
#define NULLHANDLE     255

#define LOCKED           0x80
#define UNLOCKED         0

#define MAXHANDLES            256     /* maximum no. of handles in bitmapht */ 
#define TABLEFULL              -1     /* handle can never be negative so this
                                         indicates that table is full        */
#define ARGERROR	       -1     /* error in argument value passed */

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
/*                                                                *************
                                                                  *           *
                                                                  * EDITHANDLE*
                                                                  *   TABLE   *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct
{
   char    **han;	/* the object itself */
   short   use;         /* How many times the handle is used. */
   byte    type;	/* type of object ie form, bitmap, etc */
   char    id;		/* window i.d. */
}
ehndtab;

#define FONT             ((font**) editht[handle].han)
#define FORM             ((form**) editht[handle].han)
#define CURSOR           ((cursor**) editht[handle].han)
#define ICON             ((icon**) editht[handle].han)

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

