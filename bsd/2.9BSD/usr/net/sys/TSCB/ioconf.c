#include	"param.h"
#include	<sys/systm.h>
#include	"hk.h"
#include	"hp.h"
#include	"xp.h"

#ifdef UCB_NET
#include <sys/ubavar.h>
#include "il.h"
#include "sri.h"
#endif UCB_NET

dev_t	rootdev	= makedev(4,0);
dev_t	swapdev	= makedev(6,1);
dev_t	pipedev = makedev(4,0);
daddr_t	swplo	= (daddr_t) 0;
int	nswap	= 10000;

#ifdef	UCB_AUTOBOOT
/*
 * If dumpdev is NODEV, no dumps will be taken automatically.
 * dump can point to nodev, just to resolve the reference.
 */
dev_t	dumpdev = makedev(6,1);
daddr_t	dumplo	= (daddr_t) 2000;
int	xpdump();
int	(*dump)()	= xpdump;
#endif	UCB_AUTOBOOT

/*
 *	Device configuration information.
 */

#if	NHK > 0
struct	hkdevice *HKADDR = 0177440;
struct	size hk_sizes[] = {
	10240,	0,		/* cyl   0 - 155 */
	43164,	156,		/* cyl 156 - 809 */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	27126,	0,		/* cyl   0 - 410, whole RK06 */
	53790,	0		/* cyl   0 - 814, whole RK07 */
};
#endif	NHK

#if	NHP > 0
struct	hpdevice *HPADDR = 0176700;
#endif
#if	NHP > 0 || NXP > 0
struct	size hp_sizes[] = {
	10944,	0,		/* cyl   0 - 17 */
	10944,	18,		/* cyl  18 - 35 */
	473632,	36,		/* cyl  36 - 814 */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	340670,	0		/* cyl   0 - 814 (whole rp06) */
};
#endif	NHP

#if	NHS > 0
struct	hsdevice *HSADDR = 0172040;
#endif	NHS

struct	dldevice *KLADDR = 0177560;

#if	NRK > 0
struct	rkdevice *RKADDR = 0177400;
#endif	NRK

#if	NRL > 0
struct	rldevice *RLADDR = 0174400;
#endif	NRL

#if	NRM > 0
struct	rmdevice *RMADDR = 0176700;
#endif
#if	NRM > 0 || NXP > 0
struct	size rm_sizes[] = {
	4800,	0,		/* cyl   0 -  29 */
	4800,	30,		/* cyl  30 -  59 */
	122080,	60,		/* cyl  60 - 822 */
	62720,	60,		/* cyl  60 - 451 */
	59360,	452,		/* cyl 452 - 822 */
	0,	0,
	0,	0,
	131680,	0,		/* cyl   0 - 822 */
};
#endif	NRM

#if	NRP > 0
struct	rpdevice *RPADDR = 0176710;
struct	size rp_sizes[] = {
	 10400,	0,		/* cyl   0 -  51 */
	 5200,	52,		/* cyl  52 -  77 */
 	 67580,	78,		/* cyl  78 - 414 */
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 83180,	0		/* cyl   0 - 414 */
};
#endif	NRP

#if	NXP > 0
#include <sys/hpreg.h>

/* RM05 */
struct	size rm5_sizes[] = {
	9120,	  0,		/* cyl   0 -  14 */
	9120,	 15,		/* cyl  15 -  29 */
	234080,	 30,		/* cyl  30 - 414 */
	248064,	415,		/* cyl 415 - 822 */
	164160,	 30,		/* cyl  30 - 299 */
	152000,	300,		/* cyl 300 - 549 */
	165376,	550,		/* cyl 550 - 822 */
	500384,	  0		/* cyl   0 - 822 */
};

/* Eagle */
struct	size si_sizes[] = {
	11520,	0,		/* cyl   0 -  11 */
	11520,	12,		/* cyl  12 -  23 */
	474240,	24,		/* cyl  24 - 517 */
	92160,	518,		/* cyl 518 - 613 */
	214080,	614,		/* cyl 614 - 836, reserve 5 cyls */
	0,	0,
	0,	0,
	808320,	0,		/* cyl   0 - 841 (everything) */
};

/* Diva Comp V + Ampex 9300 in direct mode */
struct	size dv_sizes[] = {
	9405,	  0,		/* cyl   0 -  14 */
	9405,	 15,		/* cyl  15 -  29 */
	241395,	 30,		/* cyl  30 - 414 */
	250800,	415,		/* cyl 415 - 814 */
	169290,	 30,		/* cyl  30 - 299 */
	156750,	300,		/* cyl 300 - 549 */
	165528,	550,		/* cyl 550 - 814 */
	511005,	  0		/* cyl   0 - 814 */
};

/*
 *  Xp_controller structure: one line per controller.
 *  Only the address need be initialized in the controller structure
 *  if XP_PROBE is defined (at least the address for the root device);
 *  otherwise the flags must be here also.
 *  The XP_NOCC flag is set for RM02/03/05's (with no current cylinder
 *  register); XP_NOSEARCH is set for Diva's without the search command.
 *  The XP_RH70 flag need not be set here, the driver will always check that.
 */
struct	xp_controller xp_controller[NXP_CONTROLLER] = {
/*	0	0	addr	  flags			0 */
	0,	0,	0176300, XP_NOCC,		0
};

/*
 *  Xp_drive structure: one entry per drive.
 *  The drive structure must be initialized if XP_PROBE is not enabled.
 *  Macros are provided in hpreg.h to initialize entries according
 *  to drive type, and controller and drive numbers.
 *  See those for examples on how to set up other types of drives.
 *  With XP_PROBE defined, xpslave will fill in this structure,
 *  and any initialization will be overridden.  There is one exception;
 *  if the drive-type field is set, it will be used instead of the
 *  drive-type register to determine the drive's type.
 */
struct	xp_drive xp_drive[NXP] = {
#ifndef	XP_PROBE
	SI_INIT(0,0)		/* SI Eagle on controller 0, drive 0 */
#endif	XP_PROBE
};
#endif	NXP

#ifdef UCB_NET

u_long	LocalAddr;		/* Internet address for this host */

#if	NSRI > 0
struct uba_driver sridriver;
#endif

#if	NIL > 0
struct uba_driver ildriver;
#endif

struct uba_device ubdinit[] = {
	/* driver,  unit,  ubanum,   addr,	flags*/
#if	NSRI > 0
	{ &sridriver,   0,     0,   0167770,	0x0a000000 }, /*net 10, lh 0*/
#endif
#if	NIL > 0
	{ &ildriver,	0,     0,   0164000,	0x80120000 }, /* 128.18.0.0 */
#endif
	0
};

#endif UCB_NET
