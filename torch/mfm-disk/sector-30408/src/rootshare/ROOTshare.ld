/*
 * load the shared library at 0x800000 (maximum 32K bytes)
 */
MEMORY {
	valid:	org = 0x800000, len = 0x80000
}

/*
 * the pointers to the data and bss areas accessible to routines
 * in the shared library are in known locations in low memory
 * (they are set up by crt0.s)
 */
SH_DATA	= 4;
SH_BSS	= 8;

/*
 * the shared library must be completely in the ".text" segment.
 */
SECTIONS {
	.text: {}
}
