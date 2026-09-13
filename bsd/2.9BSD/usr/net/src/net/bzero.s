.globl  _bzero
_bzero:
	mov     2(sp),r0
	mov     4(sp),r1
	bit     $1,r0
	bne     1f
	bit     $1,r1
	bne     1f
	asr     r1
2:      clr     (r0)+
	sob     r1,2b
	rts     pc

1:      clrb    (r0)+
	sob     r1,1b
	rts     pc
