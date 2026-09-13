/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Control Definitions                       *
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 * CREATED   : 27th October 1985                                              *
 *                                                                            *
 * PURPOSE   :                                                                *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

typedef unsigned long w_flags;
typedef int (*w_intfn)();

/*                                                            *****************
                                                              *               *
                                                              * W_HTONES      *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure (array) of pointers to control halftone handle numbers.           |
-------------------------------------------------------------------------------
*/

typedef struct
{
   short	fpat;
   short	bpat;
   short	clrpat;
   short	framepat;
   short	labelpat;
   short	shadepat;
   short	hilitepat;
} w_htones;

/*                                                            *****************
                                                              *               *
                                                              * W_GFNS        *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure (array) of pointers to control Graphic functions.                 |
-------------------------------------------------------------------------------
*/

typedef struct
{
   w_intfn	Draw;
   w_intfn	Clear;
   w_intfn	Shade;
   w_intfn	UnShade;
   w_intfn	HiLite;
   w_intfn	LoLite;
   w_intfn	Frame;
   w_intfn	UnFrame;
   w_intfn	Label;
   w_intfn	UnLabel;
} w_gfns;

/*                                                            *****************
                                                              *               *
                                                              * W_AFNS        *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure (array) of pointers to control Action functions.                  |
-------------------------------------------------------------------------------
*/

typedef struct
{
   w_intfn	LMDAct;		/* Left mouse-down action */
   w_intfn	RMDAct;		/* Right mouse-down action */
   w_intfn	LMUAct;		/* Left mouse-up action */
   w_intfn	RMUAct;		/* Right mouse-up action */
   w_intfn	MUPDAct;	/* Mouse update (movement) action */
} w_afns;

/*                                                            *****************
                                                              *               *
                                                              * INTTAB        *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Dynamic table of integers.                                                  |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int	size;		/* number of entries - 0..size-1 */
   int tabsize;		/* number of spaces currently allocated in memory */
   int *tab;		/* the entries themselves - malloc()ed space */
} inttab;

/*                                                            *****************
                                                              *               *
                                                              * W_CSTRUCT     *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure containing information relevent to a particular control.          |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int		fd;		/* file descriptor of the owning window */
   inttab	*wctrltab;	/* pointer to owning window's control table */
   int		userid;		/* user - specified integer control id */
   w_flags	flags;		/* visible and other state flags */
   short	hidesema;	/* depth to which control is hidden */
   rectangle	position;	/* overall size of control (inside frame) */
   rectangle	absareg;	/* active region in absolute local coords */
   short	value;		/* 0..limit */
   short	limit;		/* 1..MAXINT */
   char		*label;		/* label to be printed on screen */
   int		uflags;		/* flags for specialised actions */
   short	fmhan;		/* form handle number used to draw control */
   w_htones	htones;		/* halftone handles for Graphic functions */
   w_gfns	gfns;		/* Graphic functions for control */
   w_afns	afns;		/* Action functions for control */
   w_intfn	ValFn;		/* Value update function */
   w_intfn	Event;		/* the user-defined event function */
} w_cstruct;

/*                                                            *****************
                                                              *               *
                                                              * W_CTYPESTRUCT *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Structure containing the default information used to open a new control.    |
-------------------------------------------------------------------------------
*/

typedef struct
{
   w_htones	htones;			/* default halftones for drawing */
   w_gfns	gfns;			/* default Graphic functions */
   w_afns	afns;			/* type-defined Action functions */
   w_intfn	ValFn;			/* type-defined Value update */
} w_ctypestruct;

/* possible flags for w_cstruct */

#define W_CLIBLOCK	(1 << 0)	/* Window Library handling it ! */
#define W_CVISIBLE	(1 << 1)	/* actually drawn in it's window */
#define W_CDRAWN	(1 << 2)	/* main part is to be shown */
#define W_CFRAMED	(1 << 3)	/* frame is to be shown */
#define W_CLABELLED	(1 << 4)	/* label is to be shown */
#define W_CDISABLED	(1 << 5)	/* not mouse-sensitive so shaded */
#define W_CACTIVE	(1 << 6)	/* mouse-active so hilited */

/* useful type cast definitions */

#define CPTRCAST	(w_cstruct *)		/* ptr to a ctrl structure */
#define CTPTRCAST	(w_ctypestruct *)	/* ptr to a ctrl type struct */

/*                                                            *****************
                                                              *               *
                                                              * W_CTRLINFO    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Information about a control passed to an event handler.                     |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int		id;
   int		userid;
   short	inareg;
   int		uflags;
} w_ctrlinfo;

/*                                                            *****************
                                                              *               *
                                                              * W_EVSTRUCT    *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| 'Event' record for the action functions assiciated with controls.           |
-------------------------------------------------------------------------------
*/

typedef struct
{
   point	where;
   word		what;
   Dword	message;
   int		when;
   int		modifiers;
   int		fd;
   w_ctrlinfo	ctrl;
} w_EventRec;

#ifdef NEOPOLITAN
#   define W_NEOPOLITAN		3
#   undef  NEOPOLITAN
#   define NEOPOLITAN		(WUseNeop())
#endif

#define W_FCTYPEID	1000		/* first static control type id */
#define W_FCTRLID	1500		/* first control id */

#define CLICKBUT	W_FCTYPEID	/* Click button type id */
#define TOGBUT		(W_FCTYPEID+1)	/* toggle button type */
#define PUSHBUT		(W_FCTYPEID+2)	/* push button type id */
#define VSLIDEBAR	(W_FCTYPEID+3)	/* vertical slide bar type id */
#define HSLIDEBAR	(W_FCTYPEID+4)	/* horizontal slide bar type id */

#define UPBUT		0x010000
#define DOWNBUT		0x020000
#define LEFTBUT		0x040000
#define RIGHTBUT	0x080000
#define VSBAR		0x100000
#define HSBAR		0x200000
#define GROWBUT		0x400000

#define VSCRLBAR	(UPBUT | VSBAR | DOWNBUT)
#define HSCRLBAR	(LEFTBUT | HSBAR | RIGHTBUT)

#define SCROLLEVENT	(0x101)		/* derived event number n is 256 + n */

#define UPSCRL		1		/* message field contents for */
#define DOWNSCRL	2		/* the derived event SCROLLEVENT */
#define LEFTSCRL	3
#define RIGHTSCRL	4
#define VERTSCRL	5
#define HORIZSCRL	6

#define W_SCTRLMASK	(VSCRLBAR | HSCRLBAR | GROWBUT)

#define C_MUPDATE	(1 << 0)


