/
/	SCCS id	@(#)M.s	1.7 (Berkeley)	7/11/83
/
/ Startup code for two-stage bootstrap
/ With support for UCB_AUTOBOOT
/ Supports 11/40, 11/45, 11/70, 11/23, 11/23+I/O map (11/24)
/ and similar machines

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
halt	= 0
reset	= 5
systrap = 104400

/  The boot options and device are placed in the last SZFLAGS bytes
/  at the end of core by the kernel if this is an autoboot.
/  The first-stage boot will leave them in registers for us;
/  we clobber possible old flags so that they don't get re-used.
ENDCORE=	160000		/ end of core, mem. management off
SZFLAGS=	6		/ size of boot flags
BOOTOPTS=	2		/ location of options, bytes below ENDCORE
BOOTDEV=	4
CHECKWORD=	6

.globl	_end
.globl	_main,_ubmapset
	jmp	start

/
/ trap vectors
/
	trap;340	/ bus error -- grok!
	trap;341	/ illegal instruction
	trap;342	/ BPT
	trap;343	/ IOT
	trap;344	/ POWER FAIL
	trap;345	/ EMT
tvec:
	start;346	/ TRAP
.=400^.


start:
	reset
	mov	$340,PS
	mov	$140100,sp

/save boot options, if present
	mov	r4,_bootopts
	mov	r3,_bootdev
	mov	r2,_checkword
/clobber any boot flags left in memory
	clr	ENDCORE-BOOTOPTS
	clr	ENDCORE-BOOTDEV
	clr	ENDCORE-CHECKWORD
/
/ determine what kind of cpu we are running on
/ first, check switches. if they are 40, 24, 45 or 70
/ set appropriately
/
	clrb	_sep_id
	clrb	_ubmap
	clrb	_haveCSW
	mov	$2f,nofault	/ check if we even have switches!
	tst	*$SWREG
	clr	nofault		/ apparently we do
	incb	_haveCSW
	mov	$40.,r0
	cmp	*$SWREG,$40
	beq	gotcha
	cmp	*$SWREG,$24
	bne	1f
	mov	$24.,r0
	incb	_ubmap
	jbr	gotcha
1:
	cmp	*$SWREG,$45
	bne	1f
	mov	$45.,r0
	incb	_sep_id
	jbr	gotcha
1:
	cmp	*$SWREG,$70
	bne	2f
	mov	$70.,r0
	incb	_sep_id
	incb	_ubmap
	jbr	gotcha
/
/ if we can't find out from switches,
/ explore and see what we find
/
2:
	mov	$40.,r0		/ assume 11/40
	mov	$2f,nofault
	mov	*$KDSA6,r1	/ then we have sep i/d
	incb	_sep_id
	mov	$45.,r0
	mov	$1f,nofault
	mov	*$UBMAP,r1	/ then we have a unibus map
	incb	_ubmap
	mov	$70.,r0
	br	1f
2:
	mov	$1f,nofault
	mov	*$UBMAP,r1	/ then we have a unibus map
	incb	_ubmap
	mov	$24.,r0		/ unibus map, no sep. I/D = 24
1:	clr	nofault
gotcha:
	mov	r0,_cputype

/
/	Set kernel I space registers to physical 0 and I/O page
/
	clr	r1
	mov	$77406, r2
	mov	$KISA0, r3
	mov	$KISD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

/
/	Set user I space registers to physical 128kb and I/O page
/
	mov	$6000, r1	/ 06000 = 192*1024/64
	mov	$UISA0, r3
	mov	$UISD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

/
/	If 11/40 class processor, only need set the I space registers
/
	tstb	_sep_id
	jeq	1f

/
/	Set kernel D space registers to physical 0 and I/O page
/
	clr	r1
	mov	$KDSA0, r3
	mov	$KDSD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

/
/	Set user D space registers to physical 128kb and I/O page
/
	mov	$6000, r1	/ 06000 = 192*1024/64
	mov	$UDSA0, r3
	mov	$UDSD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

1:
/ enable map
	clrb	_ksep
	tstb	_ubmap
	beq	2f
	jsr	pc,_ubmapset
	tstb	_sep_id
	bne	3f
	mov	$60,SSR3	/ 22-bit map, no separation
	br	1f
3:
	mov	$65,SSR3	/ 22-bit map, sep. I/D user and kernel
	movb	$1,_ksep
	cmp	_cputype,$70.
	jne	1f
	mov	$3,MSCR
	br	1f
2:
	tstb	_sep_id		/ no ubmap; sep_id?
	beq	1f
	mov	$5,SSR3
	movb	$1,_ksep
1:
	mov	$30340,PS
	inc	SSR0


/ copy program to user I space
	mov	$_end,r0
	asr	r0
	clr	r1
1:
	mov	(r1),-(sp)
	mtpi	(r1)+
	sob	r0,1b


/ continue execution in user space copy
	mov	$140004,sp
	tstb	_sep_id
	bne	1f
	clr	*$KISA6
	br	2f
1:
	clr	*$KDSA6
2:	mov	$140340,-(sp)
	mov	$user,-(sp)
	rtt
user:
/ clear bss
	mov	$_edata,r0
	mov	$_end,r1
	sub	r0,r1
	inc	r1
	clc
	ror	r1
1:
	clr	(r0)+
	sob	r1,1b
	mov	$_end+512.,sp
	mov	sp,r5

	jsr	pc,_main
	mov	_cputype,r0
	mov	_bootopts,r4
	mov	r4,r2
	com	r2		/ checkword
	systrap

	br	user

setseg:
	mov	$8,r0
1:
	mov	r1,(r3)+
	add	$200,r1
	mov	r2,(r4)+
	sob	r0,1b
	rts	pc

.globl	_setseg
_setseg:
	mov	2(sp),r1
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	tstb	_ksep
	bne	1f
	mov	$IO,-(r3)
1:
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

.globl	_setnosep
_setnosep:
	bic	$4,SSR3	/ turn off kernel i/d sep
	clrb	_ksep
	rts pc

.globl	_setsep
_setsep:
	bis	$4,SSR3	/ turn on kernel i/d sep (if not already)
	movb	$1,_ksep
	rts pc

/  Relocate higher in memory for large kernel.
.globl	_reloc, _segflag
_reloc:
	jsr	r5, csv
	mov	$2f, nofault
	/  Set Kernel I to point to new area
	mov	$6000, r1			/ 192 KB
	mov	$77406,r2
	mov	$KISA0, r3
	mov	$KISD0, r4
	jsr	pc, setseg
	/  Copy to new area
	clr	r0
	mov	$28.*1024., r1			/ 28 K words
1:
	mov	(r0), -(sp)
	mtpi	(r0)+
	sob	r1, 1b
	clr	nofault

	/  Repoint kernel I at 0.
	clr	-(sp)
	jsr	pc, _setseg
	tst	(sp)+

	/  Point User D at new area
	tstb	_sep_id
	beq	1f
	mov	$6000, r1			/ 192 KB
	mov	$UDSA0, r3
	mov	$UDSD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

	/  Now User I-- execution switches to new copy.
1:
	mov	$6000, r1			/ 192 KB
	mov	$UISA0, r3
	mov	$UISD0, r4
	mov	$7,r0
1:
	mov	r1,(r3)+
	add	$200,r1
	mov	r2,(r4)+
	sob	r0,1b

	mov	$3, _segflag			/ new extension bits
	clr	r0				/ success
	jmp	cret

	/  Trap here on bus error (not enough memory)
2:
	mov	$-1, r0
	jmp	cret

/ clrseg(addr,count)
.globl	_clrseg
_clrseg:
	mov	4(sp),r0
	beq	2f
	asr	r0
	bic	$!77777,r0
	mov	2(sp),r1
1:
	clr	-(sp)
	mtpi	(r1)+
	sob	r0,1b
2:
	rts	pc


/ mtpd(word,addr)
.globl	_mtpd
_mtpd:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpd	(r0)+
	rts	pc

/ mtpi(word,addr)
.globl	_mtpi
_mtpi:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpi	(r0)+
	rts	pc

.globl	__rtt
__rtt:
	halt

.globl	_trap

trap:
	mov	*$PS,-(sp)
	mov	r0,-(sp)
	mov	r1,-(sp)
	tst	nofault
	bne	3f
	jsr	pc,_trap
	mov	(sp)+,r1
	mov	(sp)+,r0
	tst	(sp)+
	rtt
3:	mov	(sp)+,r1
	mov	(sp)+,r0
	tst	(sp)+
	mov	nofault,(sp)
	rtt

PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
KDSA0	= 172360
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD5	= 172332
SISA0	= 172240
SISA1	= 172242
SISD0	= 172200
SISD1	= 172202
UISA0	= 177640
UISD0	= 177600
UDSA0	= 177660
UDSD0	= 177620
MSCR	= 017777746	/ 11/70 memory control register
IO	= 177600
SWREG	= 177570
UBMAP	= 170200


.data
.globl	_cputype
.globl	_ksep, _sep_id, _ubmap, _haveCSW
.globl	_bootopts, _bootdev, _checkword

nofault:	.=.+2	/ where to go on predicted trap
_cputype:	.=.+2	/ cpu type (currently 40,45 or 70)
_sep_id:	.=.+1	/ 1 if we have separate I and D
_ubmap:		.=.+1	/ 1 if we have a unibus map
_haveCSW:	.=.+1	/ 1 if we have a console switch register
_ksep:		.=.+1	/ 1 if kernel mode has sep I/D enabled
_bootopts:	.=.+2	/ flags if an autoboot
_bootdev:	.=.+2	/ device to get unix from, if not RB_ASKNAME
_checkword:	.=.+2	/ saved r2, complement of bootopts if an autoboot
