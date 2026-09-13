#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */
/******************************************************************************
 *                                                                            *
 *          A Slowdraw definitions file                                       *
 *                                                                            *
 *                                     M.P.Andrews 25/2/85                    *
 *                                                                            *
 ******************************************************************************/

   /* constant definitions */

#define	PRI_HEAP	(PZERO + 1)
#define MINMEM          115         /* Leave at least 115 clicks to user */
#define MAXFREE         0x2000      /* If MAXFREE free units, contract */

   /* Definition of memory manager constructs */

typedef union mf
{
   char      *ptr;      /* Pointer to allocated block   */
   union  mf *nptr;     /* pointer to next free element */
}
mel;
#define MAXMAST   1024

typedef word ALIGN;

union header  /* TrFree block header */
{
   struct
   {
      union
      {
         int   offset;     /* offset to next TrFree block */
         mel   *mptr;      /* pointer to a master pointer */
      } m;
      Dword size;          /* Size of this TrFree block in units of HEADER */
   } s;
   ALIGN x;    /* Force alignment of blocks */
};

typedef union header HEADER;

   /* Definition of window process address space */

typedef struct
{
   int        prevaddr;     /* Address of process before last swap */
   HEADER     *baddr;       /* Pointer to first part of heap       */
   Dword      maxsize;      /* Current maximum number of units in heap */
   Dword      freeunits;    /* Number of units not allocated           */
   int        alloco;       /* Offset to start of search           */
   HEADER     base;         /* 1st part of heap                    */
}
hprocs;

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

