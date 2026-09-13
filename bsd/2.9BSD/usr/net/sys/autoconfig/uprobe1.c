/*
 * User-level probe routines to make devices interrupt.
 * One per device; entered through uprobe table.
 * Return values:
 *	ACP_NXDEV	device doesn't exist
 *	ACP_IFINTR	OK if device has interrupted by now
 *	ACP_EXISTS	OK, not checking interrupt
 *
 * NOTES:
 *	Reads and writes to kmem (done by grab, stuff)
 *	are currently done a byte at a time in the kernel.
 *	Beware!
 *
 *	The hs, rp, hk and dvhp probes have not been tested.
 */

#include	"uprobe.h"
#include	<sys/param.h>
#include	<sys/autoconfig.h>

#include	<sys/hkreg.h>
#include	<sys/hpreg.h>
#include	<sys/hsreg.h>
#include	<sys/rkreg.h>
#include	<sys/rlreg.h>
#include	<sys/rpreg.h>

#include	<sys/htreg.h>
#undef	b_repcnt
#include	<sys/tmreg.h>
#undef	b_repcnt
#include	<sys/tsreg.h>

int	xpprobe(), hkprobe(), hsprobe(), rlprobe(), rkprobe(), rpprobe();
int	htprobe(), tmprobe(), tsprobe();
int	dnprobe(), klprobe(), dzprobe(), dhprobe(), dmprobe();
int	lpprobe(), vpprobe();
#ifdef	VIRUS
int	caryprobe();
#endif
extern	int errno;

struct uprobe uprobe[] = {
	/*
	 *	Disks
	 */
	"xp",	xpprobe,
	"rm",	xpprobe,
	"hp",	xpprobe,
	"hk",	hkprobe,
	"hs",	hsprobe,
	"rl",	rlprobe,
	"rk",	rkprobe,
	"rp",	rpprobe,

	/*
	 *	Tapes
	 */
	"ht",	htprobe,
	"tm",	tmprobe,
	"ts",	tsprobe,

	/*
	 *	Communication interfaces
	 */
	"dn",	dnprobe,
	"kl",	klprobe,
	"dz",	dzprobe,
	"dh",	dhprobe,
	"dm",	dmprobe,

	/*
	 *	Printers
	 */
	"lp",	lpprobe,
	"vp",	vpprobe,
#ifdef	VIRUS
	/*
	 *	Don't ask
	 */
	"cary",	caryprobe,
#endif
	0,	0
};

xpprobe(addr)
struct hpdevice *addr;
{
	stuff(HP_IE | HP_RDY, &(addr->hpcs1.w));
	DELAY(10);
	stuff(0, &(addr->hpcs1.w));
	return(ACP_IFINTR);
}

hkprobe(addr)
struct hkdevice *addr;
{
	stuff(HK_CDT | HK_IE | HK_CRDY, (&(addr->hkcs1)));
	DELAY(10);
	stuff(HK_CDT, (&(addr->hkcs1)));
	return(ACP_IFINTR);
}

hsprobe(addr)
struct hsdevice *addr;
{
	stuff(HS_IE | HS_DCLR | HS_GO, (&(addr->hscs1)));
	DELAY(10);
	stuff(0, (&(addr->hscs1)));
	return(ACP_IFINTR);
}

rlprobe(addr)
struct rldevice *addr;
{
	stuff(RL_GETSTATUS | RL_IE, (&(addr->rlcs)));
	DELAY(100);
	stuff(RL_CRDY, (&(addr->rlcs)));
	return(ACP_IFINTR);
}

rkprobe(addr)
struct rkdevice *addr;
{
	stuff(RKCS_IDE | RKCS_DRESET | RKCS_GO, (&(addr->rkcs)));
	DELAY(10);
	stuff(0, (&(addr->rkcs)));
	return(ACP_IFINTR);
}


rpprobe(addr)
struct rpdevice *addr;
{
	stuff(RP_IDE | RP_IDLE | RP_GO, (&(addr->rkcs.w)));
	DELAY(10);
	stuff(0, (&(addr->rpcs.w)));
	return(ACP_IFINTR);
}

htprobe(addr)
struct htdevice *addr;
{
	stuff(HT_SENSE | HT_IE | HT_GO, (&(addr->htcs1)));
	DELAY(10);
	stuff(0, (&(addr->htcs1)));
	return(ACP_IFINTR);
}

/*
 * TM-11 probe routine.
 * Also check one of the more distant registers
 * to make sure this isn't a TS-11.
 */
tmprobe(addr)
struct tmdevice *addr;
{
	stuff(TM_IE, &(addr->tmcs));
	errno = 0;
	grab(&(addr->tmba));
	if (errno != 0)
		return(ACP_NXDEV);
	return(ACP_IFINTR);
}

/*
 * TS-11 probe.
 * Assume that the device exists if there's no TM-11 there.
 */
tsprobe(addr)
struct tsdevice *addr;
{
	errno = 0;
	grab(&((struct tmdevice *)addr->tmba));
	if (errno == 0)
		return(ACP_NXDEV);
	return(ACP_EXISTS);
}
