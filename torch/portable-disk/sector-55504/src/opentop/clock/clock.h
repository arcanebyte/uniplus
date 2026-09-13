/*****************************************************************************
 *
 * Program:	Clock
 * Filename:	clock.h
 *
 * Author:	MPA, DMG
 * Date:	25 June 86
 *
 * Purpose:	To display the time, in analogue or digital form. The
 *		date may also be displayed.
 *
 *****************************************************************************/

/* SCCS identification string. @(#) clock.h 1.3@(#) */

/* ---------- Constants ------------------------------------------- */

#define ERASE     	1		/* Erase or draw the time */
#define DRAWTIME  	0

#define FALSE		0
#define TRUE		1
#ifndef ERROR
#define ERROR		-1
#endif

#define WHITE		BITS_0		/* Colour definitions */
#define RED		BITS_5
#define GREEN		BITS_10
#define BLUE		NULLBITS	/* Same as BITS_15 but faster */

#define LIGHTRED	BITS_4_1
#define LIGHTGREEN	BITS_8_2
#define LIGHTBLUE	BITS_12_3

#define PURPLE		BITS_13_7
#define BROWN		BITS_6_9
#define GREENBLUE	BITS_14_11

#define GREY1		BITS_15_0
#define GREY2		BITS_3
#define DARKGREY	BITS_15_3

#define SECSPERDAY	(60 * 60 * 24)	/* Used by date calculation code */
#define PENWIDTH  	8		/* Pensize used for plotting */
#define PENHEIGHT 	8

#define BIGPENWIDTH  	16		/* Pensize used for plotting */
#define BIGPENHEIGHT 	16

#define RADIUS   	112		/* Clock face radius */
#define NULLM    	0		/* Draw in white */
#define OX       	(16 + RADIUS)	/* X origin */
#define OY       	(16 + RADIUS)	/* Y origin */

#define DISPWIDTH	256		/* Width of face in locals */
#define WINWIDTH	384             /* Width of window in locals */
#define WINHEIGHT	256             /* Height of window in locals */

#define CLOSECTRL	1		/* User id for close box control */


#define MENUENABLED	0x80000000	/* Menu enabled bit */

#define MENUID		1		/* Used to distinguish this menu */

#define ANALOGUE	0		/* Codes for the menu items. */
#define DIGITAL		1		/* They start at zero. Note the gap */
#define DATE		3		/* for the separator line */

#define ENABLEDFLAG	((1 << ANALOGUE)| (1 << DIGITAL) | \
			(1 << DATE) | MENUENABLED )

#define CHECKANALOGUE	(1 << ANALOGUE) /* Analogue initially ticked */


