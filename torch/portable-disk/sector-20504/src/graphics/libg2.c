
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

linemod (s)
char *s;		/* select the line drawing mode */
{
    static struct {
	char *str;
	int lncolor;
    } lnmodes[] = {
	"solid",	DARKBLUE,
	"dotted",	SPECLEGREY,	
	"longdashed",	MUDDYYELLOW,
	"shortdashed",	BEIGE,
	"dotdashed",	HALFGREY,
	"dashed",	PURPLE,
	NULL,		NULL
    };

    int i;

    for (i = 0; lnmodes[i].str ;++i)
	if(strcmp(lnmodes[i].str,s) == 0) {	/* 0 == SAME */
#ifndef DEBUG
	SetFPat(lnmodes[i].lncolor);
#endif
		return;
	}
}

/*
linemod (s)
char *s;	
{
    static struct {
	char *str,lngap,lnsize;
    } lnmodes[] = {
	"solid",	0,	0,
	"dotted",	16,	8,
	"longdashed",	32,	64,	
	"shortdashed",	16,	16,
	"dotdashed",	16,	24,
	"dashed",	24,	24,
	NULL,		NULL
    };

    int i;

    for (i = 0; lnmodes[i].str ;++i)
	if(strcmp(lnmodes[i].str,s) == 0) {	
		lngap = (lnmodes[i].lngap);
		lnsize = (lnmodes[i].lnsize);
		return;
	}
}
*/

setstyle(style)
int style;
{
#ifdef DEBUG
printf("style: %d\n",style);
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
printf("weight: %d\n",weight);
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
printf("color: %d\n",color);
#else

	switch (color) {

		case GPSBLACK :		SetFPat(DARKBLUE);
					break;

		case GPSBLUE :		SetFPat(GREY);
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
printf("font: %d\n",font);
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

/*	angle  = ((double) rotate / 256) * (double)(2 * PI); */

	x_angle = (short) (32 * cos(rotate)) - 32;	/* less one ch width */
	y_angle = (short) (-64 * sin(rotate));	/* negative direction */
#ifdef DEBUG
printf("setrot: %d gives angle %f and x_angle %d y_angle %d\n",
			rotate,angle,x_angle,y_angle);
#endif
}

styleline(x0,y0,x1,y1) 	 /* draw a line with gap and length for style */
short x0,y0,x1,y1; {
	double slope,ix,iy,lx,ly;
	int nx,ny;	/* number of */

#ifdef DEBUG
printf("styleline: (%d,%d) to (%d,%d)\n",x0,y0,x1,y1);
#endif
	scale(&x0,&y0);
	adjust(&x0,&y0);
	scale(&x1,&y1);
	adjust(&x1,&y1);

#ifdef DEBUG
printf("styleline: adjusted (%d,%d) to (%d,%d)\n",x0,y0,x1,y1);
#endif

	if (y1 - y0 < 0)	/* decreasing y */
		ly = iy = -1;
	else
		ly = iy = 1;

	if (x1 - x0 < 0)	/* decreasing x */
		lx = ix = -1;
	else
		lx = ix = 1;

	if (x1 - x0)	/* not zero */
		slope = atan((double)((y1 - y0)/(x1 - x0)));
	else
		slope = PI/2;	/* vertical */
#ifdef DEBUG
printf("styleline: slope %f\n",slope);
#endif
	ix *= cos(slope) * lngap;
	iy *= sin(slope) * lngap;
	lx *= cos(slope) * lnsize;
	ly *= sin(slope) * lnsize;

				/* number of points */
	if (ix + lx)
		nx = ((double)(x1 - x0)/(double)(ix + lx));
	else nx = 1;

	if (iy + ly)
		ny = ((double)(y1 - y0)/(double)(iy + ly));
	else ny = 1;

#ifdef DEBUG
printf("styleline: gap %d size %d\n",lngap,lnsize);
printf("ix %f, iy %f, lx %f,ly %f, nx %d, ny %d\n",ix,iy,lx,ly,nx,ny);
#else
	DropPen();
	while (nx-- > 0 || ny-- > 0) {
				/* dash , dot or long dash */
		x0 += lx;
		y0 += ly;
		DrawTo(x0,y0);

				/* gap */
		x0 += ix;
		y0 += iy;
		MoveTo(x0,y0);
	}
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
	printf("label: %s\n",p);
#endif

			/* count characters to print */
    while (*p) p++;
			/* adjust position of first char point for XXX */
			/* if not already adjusted */
    if(! ch_adjusted) {
#ifndef DEBUG
	DrawBy(X_CH_ADJ,Y_CH_ADJ);
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
}

