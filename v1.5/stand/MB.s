|init bootstrap

	.text
	.globl	_main, _end, _edata

start:	movl	#start,sp	| initialize stack pointer
	bsr	getpc		| put next pc onto the stack
getpc:	movl	sp@+,d0		| put pc into d0
	subl	#getpc-start,d0	| subtract offset
	movl	d0,a0
	movl	#start,a1	| destination address
	movl	#4095,d0	| copy 16K bytes
1$:	movl	a0@+,a1@+
	dbra	d0,1$
	jmp	cont		| long jump
cont:
	movl	#start,sp	| initialize stack pointer
	movl	#_end,d0	| clear bss
	movl	#_edata,a0	| clear bss
1$:	movl	#0,a0@+		| clear bss
	cmpl	d0,a0		| clear bss
	jcs	1$		| clear bss
	jmp	_main
