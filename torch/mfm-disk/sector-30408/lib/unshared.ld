/*
 *	Loader command file for producing unshared executables
 *	with text relocation at address 0.
 *
 *	Text starts at 0, data and bss immediately thereafter.
 *
 *	Note that this only tells ld where to load the various
 *	sections.  The -N flag must be used to tell ld that
 *	the magic number for unshared is to be placed in the
 *	file header.
 */

-N

SECTIONS
{
	.text: {}
	.data: {}
	.bss: {}
}
