
/* This file contains a library of routines that
 * may be used to produce graphics under System V
 * on the Triple X machine.
 */

#include <wlib.h>
#include <stdio.h>
#include "libg.h"
#include <math.h>
#include <termio.h>

#define PI 3.1415926
#define BODGE		/* if defined, then label() does not change */
			/* the text colour to red */

			/* angular adjustment for text */
short x_angle = 0;
short y_angle = 0;

			/* character positioning adjustment */
short ch_adjusted = 0;

			/* line gaps and size */
static short lngap = 0;
static short lnsize = 0;

extern short scale(),adjust();
#ifdef DEBUG
extern FILE *debug;
#endif

linemod (s)
char *s;		/* select the line drawing mode */
{
    static struct {
	char *str;
	int lncolor;
        char lngap,lnsize;
    } lnmodes[] = {
	"solid",	DARKBLUE,      0,    0,
	"dotted",	SPECLEGREY,   16,    8,
	"longdashed",	MUDDYYELLOW,  32,   64,
	"shortdashed",	BEIGE,        16,   16,
	"dotdashed",	HALFGREY,     16,   24,
	"dashed",	PURPLE,       24,   24,
	NULL,		NULL,          0,    0
    };

    int i;

    for (i = 0; lnmodes[i].str ;++i)
	if(strcmp(lnmodes[i].str,s) == 0) {	/* 0 == SAME */
            lngap = lnmodes[i].lngap;
            lnsize = lnmodes[i].lnsize;
#ifndef DEBUG
/* Don't do this
	    SetFPat(lnmodes[i].lncolor);
*/
#endif
            return;
	}
}


setstyle(style)
int style;
{
#ifdef DEBUG
if (debug != 0)
fprintf(debug, "style: %d\n",style);
#endif

	switch (style) {

		case SOLID :	linemod("solid"); break;

		case DOTTED :	linemod("dotted"); break;

		case DOTDASH : linemod("dotdashed"); break;

		case DASHED :  linemod("shortdashed"); break;

		case LONGDASH : linemod("longdashed"); break;
	};
}

setweight(weight) {

#ifdef DEBUG
if (debug != 0)
fprintf(debug, "weight: %d\n",weight);
#endif

	switch (weight) {

		case NARROW :		PenSize(NARROW_W,NARROW_H);
					break;

		case INVISIBLE :	PenSize(0,0);
					break;

		case MEDIUM :		PenSize(MEDIUM_W,MEDIUM_H);
					break;

		case WIDE :		PenSize(BOLD_W,BOLD_H);
					break;
	};
}

setcolor(color) {
#ifdef DEBUG
if (debug != 0)
fprintf(debug, "color: %d\n",color);
#else

	switch (color) {

		case GPSBLACK :		SetFPat(DARKBLUE);
					break;

		case GPSBLUE :		SetFPat(SPECLEGREY);
					break;

		case GPSGREEN :		SetFPat(GREEN);
					break;

		case GPSRED :		SetFPat(CHERRYRED);
					break;
	};
#endif
}

setfont(font) {
#ifdef DEBUG
if (debug != 0)
fprintf(debug, "font: %d\n",font);
#else

	switch (font) {
			/* not really a font but.. */

		case FONTNSO :		Italic(TOGGLE);
					break;

		case FONTNDO :		Bold(TOGGLE);
					break;

		case FONTNDD :		ULine(TOGGLE);
					break;

		case FONTNDA :		Inverse(TOGGLE);
					break;
	}
#endif
}

setsize(textsize) {	/* adjust to this text size */
}

setrot(rotate)
int rotate;
{
						/* radians to turn */
	double angle;

	if (!rotate) {
		x_angle = y_angle = 0;
		return;
	}

	x_angle = (short) (32 * cos(rotate)) - 32;	/* less one ch width */
	y_angle = (short) (-64 * sin(rotate));	/* negative direction */
#ifdef DEBUG
if (debug != 0)
fprintf(debug, "setrot: %d gives angle %f and x_angle %d y_angle %d\n",
			rotate,angle,x_angle,y_angle);
#endif
}

styleline(x0,y0,x1,y1) 	 /* draw a line with gap and length for style */
short x0,y0,x1,y1; {
	double slope,ix,iy,lx,ly,cx,cy;
        int d,xl,xh,yl,yh;

#ifdef DEBUG
if (debug != 0)
fprintf(debug, "styleline: (%d,%d) to (%d,%d)\n",x0,y0,x1,y1);
#endif
	scale(&x0,&y0);
	adjust(&x0,&y0);
	scale(&x1,&y1);
	adjust(&x1,&y1);

#ifdef DEBUG
if (debug != 0)
fprintf(debug, "styleline: adjusted (%d,%d) to (%d,%d)\n",x0,y0,x1,y1);
#endif

	slope = atan2((double)(y1 - y0),(double)(x1 - x0));
#ifdef DEBUG
if (debug != 0)
fprintf(debug, "styleline: slope %f\n",slope);
#endif

	ix = lnsize == 0 ? 0       : cos(slope) * lngap;
	iy = lnsize == 0 ? 0       : sin(slope) * lngap;
	lx = lnsize == 0 ? x1 - x0 : cos(slope) * lnsize;
	ly = lnsize == 0 ? y1 - y0 : sin(slope) * lnsize;
#ifdef DEBUG
if (debug != 0)
fprintf(debug, "styleline: gap %d size %d\n",lngap,lnsize);
if (debug != 0)
fprintf(debug, "ix %f, iy %f, lx %f,ly %f\n",ix,iy,lx,ly);
#else

        xl = x0 < x1 ? x0 : x1;
        xh = x0 < x1 ? x1 : x0;
        yl = y0 < y1 ? y0 : y1;
        yh = y0 < y1 ? y1 : y0;
        cx = x0; cy = y0;
        d = 0;
	DropPen();
        while (xl <= cx && cx <= xh && yl <= cy && cy <= yh)
        {
            if (d)
                DrawTo ((int) cx, (int) cy);
            else
                MoveTo ((int) cx, (int) cy);
            d = !d;
            cx += d ? lx : ix;
            cy += d ? ly : iy;
        }
        DrawTo (x1, y1);
	RaisePen();
#endif
        ch_adjusted = 0;
}

label (p)
char *p;
{		/* Place the following ascii string at current point */
    char *fp = p;
    upatstate lastpats;


#ifdef	DEBUG
if (debug != 0)
fprintf(debug, "label: %s\n",p);
#endif

    DropPen();

			/* count characters to print */
    while (*p) p++;
			/* adjust position of first char point for XXX */
			/* if not already adjusted */
    if(! ch_adjusted) {
#ifndef DEBUG
	MoveBy(X_CH_ADJ,Y_CH_ADJ);
#endif
	ch_adjusted = 1;
    }

#ifndef BODGE
			/* get current forground and background handles */
    GetPatState(&lastpats);

			/* set foreground color to red */
    SetFPat(CHERRYRED);
#endif

			/* if angle adjusted labels, make further adjustment */
    if(x_angle || y_angle) {

			/* Bodge to make text output nicer */
	if (y_angle < 0 && x_angle <= 0) {	/* write from end to start */
#ifndef DEBUG
		MoveBy((p - fp) * (x_angle + 32),(p - fp) * y_angle/2);
#endif
		x_angle = -(x_angle + 32) - 32;
		y_angle = -y_angle;
	}

	ch_adjusted = 0;
    	while(*fp) {
#ifndef DEBUG
	    MoveBy(x_angle,y_angle);
	    putchar(*fp++);
#else
	    ++fp;
#endif
        }
    } else {
				/* normal horizontal text */
#ifndef DEBUG
printf("%s",fp);
#endif
    }

#ifndef BODGE
				/* reset color */
    SetFPat(lastpats.fpathandle);
#endif
    RaisePen();
}

