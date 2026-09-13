#define ZOOMINCLUDED

/******************************************************************************
 *                                                                            *
 * An MMI header file for tracking/zooming shapes                             *
 *                                                                            *
 ******************************************************************************/

#define FREEDRAG	0       /* Do an unfettered drag */
#define GROWING		1       /* Do a grow */
#define HORIZDRAG	2       /* Horizontal motion only */
#define VERTDRAG	3       /* Vertical motion only */
#define MVTYPE          3       /* Movement type mask */

#define PORTDRAG	4       /* Do the drag in a particular grafport */
#define NOFLASH         8       /* Use a steady drag */
#define TWANG           0x10    /* Do a twang */
#define MUPDATE		0x20    /* Send mouse update events as well */

#define RUBBERRECT      0x40    /* Rubber band the vectors */
#define RUBBERLINE      0x80    /* Rubber band a line */
#define RUBBERBAND      0xc0    /* */

      /* line type constants */

#define HORIZONTAL      0
#define VERTICAL        1
#define FREELINE        2

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

#define ZS              32      /* No. of zooming rectangle steps    */
#define ZOOMBITS        0xfeeedaaaa

#define TS               8      /* number of twang steps */
#define TS2             16      /* number of twang steps *2 */

#define FLASHCNT	2 	/* Counter values to determine calling */
#define MARKCNT		1	/* frequency of DoDrag () and DoGrow ()*/
#define DRAGGING	0

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

/*                                                                *************
                                                                  *           *
                                                                  * STRUCTURES*
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct
{
   point origin;
   short length;
   byte  type;             /* type of vector */
}
trackvec;

typedef struct
{
   word      elno;         /* number of elements in list */
   rectangle boundsrect;   /* Boundary rectangle */
   word      type;         /* Flashing etc, 0 = flashing */
   trackvec  veclist [1];  /* list of vectors */
}
trackdef;

