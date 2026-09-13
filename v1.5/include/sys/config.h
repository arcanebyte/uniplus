/*
 * RECONSTRUCTED -- not an original UniSoft file (2026).
 * The Lisa sys/config.h was lost; the Torch Triple X copy is for different
 * hardware.  Contents are only what the V.1.5+ kernel sources require.
 * See ../PROVENANCE.md for sources and confidence of each definition.
 */

/*
 * Block device major numbers.  The kernel uses these both as bdevsw[]
 * indices and as slot[] type codes; each name carries its own value.
 * Evidence: bdevsw[] order in conf.c (0 pro, 1 snb, 2 cv, 3 pm) and
 * tabinit(PR0,...)/tabinit(PM3,...) in pro.c/priam.c.       CONFIRMED
 */
#define	PR0	0		/* ProFile (parallel port) */
#define	SN1	1		/* Sony floppy */
#define	CV2	2		/* Corvus */
#define	PM3	3		/* Priam */

/*
 * Number of SCC serial lines: conf.c declares two sc_ttptr[] entries
 * (0xFCD240 channel B, 0xFCD242 channel A).                  CONFIRMED
 */
#define	NSC	2

/*
 * Number of Tecmar quad serial ports (4 per card).  conf.c only defines
 * te_tty[], te_ttptr[], te_dparam[], te_modem[] and te_cnt "#if NTE != 0",
 * and tecmar.c always references them.  Value: te_cnt = 4 in the working
 * UniPlus 1.4 /unix on the 10 MB Lisa disk (te_tty[] spans 4 struct tty).
 *                                                            CONFIRMED
 */
#define	NTE	4

/*
 * Device number of the console (bitmap display/keyboard): cdevsw major 0,
 * minor 0.  co.c compares a minor number, ms.c compares u.u_ttyd against
 * it; both are 0 for the console.                            INFERRED
 */
#define	CONSOLE	0
