|init bootstrap

	.text
	.globl	_main, _end, _edata

start:
	movw	#0x2600,sr	| spl6
	movl	#0x12000-2,sp	| initialize stack pointer
	movl	#_end,d0	| clear bss
	movl	#_edata,a0
1$:	movl	#0,a0@+
	cmpl	d0,a0
	jcs	1$
	jmp	_main

	.globl	_spl2, _splx
_spl2:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2200,sr	| set priority 2
	rts

_splx:	movw	sp@(6),sr	| set priority
	rts
