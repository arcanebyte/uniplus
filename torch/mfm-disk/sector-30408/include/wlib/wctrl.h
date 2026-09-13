/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Control User Include File                 *
 *                                                                            *
 * COPYRIGHT : (c) 1986 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) wctrl.h 1.1@(#)" */

typedef int (*w_intfn)();

/*                                                            *****************
                                                              *               *
                                                              * W_HTONES      *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Halftone pattern handle numbers to be used when drawing the control.        |
| The names indicate the preferred uses and are adhered to by all built-in    |
| control types.							      |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int		fore;		/* general foreground */
   int		back;		/* general background */
   int		clear;		/* clearing when hidden or undrawn */
   int		frame;		/* frame around the position */
   int		label;		/* text within the position or active region */
   int		shade;		/* shading when disabled */
   int		hilight;	/* hilighting when active (may be tracking) */
   int		actr;		/* active region */
   int		shadow;		/* drop shadow below and to right of frame */
} w_htones;

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
   w_intfn	AbortAct;	/* called to abort an action and tidy up */
} w_afns;

/*                                                            *****************
                                                              *               *
                                                              * W_UFNS        *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Update functions, according to control type, to replace the standard ones.  |
-------------------------------------------------------------------------------
*/

typedef struct
{
   w_intfn	PutCPos;
   w_intfn	PutCValue;
   w_intfn	PutCLimit;
   w_intfn	PutCActR;
   w_intfn	PutCLabel;
   w_intfn	PutCForm;
   w_intfn	PutCPats;
} w_ufns;

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
   int		references;		/* how many controls open */
   int		initstate;		/* to be or-ed with the initial state */
   int		statemask;		/* which state changes cause re-draw */
   int		changesmask;		/* which changes cause re-draw */
   w_htones	htones;			/* default halftones for drawing */
   w_intfn	Draw;			/* type-defined graphic function */
   w_afns	Act;			/* type-defined action functions */
   w_ufns	Update;			/* type-defined update functions */
} w_ctypestruct;

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
| The information necessary to record the overall state and change in state   |
| of a control.                                                               |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int		 id;		/* the control's identifier */
   int		 fd;		/* parent window */
   inttab	 *wctrltab;	/* parent window's control table */
   int		 type;		/* the control's type identifier */
   w_ctypestruct *typeptr;	/* pointer to the type definition */
   w_intfn	 Event;		/* user-defined event handling function */
   int		 flags;		/* defining activity, locked etc. */
   int		 hidesema;	/* depth to which control is hidden */
   rectangle	 posn;		/* overall position rectangle (inside frame) */
   int		 statemask;	/* all the state changes causing a re-draw */
   int		 statechanges;	/* flags indicating changes in state below */
   int		 state;		/* flags indicating framed, hilighted etc */
   int		 changesmask;	/* all the changes causing a re-draw */
   int		 changes;	/* flags indicating changes in following */
   int		 userid;	/* user-defined identifier for grouping etc. */
   rectangle	 actr;		/* active region within position */
   int		 value;		/* value 0 thru limit */
   int		 limit;		/* 0 thru largest positive int */
   char		 *label;	/* pointer to malloc()ed label */
   int		 fmhan;		/* form handle plotted in active region */
   w_htones	 htones;	/* halftones */
} w_cstruct;


/* possible flags for w_cstruct */

#define	CF_AUTOMATIC	(1 << 0)	/* window library handling it */
#define	CF_LOCKED	(1 << 1)	/* events disabled */
#define	CF_AUTOREDRAW	(1 << 2)	/* automatic re-drawing when changed */

/* possible state for w_cstruct */

#define	CS_VISIBLE	(1 << 0)	/* drawn in the window */
#define	CS_DISABLED	(1 << 1)	/* shaded */
#define	CS_ACTIVE	(1 << 2)	/* hilighted */
#define	CS_FRAMED	(1 << 3)	/* has a frame */
#define	CS_DROPFRAME	(1 << 4)	/* and a drop shadow */
#define	CS_ACTRFRAMED	(1 << 5)	/* frame the active region */
#define	CS_MUPDATE	(1 << 6)	/* mouse update events required */
#define	CS_LMD		(1 << 7)	/* left mouse down events required */
#define	CS_RMD		(1 << 8)	/* right mouse down events required */

/* all the state flags which may be initialised by the user at OpCtrl() */

#define CS_INITFLAGS	(~(CS_VISIBLE | CS_DISABLED | CS_ACTIVE))

/* possible changes flags for w_cstruct */

#define	CC_USERID	(1 << 0)	/* user id changed */
#define CC_ACTR		(1 << 1)	/* active region changed */
#define	CC_VALUE	(1 << 2)	/* value changed */
#define	CC_LIMIT	(1 << 3)	/* limit changed */
#define	CC_LABEL	(1 << 4)	/* label changed */
#define	CC_FORM		(1 << 5)	/* form handle no. changed */
#define	CC_HTONES	(1 << 6)	/* half tones changed */

/* useful type cast definitions */

#define CPTRCAST	(w_cstruct *)		/* ptr to a ctrl structure */
#define CTPTRCAST	(w_ctypestruct *)	/* ptr to a ctrl type struct */

/* control type identifiers */

#define W_FCTYPEID	1000		/* first static control type id */
#define W_FCTRLID	1500		/* first control id */

#define CLICKBUT	(W_FCTYPEID+0)	/* click button type id */
#define TOGBUT		(W_FCTYPEID+1)	/* toggle button type */
#define PUSHBUT		(W_FCTYPEID+2)	/* push button type id */
#define VSLIDEBAR	(W_FCTYPEID+3)	/* vertical slide bar type id */
#define HSLIDEBAR	(W_FCTYPEID+4)	/* horizontal slide bar type id */
#define ONBUT		(W_FCTYPEID+5)	/* on button type id */

