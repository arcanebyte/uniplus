/* SCCS identification "@(#) sema.h 1.1@(#)" */

#ifdef KERNEL /* =========== START OF KERNEL ONLY SECTION ============ */

#define PROCFLAG	(1<<0)	/* Mask indicating PROC semaphore */
#define MSEFLAG		(1<<1)	/* Mask indicating Mouse moving   */
#define CARFLAG		(1<<2)	/* Mask indicating Caret Flashing */
#define SYSFLAG		(1<<3)	/* Mask indicating system print   */
#define TIMERFLAG	(1<<4)	/* Mask indicating system print   */
#define ALLFLAGS	0xff	/* Mask indicating all above      */

#endif KERNEL /* ============= END OF KERNEL ONLY SECTION ============== */

