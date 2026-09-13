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

   /* buffer sizes */

#define K_BUFFSIZE   32

   /* key modification masks */

#define CTRLK        0x100
#define ACTK         0x200
#define DK           0x200
#define SHFTK        0x400
#define CAPSDOWN     0x800
#define CAPSUP       0xf7ff
#define CAPSLK       0x800

   /* 16 bit keyboard code masks */

#define FNMASK       0x4000

   /* Some constants */

#define F1           FNMASK
#define F2           (FNMASK | 1)
#define F3           (FNMASK | 2)
#define F4           (FNMASK | 3)
#define F5           (FNMASK | 4)
#define F6           (FNMASK | 5)
#define F7           (FNMASK | 6)
#define F8           (FNMASK | 7)
#define F9           (FNMASK | 8)
#define F10          (FNMASK | 9)
#define HELPK        (FNMASK | 10)

#define MAXFNS       11

   /* Some more constants */

#define ESCSEQ       0x1000
#define PTRUK        0x100b   /* Top half will be ignored on output */
#define PTRDK        0x101f
#define PTRLK        0x1008
#define PTRRK        0x101d

#define DELK         0x7f
#define TABK         9
#define CULK         8
#define CURK         29
#define CUDK         31
#define CUPK         11
#define CRK          13
#define NULLK        0

   /* A few locations */


#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

static byte *ktailptr  = &((byte*)0) [0x1f0];
static byte *kheadptr  = &((byte*)0) [0x1f1];
static word *kbufaddr  = (word*) (&((byte*)0) [0x1f2]);

static byte *stailptr  = &((byte*)0) [0x2f0];
static byte *sheadptr  = &((byte*)0) [0x2f1];
static word *sbufaddr  = (word*) (&((byte*)0) [0x2f2]);

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */
