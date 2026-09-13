| kpad.s -- DIAGNOSTIC, not an original UniSoft file (2026).
| 316 bytes of padding linked after unix.o so that the device and
| configuration code in unix.pad sits at the same addresses as in the
| working UniPlus 1.4 /unix (the V.1.5+ code built with the Lisa cc is
| 316 bytes shorter).  Used to test whether the root-disk panic depends
| on code addresses.
	.text
	.globl	_kpad
_kpad:
	.space	316
