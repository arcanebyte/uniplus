


            /********************************************\
            *                                            *
            *  XXX MMI  - Definitions file               *
            *                                            *
            *  (C) 1984 Paul Andrews                     *
            *                                            *
            \********************************************/

/* SCCS identification "@(#) term.h 1.1@(#)" */

/*                                                                *************
                                                                  *           *
                                                                  * CONSTANTS *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| constant definitions                                                        |
-------------------------------------------------------------------------------
*/

#define DESKDEV     99

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

typedef struct
{
   struct tty    ttw;         /* a TTY structure */
   V_term        *windowptr;  /* Pointer to a virtual terminal */
}
ttystruct;

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

