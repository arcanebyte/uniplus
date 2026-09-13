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

/  Bootstrap for rl01/02 drive - salkind@nyu

WC	= -256.
READ	= 6\<1
SEEK	= 3\<1
RDHDR	= 4\<1

rlcs	= 174400
rlda	= 174404
rlba	= 174402
rlmp	= 174406

	mov	$RDHDR,*$rlcs	/find out where we are (cyl)
1:
	tstb	*$rlcs
	bpl	1b
	mov	*$rlmp,r0
	bic	$!77600,r0
	bis	$1,r0
	mov	r0,*$rlda
	mov	$SEEK,*$rlcs	/ move it
1:
	tstb	*$rlcs
	bpl	1b
/
	mov	$rlmp,r0
	mov	$WC,(r0)	/wc into rlmp
	clr	-(r0)		/da into rlda
	clr	-(r0)		/ba
	mov	$READ,-(r0)	/cmd into rlcs
1:
	tstb	*$rlcs
	bpl	1b
/
	jmp	*$0		/ and away we go

#endif	UCB_AUTOBOOT
