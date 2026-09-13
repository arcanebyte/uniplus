| USIZE dependencies
USIZE	= 0x800			| Size of U area
PGSIZE	= 512			| Size of a page
PAGESHIFT= 9			| Page shift

| Configuration dependencies
HIGH	= 0x2700		| High priority supervisor mode (spl 7)
LOW	= 0x2000		| Low priority, supervisor mode (spl 0)
USTART	= 0x0			| Start of user program
PAGEBASE= 0xA00000		| Base page address
UDOT	= 0xFA0000		| Logical start of U dot
UBASE	= PAGEBASE+UDOT		| U dot page map address

SETUP_0	= 0xFCE012		| Turn setup bit off
SETUP_1	= 0xFCE010		| Turn setup bit on
SEG1_0	= 0xFCE008		| Turn SEG1 bit off
SEG1_1	= 0xFCE00A		| Turn SEG1 bit on
SEG2_0	= 0xFCE00C		| Turn SEG2 bit off
SEG2_1	= 0xFCE00E		| Turn SEG2 bit on

	.globl	_u, _segoff
	.data
_u	= UDOT
cputype:.word	0		| Local copy of _cputype, 0 if 68000

	.text
	.globl	start, _end, _edata, _main, _cputype, _dispatc

start:	movw	#HIGH,sr		| spl7
	movl	#_end+PGSIZE+USIZE-1,d7	| End of unix
	andl	#-PGSIZE,d7		| Round to nearest click
	movl	#_edata,a0		| Start clearing here
clrbss:	movl	#0,a0@+			| Clear bss
	cmpl	d7,a0
	jcs	clrbss

	| Determine cpu type
	movl	#1$,16			| Illegal instruction vector
	.word	0x4E7A			|	movec
	.word	0x0801			|	vbr,d0
	movl	#68010,_cputype		| No trap. Must be 68010
	movw	#1,cputype		| Local copy

1$:	subl	#USIZE,d7		| Start of U dot
	moveq	#PAGESHIFT,d1		| Calculate U dot page entry
	lsrl	d1,d7			| Calculate U dot page entry
	movb	#0,SEG1_0		| Set Contex 0
	movb	#0,SEG2_0		| Set Contex 0
	movb	#0,SETUP_1		| Enable MMU modification
	movw	0+0x8008,d1		| Read segment origin
	movb	#0,SETUP_0		| Disable MMU modification
	andl	#0xFFF,d1		| Take just what is valid
	movw	d1,_segoff		| Save segment offset
	addl	d1,d7			| Add segment offset
	movb	#0,SETUP_1		| Enable MMU modification
	movw	d7,UDOT+0x8008		| Segment origin
	movw	#0x7FC,UDOT+0x8000	| Segment access and length (4 clicks)
	movb	#0,SETUP_0		| Disable MMU modification
	subl	d1,d7			| Subtract off segment offset
	movl	#UDOT+USIZE,sp		| Set stack at top of U area

	jsr	_bminit			| initialize raster display

	movl	d7,sp@-			| Click address of udot to main
	jsr	_main			| Long jump to unix, init returns here

	clrl	sp@-			| Indicate short 4 byte stack format
	movl	#USTART,sp@-		| Starting program address
	clrw	sp@-			| New sr value
	rte				| Call init

| save and restore of register sets

	.globl	_save, _resume, _qsave
_save:	movl	sp@+,a1		| return address
	movl	sp@,a0		| ptr to label_t
	moveml	#0xFCFC,a0@	| save d2-d7, a2-a7
	movl	a1,a0@(48)	| save return address
	moveq	#0,d0
	jmp	a1@		| return

_qsave:	movl	sp@+,a1		| return address
	movl	sp@,a0		| ptr to label_t
	addw	#40,a0
	movl	a6,a0@+		| save a6
	movl	a7,a0@+		| save a7
	movl	a1,a0@+		| save return address
	moveq	#0,d0
	jmp	a1@		| return

_resume:movl	sp@(4),d0		| click address of new udot
	movl	sp@(8),a0		| ptr to label_t
	movw	#HIGH,sr		| spl 7
	movb	#0,SEG1_0		| Set Contex 0
|	movb	#0,SEG2_0		| Set Contex 0
	addw	_segoff,d0		| Add segment offset
	movb	#0,SETUP_1		| Enable MMU modification
	movw	d0,UDOT+0x8008		| Segment origin
	movw	#0x7FC,UDOT+0x8000	| Segment access and length (2k)
	movb	#0,SETUP_0		| Disable MMU modification
	movb	#0,SEG1_1		| Set Contex 1
|	movb	#0,SEG2_0		| Set Contex 1
	moveml	a0@+,#0xFCFC		| restore the registers
	movw	#LOW,sr			| restore spl 0
	movl	a0@,a1			| fetch the original pc
	moveq	#1,d0			| return 1
	jmp	a1@			| return

| spl commands
	.globl	_splhi,_spl7,_spl6,_spl5,_spl4,_spl3,_spl2,_spl1,_spl0,_splx

_splhi:
_spl7:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2700,sr	| set priority 7
	rts
_spl6:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2600,sr	| set priority 6
	rts
_spl5:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2500,sr	| set priority 5
	rts
_spl4:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2400,sr	| set priority 4
	rts
_spl3:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2300,sr	| set priority 3
	rts
_spl2:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2200,sr	| set priority 2
	rts
_spl1:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2100,sr	| set priority 1
	rts
_spl0:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2000,sr	| set priority 0
	rts

_splx:	movw	sp@(6),sr	| set priority
	rts

	.data
	.globl	_idleflg
_idleflg:.word	0

	.text
	.globl	_idle, idle1, _waitloc
_idle:	movw	sr,d0		| Fetch current CPU priority
	movw	#1,_idleflg	| Set idle flag
	movw	#0x2000,sr	| Set priority zero
idle1:	tstw	_idleflg	| Wait for interrupt
_waitloc:			| Pseudo location addr used by kernel profiling
	bne	idle1
	movw	d0,sr		| Restore priority
	rts

	.globl	buserr, addrerr, fault, call, _busaddr
	.globl	_runrun, _trap

fault:	clrw	sp@-		| this makes ps long aligned
	clrw	_idleflg	| clear idle flag
	moveml	#0xFFFF,sp@-	| save all registers
	movl	usp,a0
	movl	a0,sp@(60)	| save usr stack ptr
	movl	sp@(66),d0	| return ptr from the jsr
	subl	#_dispatc+4,d0	| subtract dispatch table offset
	asrl	#2,d0		| calculate vector number
resched:movl	d0,sp@-		| argument to trap
	jsr	_trap		| C handler for traps and faults
	addql	#4,sp
	jsr	checknet	| check for net int requests
	btst	#5,sp@(70)	| did we come from user mode?
	jne	2$		| no, just continue
	tstb	_runrun		| should we reschedule?
	jeq	2$		| no, just return normally
	movl	#256,d0		| 256 is reschedule trap number
	jra	resched		| go back into trap

2$:	movl	sp@(60),a0
	movl	a0,usp		| restore usr stack ptr
	movb	#0,SEG1_1	| Set Contex 1
|	movb	#0,SEG2_0	| Set Contex 1
	moveml	sp@+,#0x7FFF	| restore all other registers
	addw	#10,sp		| sp, pop fault pc, and alignment word
	rte

	.globl	syscall, _syscall
syscall:clrw	sp@-		| this makes ps long aligned
	moveml	#0xFFFF,sp@-	| save all registers
	movl	usp,a0
	movl	a0,sp@(60)	| save usr stack ptr
	btst	#5,sp@(70)	| did we come from user mode?
	jne	3$		| no, error !!!
	jsr	_syscall	| Process system call
	jsr	checknet	| check for net int requests
	tstb	_runrun		| should we reschedule?
	jeq	2$		| no, just return normally
	movl	#256,d0		| 256 is reschedule trap number
	jra	resched		| go back into trap

2$:	movl	sp@(60),a0
	movl	a0,usp		| restore user stack pointer
	moveml	sp@+,#0x7FFF	| restore all other registers
	addw	#10,sp		| sp, pop fault pc, and alignment word
	rte

3$:	moveml	sp@+,#0x7FFF	| restore registers
	addql	#6,sp		| sp, pop fault pc, and alignment word
	movl	#_dispatc+132,sp@ | simulate a bsr
	jra	fault

| Bus error entry, this has its stack somewhat different.  We will
| call a C routine to save the info then fix the stack to look like a trap.
| These entries will be called directly from interrupt vector.

buserr:	tstw	cputype		| test cpu type
	bne	3$		| branch if 68010
	moveml	#0xC0C0,sp@-	| save registers that C clobbers
	jsr	_busaddr	| save the info for a bus or address error
	moveml	sp@+,#0x303	| restore registers
	addql	#8,sp		| pop bsr address, fcode, aaddr and ireg
3$:	movl	#_dispatc+12,sp@| simulate a bsr
	bra	fault

addrerr:tstw	cputype		| test cpu type
	bne	4$		| branch if 68010
	moveml	#0xC0C0,sp@-	| save registers that C clobbers
	jsr	_busaddr	| save the info for a bus or address error
	moveml	sp@+,#0x303	| restore registers
	addql	#8,sp		| pop bsr address, fcode, aaddr and ireg
4$:	movl	#_dispatc+16,sp@| simulate a bsr
	bra	fault

| common interrupt dispatch

call:	moveml	#0xFFFF,sp@-	| save all registers
	clrw	_idleflg	| clear idle flag
	movl	usp,a0
	movl	a0,sp@(60)	| save usr stack ptr
	movl	sp@(66),a0	| fetch interrupt routine address
	movl	sp,sp@-		| push argument list pointer onto stack
	jsr	a0@		| jump to actual interrupt handler
	addql	#4,sp
	jsr	checknet	| check for net int requests
	btst	#5,sp@(70)	| did we come from user mode?
	jne	2$		| no, just continue
	tstb	_runrun		| should we reschedule?
	jeq	2$		| no, just return normally
	movl	#256,d0		| 256 is reschedule trap number
	jra	resched		| go back into trap

2$:	movl	sp@(60),a0
	movl	a0,usp		| restore usr stack ptr
	movb	#0,SEG1_1	| Set Contex 1
|	movb	#0,SEG2_0	| Set Contex 1
	moveml	sp@+,#0x7FFF	| restore all other registers
	addw	#10,sp		| sp, pop fault pc, and alignment word
	rte			| return from whence called

|	General purpose code

	.globl	_tstb, _getusp, _getsr

_tstb:	tstb	sp@(0)		| stack probe instruction prototype

_getusp:movl	usp,a0		| get the user stack pointer
	movl	a0,d0
	rts

_getsr:	moveq	#0,d0		| get the sr
	movw	sr,d0
	rts

|	Net int request handler

	.globl	_netisr, _netintr, _svstak
|.globl	_chkstak
checknet:
	tstl    _netisr                 | net requesting soft interrupt?
	beq     3$			| no: return
	movw	#0x2700,sr		| set priority 7
	tstb	innet			| already in net code ?
	bne     3$			| yes: return
	movb	#1,innet		| no: set flag -> in the net code
	movl	sp,_svstak		| save stack, get net stack
	movl	#_netstak+2996,sp
	|movw	#0x2600,sr		| set priority 6
	movw	#0x2000,sr		| set priority 0
	jsr     _netintr		| do tasks
	movl	_svstak,sp
	clrb	innet			| clear flag
3$:
	rts

	.data
innet:	.byte	0

	.text
|	.globl	cs1, _chkstak

| first stage of checkstack routine. save regs, call real routine
|cs1:
	| moveml	#0xFFFF,sp@-	| save all registers
	|jsr	_chkstak	| really check the stack
	| moveml	sp@+,#0xFFFF	| restore all registers
	|rts
