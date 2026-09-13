#define MENUINCLUDED

/*****************************************************************************\
 *                                                                           *
 *  Menu structures definition file                                          *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *  Update history:                                                          *
 *                                                                           *
 *  WHAT                                      BY WHOM     WHEN               *
 * -------------------------------------------|------|-----------------      *
 * Changed MenuRec into UMenuRec & SMenuRec   |  JE  | 31-May-85             *
 * so that menus are variable in length       |                              *
 *                                                                           *
\*****************************************************************************/

/* SCCS identification "@(#) menu.h 1.1@(#)" */

#define MAX_CHARS   20
#define MAX_ENTRIES 30
#define MAX_MENUS   11
#define MBARHEIGHT  13
#define SYSMENUWID  6           /* char width of system menu inc spaces */

#define RJUST	    3		/* character to right justify text in a menu */
#define BLANKITEM   4		/* blank line in a menu */
#define DIVITEM     5		/* dividing line character in a menu */


/*                                                                *************
                                                                  *           *
                                                                  * SMENUREC  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| System Menu Record                                                          |
-------------------------------------------------------------------------------
*/
typedef struct {

   word     menuid;
   char     menuname [MAX_CHARS];
   short    menuwidth;
   short    menuheight;
   unsigned enableflags;
   unsigned checkflags;
   word     itemoff [MAX_ENTRIES];  /* offsets into the following string */
   word     itemdlen;		    /* the length of the following */
   char     **itemdata;             /* the item strings all in one string */

} SMenuRec;

typedef SMenuRec *MenuPtr;

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

/*                                                                *************
                                                                  *           *
                                                                  * MENUBAR   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/
typedef struct mbar {

   MenuPtr     *menuhandle;   /* handle to a menu structure */
   word        menuid;
   struct mbar **next;

} MenuBar;

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

/*
typedef struct {

   word menuid;
   word itemnumber;

} menuselect;
 */
