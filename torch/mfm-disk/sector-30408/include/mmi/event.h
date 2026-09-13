



            /********************************************\
            *                                            *
            *  XXX MMI definitions file - EVENTS         *
            *                                            *
            *  (C) 1984 Paul Andrews                     *
            *                                            *
            \********************************************/

/* SCCS identification "@(#) event.h 1.1@(#)" */

/*                                                                *************
                                                                  *           *
                                                                  * CONSTANTS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| constant definitions                                                        |
-------------------------------------------------------------------------------
*/

      /* Event types */

#define NEVENT     0      /* Null event (nothing happened)    */
#define MLDEVENT    1     /* left mouse button has gone down  */
#define MLUEVENT    2     /* Left mouse button has gone up    */
#define KDEVENT    3      /* A key has gone down              */
#define KUEVENT    4      /* A key has gone up                */
#define AKEVENT    5      /* A key has auto-repeated          */
#define UEVENT     6      /* Windows have been moved          */
#define OBEVENT    7      /* Over bounds of rectangle         */
#define MRDEVENT   8      /* right mouse button has gone down */
#define MRUEVENT   9      /* right mouse button has gone up   */
#define DESKEVNT   10     /* Message for desktop manager      */
#define MSEVENT    11     /* Menu selection event             */

      /* Event masks */

#define NEMASK     1
#define MLDMASK     2
#define MLUMASK     4
#define KDMASK     8
#define KUMASK     0x10
#define AKMASK     0x20
#define UEMASK     0x40
#define OBMASK     0x80
#define MRDMASK    0x100
#define MRUMASK    0x200
#define DESKMASK   0x400
#define MSMASK     0x800

#define MDMASK     (MLDMASK | MRDMASK)
#define MUMASK     (MLUMASK | MRUMASK)

      /* Message masks */

#define ACTMESS    0      /* Activate event message    */
#define DEACTMESS  1      /* De-activate event message */
#define SIZEMESS   2      /* Change size event message */
#define MODEMESS   3      /* Change mode event message */
#define MOUSEMESS  4      /* Mouse update event message*/

#define UEVMSGMASK 7      /* Mask for all the above    */


      /* Miscellaneous constants */

#define ALLEVENTS  -1     /* Check for all events                 */

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
#define MAXEVENT   32     /* Size of event queue                  */
#define DEFMASK      6    /* Only mouse button changes get posted */
#define NOMASK     0      /* No events get posted                 */
#define KSIG       16     /* Kernel signal number                 */


#define DEFUTTHRESH 25    /* click time threshold for up phase = 0.5 s   */
#define DEFDTTHRESH  9    /* click time threshold for down phase = 0.2 s */
#define DEFTTHRESH ((DEFUTTHRESH<<16) | DEFDTTHRESH)

#define DEFSTHRESH  3     /* Default space threshold is 3 pixels either way */

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

/*                                                                *************
                                                                  *           *
                                                                  * EVENTREC  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct       /* EventRec */
{
   point      where;
   word       what;
   Dword      message;
   int        when;
   int        modifiers;
}
EventRec;


#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
/*                                                             ****************
                                                               *              *
                                                               * EVENTELEMENT *
                                                               *              *
                                                               ****************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

typedef struct OneElement    /* EventElement */
{
   point             where;
   word              what;
   Dword             message;
   int               when;
   int               modifiers;
   struct OneElement *previous;
   struct OneElement *next;
}
EventElement;

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */


