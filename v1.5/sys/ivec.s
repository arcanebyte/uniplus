| Copyright 1982 UniSoft Corporation


| Interrupt vector dispatch table
| One entry per interrupt vector location

	.text
	.globl	_dispatc, _pmvect, _tevect
_dispatc:
	bsr	lfault		| 0	Reset: Initial SSP
	bsr	lfault		| 1	Reset: Initial PC
	bsr	lbuserr		| 2	Bus Error
	bsr	laddrerr	| 3	Address Error
	bsr	lfault		| 4	Illegal Instruction
	bsr	lfault		| 5	Zero Divide
	bsr	lfault		| 6	CHK Instruction
	bsr	lfault		| 7	TRAPV Instruction
	bsr	lfault		| 8	Privilege Violation
	bsr	lfault		| 9	Trace
	bsr	lfault		| 10	Line 1010 Emulator
	bsr	lfault		| 11	Line 1111 Emulator
	bsr	lfault		| 12	(Unassigned, reserved)
	bsr	lfault		| 13	(Unassigned, reserved)
	bsr	lfault		| 14	(Unassigned, reserved)
	bsr	lfault		| 15	(Unassigned, reserved)
	bsr	lfault		| 16	(Unassigned, reserved)
	bsr	lfault		| 17	(Unassigned, reserved)
	bsr	lfault		| 18	(Unassigned, reserved)
	bsr	lfault		| 19	(Unassigned, reserved)
	bsr	lfault		| 20	(Unassigned, reserved)
	bsr	lfault		| 21	(Unassigned, reserved)
	bsr	lfault		| 22	(Unassigned, reserved)
	bsr	lfault		| 23	(Unassigned, reserved)
	bsr	lfault		| 24	Spurious Interrupt
	bsr	l1intr		| 25	Level 1 pp 0, sony, vert retrace (clock)
	bsr	kbintr		| 26	Level 2 cops, mouse, rtclock, buttons
	bsr	pi5		| 27	Level 3 exp slot 3 (default 2-port card)
	bsr	pi4		| 28	Level 4 exp slot 2 (default 2-port card)
	bsr	pi3		| 29	Level 5 exp slot 1 (default 2-port card)
	bsr	scintr		| 30	Level 6 scc chip
	bsr	nmi		| 31	Level 7 Interrupt Autovector
	bsr	lsyscall	| 32	System Call
	bsr	lfault		| 33	TRAP Instruction Vector
	bsr	lfault		| 34	TRAP Instruction Vector
	bsr	lfault		| 35	TRAP Instruction Vector
	bsr	lfault		| 36	TRAP Instruction Vector
	bsr	lfault		| 37	TRAP Instruction Vector
	bsr	lfault		| 38	TRAP Instruction Vector
	bsr	lfault		| 39	TRAP Instruction Vector
	bsr	lfault		| 40	TRAP Instruction Vector
	bsr	lfault		| 41	TRAP Instruction Vector
	bsr	lfault		| 42	TRAP Instruction Vector
	bsr	lfault		| 43	TRAP Instruction Vector
	bsr	lfault		| 44	TRAP Instruction Vector
	bsr	lfault		| 45	TRAP Instruction Vector
	bsr	lfault		| 46	TRAP Instruction Vector
	bsr	lfault		| 47	TRAP Instruction Vector
	bsr	lfault		| 48	(Unassigned, reserved)
	bsr	lfault		| 49	(Unassigned, reserved)
	bsr	lfault		| 50	(Unassigned, reserved)
	bsr	lfault		| 51	(Unassigned, reserved)
	bsr	lfault		| 52	(Unassigned, reserved)
	bsr	lfault		| 53	(Unassigned, reserved)
	bsr	lfault		| 54	(Unassigned, reserved)
	bsr	lfault		| 55	(Unassigned, reserved)
	bsr	lfault		| 56	(Unassigned, reserved)
	bsr	lfault		| 57	(Unassigned, reserved)
	bsr	lfault		| 58	(Unassigned, reserved)
	bsr	lfault		| 59	(Unassigned, reserved)
	bsr	lfault		| 60	(Unassigned, reserved)
	bsr	lfault		| 61	(Unassigned, reserved)
	bsr	lfault		| 62	(Unassigned, reserved)
	bsr	lfault		| 63	(Unassigned, reserved)
 	bsr	lfault		| 64	(User Interrupt Vector)
	bsr	lfault		| 65	(User Interrupt Vector)
 	bsr	lfault		| 66	(User Interrupt Vector)
 	bsr	lfault		| 67	(User Interrupt Vector)
 	bsr	lfault		| 68	(User Interrupt Vector)
 	bsr	lfault		| 69	(User Interrupt Vector)
 	bsr	lfault		| 70	(User Interrupt Vector)
 	bsr	lfault		| 71	(User Interrupt Vector)
 	bsr	lfault		| 72	(User Interrupt Vector)
 	bsr	lfault		| 73	(User Interrupt Vector)
 	bsr	lfault		| 74	(User Interrupt Vector)
 	bsr	lfault		| 75	(User Interrupt Vector)
 	bsr	lfault		| 76	(User Interrupt Vector)
 	bsr	lfault		| 77	(User Interrupt Vector)
 	bsr	lfault		| 78	(User Interrupt Vector)
 	bsr	lfault		| 79	(User Interrupt Vector)
 	bsr	lfault		| 80	(User Interrupt Vector)
 	bsr	lfault		| 81	(User Interrupt Vector)
 	bsr	lfault		| 82	(User Interrupt Vector)
 	bsr	lfault		| 83	(User Interrupt Vector)
 	bsr	lfault		| 84	(User Interrupt Vector)
 	bsr	lfault		| 85	(User Interrupt Vector)
 	bsr	lfault		| 86	(User Interrupt Vector)
 	bsr	lfault		| 87	(User Interrupt Vector)
 	bsr	lfault		| 88	(User Interrupt Vector)
 	bsr	lfault		| 89	(User Interrupt Vector)
 	bsr	lfault		| 90	(User Interrupt Vector)
 	bsr	lfault		| 91	(User Interrupt Vector)
 	bsr	lfault		| 92	(User Interrupt Vector)
 	bsr	lfault		| 93	(User Interrupt Vector)
 	bsr	lfault		| 94	(User Interrupt Vector)

| Values for expansion card interrupt vectors when Priam card present.
_pmvect:
	bsr	pmint3		| 27	Level 3 exp slot 3, Priam card
	bsr	pmint2		| 28	Level 4 exp slot 2, Priam card
	bsr	pmint1		| 29	Level 5 exp slot 1, Priam card

| Values for expansion card interrupt vectors when Techmar card present.
_tevect:
	bsr	teint3		| 27	Level 3 exp slot 3, Tecmar card
	bsr	teint2		| 28	Level 4 exp slot 2, Tecmar card
	bsr	teint1		| 29	Level 5 exp slot 1, Tecmar card

| Put actual "C" routine name onto the stack and call the system
| interrupt dispatcher

	.globl	call, fault, buserr, addrerr, syscall, _idleflg

lfault:	jmp	fault
lbuserr:jmp	buserr
laddrerr:jmp	addrerr
lsyscall:jmp	syscall

	| tty interrupt priority entry point
	.globl	_spltty
_spltty:movw	sr,d0		| fetch current CPU priority
	movw	#0x2600,sr	| set priority 6
	rts

	.globl	_l1intr
l1intr:	movw	#0x2600,sr	| go to spl6
 	movl	#_l1intr,sp@	| push call address
	movw	_idleflg,sp@-	| clock needs old value so cheat
	jmp	call		| jump to common interrupt handler

	.globl	_kbintr
kbintr:	movl	#_kbintr,sp@	| push call address
	clrw	sp@-		| device number
	jmp	call		| jump to common interrupt handler

	.globl	_ppintr
pi3:	movl	#_ppintr,sp@	| push call address
	movw	#1,sp@-		| device number
	jmp	call		| jump to common interrupt handler

pi4:	movl	#_ppintr,sp@	| push call address
	movw	#4,sp@-		| device number
	jmp	call		| jump to common interrupt handler

pi5:	movl	#_ppintr,sp@	| push call address
	movw	#7,sp@-		| device number
	jmp	call		| jump to common interrupt handler

	.globl	_pmintr
pmint3:	movl	#_pmintr,sp@	| push call address
	movw	#2,sp@-		| device number
	jmp	call		| jump to common interrupt handler

pmint2:	movl	#_pmintr,sp@	| push call address
	movw	#1,sp@-		| device number
	jmp	call		| jump to common interrupt handler

pmint1:	movl	#_pmintr,sp@	| push call address
	clrw	sp@-		| device number
	jmp	call		| jump to common interrupt handler

	.globl	_scintr
scintr: movl	#_scintr,sp@	| push call address
	movw	#1,sp@-		| device number
	jmp	call		| jump to common interrupt handler

	.globl	_nmikey
nmi:	movl	#_nmikey,sp@	| push call address
	movw	#0,sp@-		| device number
	jmp	call		| jump to common interrupt handler

	.globl	_teintr
teint3:	movl	#_teintr,sp@	| push call address
	movw	#2,sp@-		| device number
	jmp	call		| jump to common interrupt handler

teint2:	movl	#_teintr,sp@	| push call address
	movw	#1,sp@-		| device number
	jmp	call		| jump to common interrupt handler

teint1:	movl	#_teintr,sp@	| push call address
	clrw	sp@-		| device number
	jmp	call		| jump to common interrupt handler
