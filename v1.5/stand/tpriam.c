/*
 * Priam Datatower Streaming Tape Interface
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "saio.h"
#include "streamer.h"
#include "sys/cops.h"
#include "sys/local.h"
#include "sys/priam.h"

extern char pmcmd[];
extern char pmresults[];
extern struct datacopyparms dcparms;
extern char pmstate;
extern char pmxstate;
extern struct pm_base *pmhwbase;
extern struct pmfmtstat dcstat;
extern struct results reslt;
extern char	pmunit;			/* card (0,1,2) */

pmrewind(fp)
int fp;
{
	struct iob *ip = &iob[fp-3];
	int unit = ip->i_unit;

	strcpy(pmcmd, "rewind");
	return(pmioctl(unit, PMREWIND, 0));
}

pmerase(fp)
int fp;
{
	struct iob *ip = &iob[fp-3];
	int unit = ip->i_unit;

	strcpy(pmcmd, "erase");
	return(pmioctl(unit, PMERASE, 0));
}

pmreten(fp)
int fp;
{
	struct iob *ip = &iob[fp-3];
	int unit = ip->i_unit;

	strcpy(pmcmd, "retension");
	return(pmioctl(unit, PMRETEN, 0));
}

pmwmrk(fp)
int fp;
{
	struct iob *ip = &iob[fp-3];
	int unit = ip->i_unit;

	strcpy(pmcmd, "write tape mark");
	return(pmioctl(unit, PMWMRK, 0));
}

pmrmrk(fp)
int fp;
{
	struct iob *ip = &iob[fp-3];
	int unit = ip->i_unit;

	strcpy(pmcmd, "read tape mark");
	return(pmioctl(unit, PMRMRK, 0));
}

pmadvance(fp, count)
int fp, count;
{
	struct iob *ip = &iob[fp-3];
	int unit = ip->i_unit;

	strcpy(pmcmd, "advance tape mark");
	return(pmioctl(unit, PMADVANCE, count));
}

pmszdisk()
{
}

struct results *
pmtotape(dtp, dfp, dbn, comm, nblks)
int dtp, dfp; int dbn, nblks, comm;
{
	struct iob *ipf = &iob[dfp-3];
	int unitf = ipf->i_unit;
	struct iob *ipt = &iob[dtp-3];
	int unitt = ipt->i_unit;
	register struct results *rp = &reslt;
	register struct pm_base *addr;
	register struct datacopyparms *fmtp = &dcparms;
	int cmdtype, opri;

	if(unitf != unitt) {
		printf("Disk and tape MUST have same unit number\n");
		rp->errcode = -1;
		return(rp);
	}
	if(unitf >= NPM) {
		printf("pmtotape: unit number out of range\n");
		rp->errcode = -1;
		return(rp);
	}
	pmhwbase = pmaddr(unitf);
	switch(comm) {
		case ISSUE:
			addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
			if (pmwaitrf(addr)) {
				printf("pmtotape: timeout before backup\n");
				rp->errcode = -1;
				return(rp);
			}
			fmtp->pm_opcode = PMDC;
			fmtp->pm_scr1 = 0;
			fmtp->pm_sc0 = 3;	/* don't stop for errors */
			fmtp->pm_sc1 = 0x44;	/* logical addressing */
			fmtp->pm_tsize = 0;
			fmtp->pm_sdev = 0;	/* source is disk */
			fmtp->pm_stah = (dbn >> 16) & 0xFF;
			fmtp->pm_stam = (dbn >> 8) & 0xFF;
			fmtp->pm_stal = dbn & 0xFF;
			fmtp->pm_scr2 = 0;
			fmtp->pm_scr3 = 0;
			fmtp->pm_ddev = (TAPE << 4);	/* destination is tape */
			fmtp->pm_dtah = 0;
			fmtp->pm_dtam = 0;
			fmtp->pm_dtal = 0;
			addr->p0 = 0;	
			addr->p2 = 0;
			addr->p3 = sizeof(struct datacopyparms);
			addr = pmBIaddr(pmhwbase);
			cmdtype = pmstate = PMPKTXFER;
			pmxstate = ISSUE;
			pmunit = unitf;
			break;
		case PMPKTRES:
			addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
			if (pmwaitrf(addr)) {
				printf("pmtotape: timeout before resuming\n");
				rp->errcode = -1;
				return(rp);
			}
			addr->p0 = 0;
			addr->p1 = 0xFF;
			addr->p2 = 0xFF;
			addr->p3 = 0xFF;
			addr->p4 = 0;
			addr->p5 = 0;
			cmdtype = comm;
			pmxstate = RESUME;
			pmstate = PMPKTXFER;
			addr = pmBIaddr(pmhwbase);
			pmunit = unitf;
			break;
		default:
			rp->errcode = -1;
			return(rp);
	}
		opri = spl2();
		addr->cmdreg = cmdtype;
		while (pmstate != IDLING)	;
		splx(opri);
		return(rp);
}

struct results *
pmfrtape(dtp, dfp, dbn, comm, nblks)
int dtp, dfp; int dbn, nblks, comm;
{
	struct iob *ipf = &iob[dfp-3];
	int unitf = ipf->i_unit;
	struct iob *ipt = &iob[dtp-3];
	int unitt = ipt->i_unit;
	register struct results *rp = &reslt;
	register struct pm_base *addr;
	register struct datacopyparms *fmtp = &dcparms;
	int cmdtype, opri;

	if(unitf != unitt) {
		printf("Disk and tape MUST have same unit number\n");
		rp->errcode = -1;
		return(rp);
	}
	if(unitf >= NPM) {
		printf("pmtotape: unit number out of range\n");
		rp->errcode = -1;
		return(rp);
	}
	pmhwbase = pmaddr(unitf);
	switch(comm) {
		case ISSUE:
			addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
			if (pmwaitrf(addr)) {
				printf("pmfrtape: timeout before restoring\n");
				rp->errcode = -1;
				return(rp);
			}
			fmtp->pm_opcode = PMDC;
			fmtp->pm_scr1 = 0;
			fmtp->pm_sc0 = 0x44;	/* terminate at EOF,but resumable */
			fmtp->pm_sc1 = 0x40;	/* use logical addressing */
			fmtp->pm_tsize = 0;
			fmtp->pm_sdev = (TAPE << 4);	/* source is tape */
			fmtp->pm_stah = 0;
			fmtp->pm_stam = 0;
			fmtp->pm_stal = 0;
			fmtp->pm_scr2 = 0;
			fmtp->pm_scr3 = 0;
			fmtp->pm_ddev = 0;	/* destination is disk */
			fmtp->pm_dtah = (dbn >> 16) & 0xFF;
			fmtp->pm_dtam = (dbn >> 8) & 0xFF;
			fmtp->pm_dtal = dbn & 0xFF;
			addr->p0 = 0;	
			addr->p2 = 0;
			addr->p3 = sizeof(struct datacopyparms);
			addr = pmBIaddr(pmhwbase);
			cmdtype = pmstate = PMPKTXFER;
			pmxstate = ISSUE;
			pmunit = unitf;
			break;
		case PMPKTRES:
			addr = pmBWaddr(pmhwbase);	/* byte mode - waiting */
			if (pmwaitrf(addr)) {
				printf("pmfrtape: timeout before resuming\n");
				rp->errcode = -1;
				return(rp);
			}
			addr->p0 = 0;
			addr->p1 = 0xFF;
			addr->p2 = 0xFF;
			addr->p3 = 0xFF;
			addr->p4 = 0;
			addr->p5 = 0;
			cmdtype = comm;
			pmxstate = RESUME;
			pmstate = PMPKTXFER;
			pmunit = unitf;
			addr = pmBIaddr(pmhwbase);
			break;
		default:
			rp->errcode = -1;
			return(rp);
	}
		opri = spl2();
		addr->cmdreg = cmdtype;
		while (pmstate != IDLING)	;
		splx(opri);
		return(rp);
}
