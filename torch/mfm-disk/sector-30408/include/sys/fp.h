/*
static char SCCSID[] = "@(#)fp.h	2.1	";
*/
/*
 *	parameters of the implementation
 */
#define EXPSIZE 8
#define EXPOFFSET ((1 << (EXPSIZE - 1)) - 2)
#define INFEXP ((1 << EXPSIZE) - 1)
#define MAXEXP ((1 << EXPSIZE) - 2)
#define FRACSIZE 23
#define HIDDENBIT 0x00800000L
#define CARRYBIT (HIDDENBIT << (GBITS + 1))
#define GBITS 2
#define NORMMASK 0xfe000000L

/*
 *	macros to pick pieces out of long integers
 *	this must change if FRACSIZE changes
 *	presently we assume two 12-bit pieces
 */
#define CHUNK 12
#define lmul(x,y) ((long) (x) * (long) (y))
#define lo(x) ((x) & 0xfff)
#define hi(x) ((x) >> CHUNK)
#define hibit(x) (((short)(x) >> (CHUNK - 1)) & 1)

struct fpnum {
	unsigned sign:1;
	unsigned exp:EXPSIZE;
	long unsigned frac:FRACSIZE;
};

typedef struct fpnum fp;

static fp zero;
static fp infinity = {0, ~0, 0};

fp fladd(), flsub(), flmul(), fldiv(), flneg(), fp_addmag(), fp_submag();
fp itof(), uitof(), ltof(), ultof();
