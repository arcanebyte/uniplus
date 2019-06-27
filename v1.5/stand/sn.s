	.text
	.globl	_sn
_sn:
	moveml	#0xFFFE,.rsav		| all but d0
	movl	#.buf,a0		| where to put serial number
	jsr	0xFE00C4
	bcc	.fail
	movl	#.buf+2,d0
	bra	.ok
.fail:	clrl	d0
.ok:	moveml	.rsav,#0xFFFE
	rts
| end
	.data
.buf:
	.space	100
.rsav:
	.space	64
