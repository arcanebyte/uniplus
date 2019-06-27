| Apple Priam reset-booter
|
| The address of the "read block" routine is passed
| on the stack.  The logical base address in A0 needs
| to remain there as one of the passed parameters.
| This code must be relocatable since it will
| be loaded approximately 2k above its normal address.
| The boot ROM reads this program in above 20000 and
| does a jsr to it.

USTART	= 0x60000		| load address
SSEC	= 1			| starting disk sector
LEN	= 100			| sectors to load

.globl	_pmblk0
_pmblk0:
start:
	movl	sp@,a4		| where to call for rom read
	movl	a0,a5		| save a0 to restore it after each read
	movw	#0x2700,sr	| spl7
	movl	#0x40000,sp	| initialize stack pointer

	movl	#USTART,a1	| where we want to load booter
	movl	a1,a2		| addr to pass to rom (a1 and a2 are needed)
	subw	#24,a1		| where rom will load header

	movl	#SSEC,d5	| starting sector number
.1$:	movl	d5,d1		| starting sector number
	movl	#0x1000,d2	| timeout count
	movl	#10,d3		| retry count
	movl	#3,d4		| threshold
	movl	a5,a0		| restore a0 to base address
	clrl	d0
	jsr	a4@		| rom read routine
	tstb	d0		| test results
	bne	.2$		| print err message and quit
	addl	#512,a2		| update data pointer
	addql	#1,d5		| next sector
	cmpl	#LEN,d5
	ble	.1$
	jmp	USTART

.2$:	jmp	0xFE0084	| rom monitor (no return)
