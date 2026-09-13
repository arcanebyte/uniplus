			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 1.00\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *   0.11		YES and NO flags wrong values
 * RELEASED
 *
 */

/************************************************************/
/*							    */
/* YAWN - a yes/no answer gatherer			    */
/*							    */
/* Displays a warning sign and a message in its own window  */
/* and waits for the user to click either the tick (yes) or */
/* cross (no) box. The program returns code YES for the     */
/* tick and NO for the cross. If the program is unable to   */
/* return YES or NO because, for example, it cannot open a  */
/* window, it returns FAIL.				    */
/*							    */
/************************************************************/

#define YES 0
#define NO 1
#define TRUE 1
#define FALSE 0
#define FAIL 2
#define LINELENGTH 30
#define NOBOX 0
#define TICKBOX 1
#define CROSSBOX 2

#define iocode(x) (('J' << 8) || x)

/* text output box */
#define OBOX_X 180
#define OBOX_Y 12
#define OBOX_W 240
#define OBOX_H 32
/* triangle sign box */
#define TRIBOX_X 10
#define TRIBOX_Y 10
#define TRIBOX_W 120
#define TRIBOX_H 60
/* window size*/
#define WIND_X 40
#define WIND_Y 40
#define WIND_W 450
#define WIND_H 80
/* tick box */
#define TBOX_X 238
#define TBOX_Y 50
#define TBOX_W 40
#define TBOX_H 24
/* cross box */
#define CBOX_X 330
#define CBOX_Y 50
#define CBOX_W 40
#define CBOX_H 24

#define LETTERX (OBOX_X * (1 << xshift))
#define LETTERY (OBOX_Y * (1 << yshift) + LETTERDEPTH)
#define LETTERDEPTH (8 * (1 << yshift))

#include <termio.h>
#include <wlib.h>
#include "forms.c"
#include "icons.c"
#include <string.h>

static rectangle trirect  = {TRIBOX_X,TRIBOX_Y,TRIBOX_W,TRIBOX_H};
static rectangle tickrect = {TBOX_X,TBOX_Y,TBOX_W,TBOX_H};
static rectangle crossrect = {CBOX_X,CBOX_Y,CBOX_W,CBOX_H};
static rectangle questrect = {WIND_X,WIND_Y,WIND_W,WIND_H};
/* Boxes in local coordinates for PtInRect() */
static rectangle tickbox;
static rectangle crossbox;
 
static point zoompt = {0,0};
static int aspecth[] = {2,3,2,3,2,3};
static int aspectv[] = {3,3,2,2,3,3};
static int screenmode;
static int xshift,yshift;

static char textline[LINELENGTH+1],outline[LINELENGTH+1],cword[LINELENGTH+1];
static char *a,*w,*l;
static int count,linecount,carg;
static short filling = TRUE;	/* TRUE if line is to be justified */

static short inverted = 0;
static short invbox = NOBOX;
static int whand,trihand,tickhand,crosshand;
static int wae_flags = 1;
extern int werrno;
extern FILE *fs;
extern MDown(),MUp();

int (*wev_procs[NUMPROCS]) () =
{
    NULL,	/* ignore menu select event */
    NULL,	/* ignore close window event */
    MDown,	/* mouse left button down event */
    NULL,	/* ignore right button down event */
    MUp		/* mouse left button up event */
};

main(argc,argv)
int argc;
char *argv[];
{
	/* check that sufficient arguments have been supplied */
	if (argc < 3){
	    fprintf(stderr,"Usage : yawn <title> <message>...\n");
	    exit(FAIL);
	}

        whand = WOpen(0,WOANYUNIQUE,&questrect,STRAWBERRY,&zoompt,
		      wev_procs,wae_flags);
	if (whand == -1){
	    fprintf(stderr,"yawn: unable to open window, error %d\n",
                    werrno);
	    shutdown(FAIL);
	}

	/* get the current screen mode */
	ioctl(whand,iocode(ERSMODE),&screenmode);
	/* find screen scaling */
	xshift = aspecth[screenmode];
	yshift = aspectv[screenmode];
	/* find local coordinate values for the icon boxes */
	convert(&tickrect,&tickbox);
	convert(&crossrect,&crossbox);

	/* set the window title */
	TitleBits(SHOWNAME);
	SetTitle(argv[1]);
	ShowTitle();

	/* draw the boxes round the icons */
	SetFPat(BITS_5);
	PenMode(3);
	PenSize(1 << xshift,1 << yshift);
	OFrameRect(&tickbox);
	OFrameRect(&crossbox);
	Flush();
	SetFPat(NULLBITS);

	HideCursor();
	trihand = DoOpenForm(&triform,"triform");
	tickhand = DoOpenForm(&tickform,"tickform");
	crosshand = DoOpenForm(&crossform,"crossform");

	Blat(&triform,&trirect,trihand);
	Blat(&tickform,&tickrect,tickhand);
	Blat(&crossform,&crossrect,crosshand);

/*      print message */
	count = 0;
	linecount = 0;
	w = cword;
	l = textline;

	for (carg = 2;carg < argc;carg++){
	    a = argv[carg];

	    do {
		/* get next word */
		while ((*a != ' ') && (*a != '\0'))
		    *w++ = *a++;
		*w++ = '\0';

 		/* see if line full up yet */
		if ((count +  1 + strlen(cword)) > LINELENGTH)
		    PrintLine();

		if (textline[0] != '\0'){
		    strcat(textline," ");
		count++;
		}
	        strcat(l,cword);
		count += strlen(cword);
	        w = cword;

		while (*a == ' ')
		    a++;
	    } while (*a != '\0');
	}
	/* print any part line which is left */
	if (w != cword){
            strcat(l,cword);
        }
	filling = FALSE;
	PrintLine();

/*	wait for mouse down event */
	for(;;)
	    pause();
}

convert(r1,r2)
rectangle *r1,*r2;
/* make r2 the local coordinates for r1 */
{
	r2->x = r1->x << xshift;
	r2->y = r1->y << yshift;
	r2->width = r1->width << xshift;
	r2->height = r1->height << yshift;
}

Blat(fptr,rptr,formhand)
form *fptr;
rectangle *rptr;
int formhand;
{
        PenSize(fptr->width << xshift,fptr->height << yshift);
	PenForm(formhand);
	MoveTo(rptr->x << xshift,rptr->y << yshift);
	DropPen();
	PlotPoint();
	RaisePen();
}

shutdown(rc)
int rc;
{
	if (tickhand != -1)  ClForm(tickhand);
	if (crosshand != -1) ClForm(crosshand);
	if (trihand != -1)   ClForm(trihand);
	if (whand != -1)     WClose(whand);

	exit(rc);
}

int DoOpenForm(fptr,name)
    form *fptr;
    char *name;
{
	int handle;

	handle = OpForm(fptr);
	if (handle != -1)
            return(handle);

        fprintf(stderr,"yawn: unable to open form %s\n",name);
        shutdown(FAIL);
}

MDown(ep)
EventRec *ep;
{
	if (inverted){
	    fprintf(stderr,"yawn: mouse button up missed\n");
	    shutdown(FAIL);
	}

	GToL(&ep->where);

	if (PtInRect(&ep->where,&tickbox)){
	    invbox = TICKBOX;
	    InvertBox(&tickbox);
	}
	else if (PtInRect(&ep->where,&crossbox)){
	    invbox = CROSSBOX;
	    InvertBox(&crossbox);
	}
}

MUp(ep)
EventRec *ep;
{
	if (!inverted)
	    return;

	GToL(&ep->where);

	if (PtInRect(&ep->where,&tickbox) && (invbox == TICKBOX)){
	    InvertBox(&tickbox);
	    shutdown(YES);
	}
	if (PtInRect(&ep->where,&crossbox) && (invbox == CROSSBOX)){
	    InvertBox(&crossbox);
	    shutdown(NO);
	}
	if (invbox == TICKBOX)
	    InvertBox(&tickbox);
	else if (invbox == CROSSBOX)
	    InvertBox(&crossbox);

	invbox = NOBOX;
}

InvertBox(rect)
rectangle *rect;
{
	inverted = !inverted;
	InvRect(rect);
	Flush();
}

PrintLine()
{
	if (textline[0] == '\0')
	    return;		/* empty line */
	if (filling)
	    justify();
	MoveTo(LETTERX,LETTERY + linecount * LETTERDEPTH);
	fprintf(fs,"%s",textline);
	linecount++;
	count = 0;
	l = textline;
	*l = '\0';
}

justify()
{
	int nspaces;
	int count = 0;

	nspaces = LINELENGTH - strlen(textline);

	if (nspaces == 0)
	    return;

	while (nspaces != 0){
	    /* justify from the left */
	    while ((textline[count] != ' ') && (textline[count] != '\0'))
		count++;

	    if (textline[count] == '\0'){
		/* end of line reached, start again */
		count = 0;
		continue;
	    }

	    /* add a space */
	    insert(count);

	    /* see if we've done */
	    if (--nspaces == 0)
		return;

	    /* skip over spaces to find next word */
	    while (textline[count] == ' ')
		count++;

	    if (textline[count] == '\0')
		/* end of line reached, start again */
		count = 0;
	}
}

insert(num)
register int num;
/* Insert a space in textline at character number num */
{
	register int i;
	register int l;

	l = strlen(textline);
	for (i = l;i >= num;i--)
	    textline[i+1] = textline[i];

	textline[num] = ' ';
}
