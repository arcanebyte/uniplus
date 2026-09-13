/*****************************************************************************
 *
 * Program:	General
 * Filename:	std.h
 *
 * Author:	IJ
 * Date:	30 Oct 85
 *
 * Amendments:	
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

/* ---------- Macro definitions and manifest constants: ----------- */

#ifndef STDDEFS

#define STDDEFS		1

#define FALSE		0
#define TRUE		1
#ifndef ERROR
#define ERROR		-1
#endif

#define ISONORM_8	SYSFONT
#define FLASHPAL	1
#define MARKPAL		2

/* The following macros are of the form DBGn, where n is the number of */
/* arguments that the macro expects. These macros may be compiled in   */
/* when DEBUG is defined, or totally removed for release, when DEBUG   */
/* is not defined. Selective run-time debugging information is         */
/* controlled by the value of the 'debug' bit mask. This is an external*/
/* variable that is declared (as type 'int') and modified by the user. */

#ifdef DEBUG

#   define DBG0( type, format )			{if(debug&type)\
						fprintf(stderr,format);}

#   define DBG1( type, format, a1 )		{if(debug&type)\
						fprintf(stderr,format, a1);}

#   define DBG2( type, format, a1, a2 )		{if(debug&type)\
						fprintf(stderr,format, a1, a2 );}
 
#   define DBG3( type, format, a1, a2, a3 )	{if(debug&type)\
						fprintf(stderr,format, a1, a2, a3 );}

#   define DBG4( type, format, a1, a2, a3, a4 )	{if(debug&type)\
						fprintf(stderr,format,a1,a2,a3,a4);}
#else

#   define DBG0( type, format )			
#   define DBG1( type, format, a1 )	
#   define DBG2( type, format, a1, a2 )
#   define DBG3( type, format, a1, a2, a3 )	
#   define DBG4( type, format, a1, a2, a3, a4 )

#endif

extern int debug;		/* All C files now get this */

/* The following macro allows more transparent halftone initialisation */

#define W		0
#define R		1
#define G		2
#define B		3

#define HTLINE(a,b,c,d,e,f,g,h)		( (a)<<14 | (b)<<12 | (c)<<10 | (d)<<8 \
					   | (e)<<6 | (f)<<4 | (g)<<2 | h )

/* The following manifest declarations assume the usual logical to physical */
/* colour mapping, i.e. 0 = White, 1 = Red, 2 = Green and 3 = Blue	    */

#define WHITE		BITS_0
#define RED		BITS_5
#define GREEN		BITS_10
#define BLUE		NULLBITS	/* Used to be BITS_15 */

#define LIGHTRED	BITS_4_1
#define LIGHTGREEN	BITS_8_2
#define LIGHTBLUE	BITS_12_3

#define PURPLE		BITS_13_7
#define BROWN		BITS_6_9
#define GREENBLUE	BITS_14_11

#define GREY1		BITS_15_0
#define GREY2		BITS_3
#define DARKGREY	BITS_15_3

#define IOCODE(e)	('J' << 8 | (e))	/* ioctl code */

#endif  STDDEFS
