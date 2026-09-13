/ should be hostid instead of hstid, but ...
.globl  _sethstid
_sethstid:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4.(r5),0f
	mov     6.(r5),0f+2
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	clr	r0
	rts	pc
.data
9:
	sys	sethstid; 0:..; ..
.text
