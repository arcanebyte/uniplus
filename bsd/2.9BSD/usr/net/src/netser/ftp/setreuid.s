/*	@(#)setreuid.s	1.1	SCCS id keyword	*/
/ C library -- setreuid

/ error = setuid (uid)
/ error = setruid (ruid)
/ error = seteuid (euid)
/ error = setreuid(ruid, euid);

.globl	setuid
.globl	_setruid
.globl	_seteuid
.globl	_setreuid
.globl	cerror

_setuid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	r0,r1
	br	0f

_setruid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	$-1,r1
	br	0f

_seteuid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	$-1,r0
	mov	4(r5),r1
	br	0f

_setreuid:
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
	sys	setreuid
.text
