| sockcall.s -- system call stubs for the UniPlus+ UCB_NET kernel.
|
| Kernel ABI (v1.5/sys/trap.c syscall(), ivec.s vector 32):
|	trap #0, d0 = call number,
|	arguments in a0, d1, a1, d2, a2, d3,
|	on return carry set -> d0 = errno, else d0 = result.
| Verified against the shipped /lib/libc.a: read.o and ptrace.o load
| arguments exactly this way and cerror.o stores d0 in _errno.
| The kernel restores every other register.  C treats d0/d1/a0/a1 as
| scratch (mch.s), so d2 is saved here because the 4-argument calls
| use it.  Every stub loads four arguments; unused ones are ignored.
|
| Alternative without this file: libc's syscall(n, a0, d1, a1, d2, ...)
| e.g. syscall(73, SOCK_STREAM, 0, 0, 0) for socket().

	.text
	.globl	_select, _gethostname, _sethostname, _socket, _accept
	.globl	_connect, _receive, _send, _socketaddr, _netreset
	.globl	_errno

_select:	movl	d2,sp@-
		moveq	#70,d0
		jra	sys
_gethostname:	movl	d2,sp@-
		moveq	#71,d0
		jra	sys
_sethostname:	movl	d2,sp@-
		moveq	#72,d0
		jra	sys
_socket:	movl	d2,sp@-
		moveq	#73,d0
		jra	sys
_accept:	movl	d2,sp@-
		moveq	#74,d0
		jra	sys
_connect:	movl	d2,sp@-
		moveq	#75,d0
		jra	sys
_receive:	movl	d2,sp@-
		moveq	#76,d0
		jra	sys
_send:		movl	d2,sp@-
		moveq	#77,d0
		jra	sys
_socketaddr:	movl	d2,sp@-
		moveq	#78,d0
		jra	sys
_netreset:	movl	d2,sp@-
		moveq	#79,d0
		jra	sys

| stack: sp@(0) saved d2, sp@(4) return pc, sp@(8) first argument
sys:		movl	sp@(8),a0
		movl	sp@(12),d1
		movl	sp@(16),a1
		movl	sp@(20),d2
		trap	#0
		jcs	syserr
		movl	sp@+,d2
		rts
syserr:		movl	sp@+,d2
		movl	d0,_errno
		moveq	#-1,d0
		rts
