/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * "rom_mon" is used by the software power off feature (see kb.c)
 * and the reboot system call (see config.c, doboot) to return to
 * the ROM monitor in "Customer Mode".
 */

#include "sys/mmu.h"
typedef int (*pfri)();
#define ROMADDR ((pfri)(0xFE0084))

rom_mon()
{
	asm("	movl	#0,d0");	/* no error code */
	asm("	subl	a2,a2");	/* no icon */
	asm("	subl	a3,a3");	/* no message */
	asm("	movl	0x1000,sp");
	ROMADDR();
}
