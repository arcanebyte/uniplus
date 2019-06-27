/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include	"sys/types.h"
#include	"sys/param.h"
#include	"sys/inode.h"
#include	"saio.h"

nullsys()
{ ; }

extern snstrategy(), snopen(), sneject();
extern prostrategy(), proopen();
extern cvstrategy(), cvopen();
extern pmdstrategy(), pmtstrategy(), pmopen();
struct devsw devsw[] = {
	"sn", snstrategy, snopen, nullsys,
	"eject", nullsys, sneject, nullsys,
	"w", prostrategy, proopen, nullsys,
	"c", cvstrategy, cvopen, nullsys,
/*	"sunol", cvstrategy, cvopen, nullsys,*/ /* sunol not yet supported */
	"pmd", pmdstrategy, pmopen, nullsys,
	"pmt", pmtstrategy, pmopen, nullsys,
	0,0,0,0
};

extern int problk0(), snblk0(), pmblk0();
struct block0 block0[] = {	/* special zero blocks for booting */
	"sn", snblk0,
	"w", problk0,
	"pmd", pmblk0,
	0,0
};

extern sszdisk(), stotape(), sfrtape();
extern pmszdisk(), pmwmrk(), pmrmrk(), pmrewind(), pmerase(), pmadvance(),
	pmreten(), pmtotape(), pmfrtape();
struct streamer streamer[] = {	/* streaming tape information */
/*	"sunol", sszdisk, nullsys, nullsys, nullsys, nullsys, nullsys, nullsys,
		stotape, sfrtape,*/   /* sunol not yet supported */
/*	"pmt", pmszdisk, pmwmrk, pmrmrk, pmrewind, pmerase, pmadvance, pmreten,
		pmtotape, pmfrtape,*/
	0,0,0,0,0,0,0,0,0,0
};

char *env[] = {
	"TZ=PST8PDT",
	0
};
char	**environ = &env[0];
