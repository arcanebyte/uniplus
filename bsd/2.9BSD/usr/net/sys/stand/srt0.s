/ Startup code for standalone utilities
/ wfj- mod's to allow non sep i/d machines, error recovery
/      note that the bootstrap passes the cputype through in
/      r0.

/ non-UNIX instructions
mfpi	= 6500^tst
stst	= 170300^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
wait	= 1
rtt	= 6
reset	= 5
/ trap	= 104400

PS	= 177776

.globl	_end
.globl	_main, __rtt
.globl	_edata
	jmp	start

/
/ trap vectors
/
	trap;340	/ bus error
	trap;341	/ illegal instruction
	trap;342	/ BPT
	trap;343	/ IOT
	trap;344	/ POWER FAIL
	trap;345	/ EMT
tvec:
	start;346	/ TRAP
.=400^.
.text


start:
	mov	$340,*$PS
	mov	$trap,tvec
/
/ restore what kind of cpu we are running on
	mov	r0,*$_cputype	/ assume that the boot left this in r0
/ check if there is a switch register
	mov	$1f,nofault
	tst	CSW
	incb	_haveCSW
1:
	clr	nofault
	mov	$157776,sp
	mov	$_edata,r0
	mov	$_end,r1
	sub	r0,r1
	inc	r1
	clc
	ror	r1
1:
	clr	(r0)+
	sob	r1,1b
	jsr	pc,_main

/ fix up stack to point at trap ps-pc pair,
/ located at bottom of memory.
/ so we can return to the bootstrap

__rtt:
	mov	$140000,sp
	rtt				/ we hope!
	br	.


.globl	_trap
trap:
	mov	*$PS,-(sp)
	mov	r0,-(sp)
	mov	r1,-(sp)
	tst	nofault
	bne	3f
2:	jsr	pc,_trap
	mov	(sp)+,r1
	mov	(sp)+,r0
	tst	(sp)+
	rtt
3:	mov	(sp)+,r1
	mov	(sp)+,r0
	tst	(sp)+
	mov	nofault,(sp)
	rtt

.data
.globl	_cputype, _haveCSW

nofault:	.=.+2	/ where to go on predicted trap
_cputype:	.=.+2	/ cpu type (currently 24,40,45 or 70)
_haveCSW:	.=.+1	/ one if there is a switch register
KISA6 = 172354
KISA7 = 172356
KDSA6 = 172374
UBMAP = 170200
CSW   = 177570
