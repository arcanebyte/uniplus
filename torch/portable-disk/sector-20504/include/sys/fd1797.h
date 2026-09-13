/* special ioctl commands for floppy disk formatting */
#define URESTORE	50	/* restore */
#define USTEPIN		51	/* step in one track */
#define UFMTTRK		52	/* format one track */
#define MOTOROFF	53	/* turn the motor off */

/*
 * minor device assignments
 *	bits	use
 *	0-3	physical unit number
 *		  0,1	drive number
 *		  2,3	controller number
 *	4	1=5 1/4" drive, 0=8" drive
 *	5	1=single density, 0=double density
 *	6	1=40 track, 0=80 track
 */

#define fdunit(d) (minor(d)&0xF)	/* physical unit number, 0-F */
#	define controller(d) (((d)>>2)&3)	/* controller number, 0-3 */
#	define drive(d) ((d)&3)		/* drive number, 0-3 */
#define fddtype(d) (minor(d)&0x10)	/* drive type */
#define fddensity(d) (minor(d)&0x20)	/* density */
#define fd40(d)	(minor(d)&0x40)		/* 40/80 track on 5 1/4 */
