/ error = setrgid (rgid)
/	  int rgid;
/
/ error = setegid (egid)
/	  int egid;
/
/ error = setregid(rgid, egid);
/	  int rgid, egid;

.globl	_setrgid
.globl	_setegid
.globl	_setregid
.globl	cerror

_setrgid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	$-1,r1
	br	0f

_setegid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	$-1,r0
	mov	4(r5),r1
	br	0f

_setregid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),r1
0:
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	setregid
.text
