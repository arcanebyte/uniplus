| Apple Profile/Widget reset-booter
| SEE the NOTES file before you change anything
|
| There are two different rom read_block routines
| one for the builtin parallel port the other
| for the expansion slots. If its the built in
| do a jsr to READ otherwise
| on entry the stack has the address of the rom
| routine that should be called 
| and in A0 is the logic base that needs to
| remain there as one of the passed parameters
| This code must be relocatable since it will
| be loaded approximately 2k above its normal address

READ	= 0xFE0090		| rom read profile address
BDEV	= 0x1B3			| dev we were booted from
USTART	= 0x60000		| load address
SSEC	= 1			| starting disk sector
LEN	= 100			| sectors to load

.globl	_problk0
_problk0:
start:
	movl	sp@,a4		| where to call for rom read
	movw	#0x2700,sr	| spl7
	movl	#0x40000,sp	| initialize stack pointer

	movl	#USTART,a1	| where we want to load booter
	movl	a1,a2		| addr to pass to rom (a1 and a2 are needed)
	subw	#20,a1		| where rom will load header
	movl	#LEN,d5

	movl	#SSEC,d1	| starting sector number
	cmpb	#0x2,BDEV	| boot device (setup by ROM)
	bgt	.1$		| use expansion routines
	movl	#READ,a4	| builtin parallel port read routine
.1$: 	movl	#0x1000,d2	| timeout count
	movl	#10,d3		| retry count
	movl	#3,d4		| threshold
	clrl	d0
	jsr	a4@		| rom read routine
	tstb	d0		| test results
	bne	.2$		| print err message
	addl	#512,a2		| update data pointer
	addql	#1,d1		| next sector
	dbra	d5,.1$		| read loop
	jmp	USTART
.2$:	jmp	0xFE0084	| rom monitor
