/*
 * uprobe2.c: continuation of uprobes
 * (separated into two files because the include files overflow cpp otherwise).
 *
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
 */

#include	"uprobe.h"
#include	<sys/param.h>
#include	<sys/autoconfig.h>
#include	<sgtty.h>

#include	<sys/dhreg.h>
#include	<sys/dnreg.h>
#undef	BITS7
#undef	BITS8
#undef	TWOSB
#undef	PENABLE
#undef	OPAR
#include	<sys/dzreg.h>
#include	<sys/klreg.h>

#include	<sys/lpreg.h>
#include	<sys/vpreg.h>

extern	int errno;

klprobe(addr)
struct dldevice *addr;
{
	int i;

	stuff(grab(&(addr->dlxcsr)) | DLXCSR_TIE, &(addr->dlxcsr));
	for (i = 0; i < 7; i++)
		DELAY(10000);
	/*
	 *  Leave TIE enabled; kl.c never turns it off
	 *  (and this could be the console).
	 */
	return(ACP_IFINTR);
}


dzprobe(addr)
struct dzdevice *addr;
{
	register int i;

	stuff(grab(&(addr->dzcsr)) | DZ_TIE | DZ_MSE, &(addr->dzcsr));
	stuff(1, &(addr->dztcr));
	for (i = 0; i < 7; i++)
		DELAY(10000);
	stuff(DZ_CLR, &(addr->dzcsr));
	return(ACP_IFINTR);
}


dhprobe(addr)
struct dhdevice *addr;
{
	register int i;

	stuff(DH_TIE, &(addr->dhcsr));
	DELAY(5);
	stuff((B9600 << 10) | (B9600 << 6) | BITS7|PENABLE, &(addr->dhlpr));
	stuff(-1, &(addr->dhbcr));
	stuff(0, &(addr->dhcar));
	stuff(1, &(addr->dhbar));
	for (i = 0; i < 7; i++)
		DELAY(10000);
	stuff(0, &(addr->dhcsr));
	return(ACP_IFINTR);
}

dmprobe(addr)
struct dmdevice *addr;
{
	stuff(grab(&(addr->dmcsr)) | DM_DONE | DM_IE, &(addr->dmcsr));
	DELAY(20);
	stuff(0, &(addr->dmcsr));
	return(ACP_IFINTR);
}

/*
 *  Try to make the first unit of a DN-11 interrupt.
 */
dnprobe(addr)
struct dndevice *addr;
{
	stuff(DN_MINAB | DN_INTENB | DN_DONE, (&(addr->dnisr[0])));
	DELAY(5);
	stuff(0, (&(addr->dnisr[0])));
	return(ACP_IFINTR);
}

lpprobe(addr)
struct lpdevice *addr;
{
	stuff(grab(&(addr->lpcs)) | LP_IE, &(addr->lpcs));
	DELAY(10);
	stuff(0, &(addr->lpcs));
	return(ACP_IFINTR);
}

vpprobe(addr)
struct vpdevice *addr;
{
#ifdef	VAX
	/*
	 * This is the way the 4.1 driver does it.
	 */
	errno = 0;
	stuff(VP_IENABLE | VP_DTCINTR, (&(addr->prcsr)));
	stuff(0, (&(addr->pbaddr)));
	stuff(3, (&(addr->pbxaddr)));
	stuff(01777776, (&(addr->prbcr)));
	DELAY(10000);
	stuff(0, (&(addr->prcsr)));
	if (errno)
		return(ACP_NXDEV);	/* Possibly an LP csr, but no print DMA regs */
	else
		return(ACP_IFINTR);
#else
	errno = 0;
	/*
	 *  Use the plot csr now, to distinguish from a line printer.
	 */
	stuff(VP_IENABLE | VP_CLRCOM, (&(addr->plcsr)));
	DELAY(10000);
	/*
	 *  Make sure that the DMA registers are there.
	 */
	grab(&(addr->plbcr));
	/*
	 * Write the print csr this time, to leave it in print mode.
	 */
	stuff(0, (&(addr->prcsr)));
	if (errno)
		return(ACP_NXDEV);	/* Possibly an LP csr, but no plot regs */
	else
		return(ACP_IFINTR);
#endif
}

#ifdef	VIRUS
/*
 * Can't make the cary interrupt unless it's in fixed-wavelength mode.
 */
/*ARGSUSED*/
caryprobe(addr)
{
	return(ACP_EXISTS);
}
#endif
