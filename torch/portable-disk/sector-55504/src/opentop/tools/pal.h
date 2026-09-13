/*****************************************************************************
 *
 * Program:	Palette editor
 * Filename:	pal.h
 *
 * Author:	IJ
 * Date:	3 Dec 85
 *
 * Amendments:	
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

/* ---------- Macro definitions and manifest constants: ----------- */

#define NUMSLBARS	4
#define NUMMISC		3

#define REDBAR		0		/* The slide bars */
#define GREENBAR	1
#define BLUEBAR		2
#define DELAYBAR	3

#define MARK		0		/* Offsets into palettes array */
#define FLASH		1
#define TEST		2

#define REDMASK		0xE0		/* Masks to create physical col byte */
#define GREENMASK	0x1C
#define BLUEMASK	0x03
#define REDSHIFT	5
#define GREENSHIFT	2
#define BLUESHIFT	0

#define PALBKGND	WHITE
#define CTBKGND		BLUE

#define PALBORDER	8
#define PALSPACING	40
#define TEXTWIDTH	160
#define CTRLSIZE	80
#define SLBARHEIGHT	80
#define SLBARWIDTH	(8 * CTRLSIZE )

#define PALTITLE	"Palette resource file"
#define CONFRESFN	".configuration"	/* Configuration filename */
#define MAXFILNM	256			/* Max pathname length */

/* ---------- Data structure definitions: ------------------------- */


