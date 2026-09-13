


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

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

      /* Masks for kstat */

#define MMASK      0x3000      /* the mouse bits in kstat */
#define MLMASK     0x1000      /* the left mouse bit      */
#define MRMASK     0x2000      /* the right mouse bit     */

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

/*                                                                *************
                                                                  *           *
                                                                  *  CURSOR   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct       /* cursor */
{
   form data, mask;
   point hotspot;
}
cursor;

