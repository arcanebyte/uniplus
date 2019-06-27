|init bootstrap from disk

	.text
	.globl	_main, _end, _edata

start:
	.long	start
	.long	start1
	.long	0,0,0
	.long	0		| initial sector number
	.word	32		| sector count
	.long	0
	.long	start		| where to load
start1:
	movw	#0x2700,sr	| set high priority
	movl	#0x20000-2,sp	| initialize stack pointer
	movl	#_end,d0	| clear bss
	movl	#_edata,a0	| clear bss
1$:	movl	#0,a0@+		| clear bss
	cmpl	d0,a0		| clear bss
	jcs	1$		| clear bss
	jmp	_main
