/*
 *	SCCS id	@(#)boot.s	1.2 (Berkeley)	9/6/82
 */
#include "whoami.h"

#ifdef	UCB_AUTOBOOT
/  The boot options and device are placed in the last SZFLAGS bytes
/  at the end of core for the bootstrap.
ENDCORE=	160000		/ end of core, mem. management off
SZFLAGS=	6		/ size of boot flags
BOOTOPTS=	2		/ location of options, bytes below ENDCORE
BOOTDEV=	4
CHECKWORD=	6

reset= 	5

.globl	_doboot, hardboot
.text
_doboot:
	mov	4(sp),r4	/ boot options
	mov	2(sp),r3	/ boot device

#ifndef	KERN_NONSEP
/  If running separate I/D, need to turn off memory management.
/  Call the routine unmap in low text, after setting up a jump
/  in low data where the PC will be pointing.
.globl	unmap
	mov	$137,*$unmap+2		/ jmp *$hardboot
	mov	$hardboot,*$unmap+4
	jmp	unmap
	/ "return" from unmap will be to hardboot in data
.data
#else
/  Reset to turn off memory management
	reset
#endif

/  On power fail, hardboot is the entry point (map is already off)
/  and the args are in r4, r3.

hardboot:
	mov	r4, ENDCORE-BOOTOPTS
	mov	r3, ENDCORE-BOOTDEV
	com	r4		/ if CHECKWORD == ~bootopts, flags are believed
	mov	r4, ENDCORE-CHECKWORD
1:
	reset

/  The remainder of the code is dependent on the boot device.
/  If you have a bootstrap ROM, just jump to the correct entry.
/  Otherwise, use a BOOT opcode, if available;
/  if necessary, read in block 0 to location 0 "by hand".

/
/ Bootstrap for DIVA Comp/V controller without boot opcode
/
HPCSR=	176700		/ Drive #0
HPDC=	176734		/ Desired cylinder
HPBAE=	176750		/ Bus extension address (RH70)
HPCS2=	176710		/ Control/status register 2
READIT=	71
CSW=	177570		/ Console switch display register

1:
	bitb	$200,*$HPCSR	/ wait for ready
	beq	1b

	mov	$0710,*$CSW	/ For debugging
	mov	$0,*$HPDC	/ Cylinder 0
	mov	$0,*$HPBAE	/ Bus extension address = 0
	mov	$HPCS2,r0
	mov	$0,-(r0)	/ HPCSR->hpda = 0 (desired address 0)
	mov	$0,-(r0)	/ HPCSR->hpba = 0 (buf address 0)
	mov	$177400,-(r0)	/ HPCSR->hpwc = -256 (one block)
	mov	$READIT,-(r0)	/ HPCSR->hpcs1 = HP_RCOM|HP_GO
	mov	$0711,*$CSW	/ For debugging

1:
	bitb	$200,*$HPCSR	/ wait for done
	beq	1b
	mov	$0712,*$CSW	/ For debugging
	jmp	*$0

/ no return
#endif	UCB_AUTOBOOT
