


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

#define DESKDEV     99

typedef struct
{
   struct tty    ttw;         /* a TTY structure */
   V_term        *windowptr;  /* Pointer to a virtual terminal */
}
ttystruct;
