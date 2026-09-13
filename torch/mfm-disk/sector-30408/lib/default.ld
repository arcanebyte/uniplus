/*
 *	Loader command file for producing shared executables
 *	with text relocation at address 0x0 and data
 *	relocation at 0x400000.
 *
 *	The default loader behavior is to produce shared
 *	executables with the appropriate magic number in
 *	the file header.  When using this file the -N
 *	option to ld MUST NOT be used.  The -N option
 *	is not the same as the old -n option!
 *
 *	Text section is bound to (starts at) 0x0.
 *
 *	Data and bss sections are bound to any multiple of 0x400000.
 */

SECTIONS 
{
	.text:{}
	GROUP ALIGN(0x400000):
	{
		.data:{}
		.bss:{}
	}
}
