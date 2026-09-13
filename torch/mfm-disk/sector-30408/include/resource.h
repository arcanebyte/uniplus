/*****************************************************************************
 *
 * Program:	Resource manager
 * Filename:	resource.h
 *
 * Author:	IJ
 * Date:	9 Dec 85
 *
 * Amendments:	
 *
 *****************************************************************************/

/* ---------- Include Files: -------------------------------------- */

#include <mmi/mmierror.h>

/* ---------- Exported external variables/functions: -------------- */

/* ---------- Imported external variables/functions: -------------- */

/* ---------- Macro definitions and manifest constants: ----------- */

/* SCCS identification string. */
/* @(#) resource.h 1.7@(#) */

#define XXXMAGIC	0x5245534f	/* Resource file header object */

/* Torch reserved objects are in the range 0-5fffffff */

#define RESNULL		0		/* The null (deleted) object */
#define RESFORM		1		/* Form object */
#define RESICON		2		/* Icon object */
#define RESMENU		3		/* Menu object */
#define RESFONT		4		/* Font object */
#define RESCURSOR	8		/* Mouse pointer object */
#define RESHTONE	16		/* Halftone object */
#define RESPALETTE	32		/* Palette resource */
#define RESCONFIG	64		/* Configuration resource */
#define RESTEXT		65		/* Ascii text */
#define RESPOINT	66		/* X, Y coordinate pair */
#define RESRECT		67		/* Rectangle */
#define RESINT		68		/* 32 bit signed integer vector */
#define RESDOUBLE	69		/* double precision vector */
#define RESSTRING	70		/* Single nul terminated string */
#define RESWINPOS	71		/* Window's position & size */
#define RESPADPOS	72		/* Pad information (q.v.) */
#define RESPICTURE	73		/* Graphics escape sequences */
#define RESPRESTEL	74		/* Prestel page */
#define RESFAXT3	75		/* Fax T3 standard */
#define RESFAXT4	76		/* Fax T4 standard */
#define RESNAPLPS	77		/* NAPLPS */
#define RESCEPT		78		/* Cept (Cept3 - intermediate quality)*/
#define RESNEWPAL	79		/* New palette structure */
#define RESFILENAME	80		/* Full Unix pathname rel/absolute */
#define RESENVIRONMENT	81		/* Program environment */
#define RESFIRSTWORD	82		/* Support for 1st word WP package */
#define RESSOUND	83		/* Beeps & spaces */
#define RESKBDMATRIX	84		/* Keyboard matrix */
#define RESTERMIO	85		/* TTY settings */
#define RESDIALOGUE	86		/* Dialogue template */
#define RESANY		-1		/* Wildcard match for resource type */

#ifndef ERROR
#define ERROR		-1		/* Failure return code */
#endif

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

#ifndef SUCCESS
#define SUCCESS		0		/* Successful completion code */
#endif

#define RESPARTIAL	0		/* Partial compress */
#define RESTOTAL	1		/* Total compress */

#define OBJNAMELEN	16		/* 15 characters + '\0' */

#define RESMINSIZE      (sizeof(objhdr))/* Min size for free space subdivide */

typedef int resid;			/* Resource file identifier */
typedef int objid;			/* Object identifier */

#define STDRESOURCES	"/usr/lib/resources"	/* Locn of std resources */

struct pal
{
    unsigned char val[ 16 ];
    int delay;
};

typedef struct pal syspalette[ 2 ];

typedef struct
{
    int mode;
    syspalette pal;
} newpalette;

extern resid ResOpen();
extern resid ResCreate();
extern resid ResFDOPen();
extern int   ResRead();
extern int   ResWrite();
extern objid ResAdd();
extern int   ResClose();
extern objid ResNext(); 
extern objid ResFirst(); 
extern char  *ResTitle();
extern int   ResReTitle();
extern char  *ResObjName();
extern int   ResObjRename();
extern int   ResObjType();
extern int   ResObjSize();

/* Resource manager error codes: (WHAT went wrong) */

#define ERESCREATE	1		/* Failed to create file */
#define ERESOPEN	2		/* Failed to open file */
#define ERESCLOSE	3		/* Failed to close file */
#define ERESSETTIME	4		/* Failed to set timeout */
#define ERESREAD	5		/* Failed to read object */
#define ERESWRITE	6		/* Failed to write object */
#define ERESFREE	7		/* Failed to delete object */
#define ERESSEARCH	8		/* Failed to find object */
#define ERESTITLE	9		/* Failed to get title */
#define ERESRETITLE	10		/* Failed to set title */
#define ERESTYPE	11		/* Failed to get type */
#define ERESNAME	12		/* Failed to get name */
#define ERESRENAME	13		/* Failed to rename */
#define ERESSIZE	14		/* Failed to get size */
#define ERESOFFSET	15		/* Failed to get offset */
#define ERESSETMINSIZE	16		/* Failed to set min obj size */

/* Resource manager reason codes: (WHY it went wrong) */

#define ERESRESID	1		/* invalid resource file identifier */
#define ERESOBJID	2		/* Invalid object identifier */
#define ERESMALLOC	3		/* malloc failed */
#define ERESBADMAGIC	4		/* not a resource file */
#define ERESFIFO	5		/* attempt to seek on pipe */
#define ERESBADMODE	6		/* bad open mode */
#define ERESTIMEOUT	7		/* lock attempt timeout */
#define ERESBADFORMAT	8		/* bad object offset */
#define ERESNOTYPE	9		/* unknown resource type */
#define ERESRANGE	10		/* argument out of range */
#define ERESNOOBJ	11		/* no object of this type found */
#define ERESEOF		12		/* premature end of file */

/*************************** Private information ****************************/
