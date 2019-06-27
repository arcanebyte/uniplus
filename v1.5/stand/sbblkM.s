|init bootstrap

	.text
	.globl	_begin, _end, _edata

_begin:
start:	movl	#_end+0x3000,sp	| initialize stack pointer
	movl	#_end,d0	| clear bss
	movl	#_edata,a0
	movw	#0x2700,sr	| spl7
	movl	#start,0x7C	| restart on NMI
				| movl	#0,_serinit
1$:	movl	#0,a0@+
	cmpl	d0,a0
	jcs	1$
	jmp	_snblk0

	.globl	_spl2, _splx
_spl2:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2200,sr	| set priority 2
	rts

_splx:	movw	sp@(6),sr	| set priority
	rts
