/ error = gethostid();
/ should be hostid instead of hstid, but ...

.globl  _gethstid
_gethstid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	gethstid
.text
