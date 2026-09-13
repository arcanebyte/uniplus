			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 0.11\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *
 *   0.11		Make program immune to bug in hist and pie
 *			output whereby numerical strings which do not
 *			end on a word boundary are not padded with '\0'.
 *			NP 27/9/85
 *
 */

#include "gpl.h"

#ifdef	DEBUG
#include <stdio.h>
extern FILE *debug;
#endif	DEBUG

extern struct command *actptr;
extern int g_cmd;
int *getpoint();

excomm() {
	switch (g_cmd) {
	case LINES:
		ldraw();
		break;
	case ARCS:
		arcdraw();
		break;
	case COMMENT:
		break;

				/* print some text */
	case TEXT:
		smove(getpoint());
	case ALPHA:
		tprint();
		break;
	}
}

ldraw() {
	register short i;
	i = npoints();
	setcolor(actptr->color);
	setweight(actptr->weight);
	setstyle(actptr->style);
	smove(getpoint());
	while (--i) {
		sline(getpoint());
	}
}

arcdraw() {
	int res[2];
	register int *p1, *p2, *p3;
	register short i;
	setcolor(actptr->color);
	setweight(actptr->weight);
	setstyle(actptr->style);
	i = npoints();
#ifdef	DEBUG
fprintf(debug, "arcdraw: points = %d\n", i);
#endif	DEBUG
	switch (i) {
	case 0:
		break;
	case 1:
		smove(getpoint());
		break;
	case 2:
		smove(getpoint());
		sline(getpoint());
		break;
	case 3:
		/*
		 * use solve() to try and find an origin for the circle. Work
		 * out the radius and the direction of the arc (i.e. clockwise
		 * or anticlockwise). If the direction is anticlockwise then
		 * draw from the last point to the first.
		 * If no origin can be found then draw a line from the first
		 * point to the second and on to the third.
		 */
		p1 = getpoint();
		p2 = getpoint();
		p3 = getpoint();
		if (solve(res, p1, p2, p3)) {
			if (cross(res, p1, p2) >= 0)
			    sarc(res, p1, p3);
			else
			    sarc(res, p3, p1);
		} else {
			smove(p1);
			sline(p2);
			sline(p3);
		}
		break;
	default:
		smove(getpoint());
		/*
		 * predecrement i to make up for the first use of getpoint
		 * being outside the loop.
		 */
		while (--i) {
			sline(getpoint());
		}
		break;
	}
}

tprint() {
#ifdef	DEBUG
fprintf(debug, "tprint: <%s>\n", actptr->text);
#endif	DEBUG
	setcolor(actptr->color);
	setfont(actptr->font);
	setrot(actptr->trot);
	setsize(actptr->tsize);
	label(actptr->text);
}


static int shere[2];


smove(pt)
register int *pt; {
#ifdef	DEBUG
fprintf(debug, "smove: x = %d, y = %d\n", pt[0], pt[1]);
#endif	DEBUG
	move(pt[0], pt[1]);
	shere[0] = pt[0]; shere[1] = pt[1];
}

sline(pt)
int *pt; {
#ifdef	DEBUG
fprintf(debug, "sline: x = %d, y = %d\n", pt[0], pt[1]);
#endif	DEBUG
	styleline (shere[0], shere[1], pt[0], pt[1]);
	shere[0] = pt[0]; shere[1] = pt[1];
}

sarc(orig, stpt, ept)
int *orig, *stpt, *ept; {
#ifdef	DEBUG
fprintf(debug, "sarc: ox = %d, oy = %d, x1 = %d, y1 = %d, x2 = %d, y2 = %d\n",orig[0], orig[1], stpt[0], stpt[1], ept[0], ept[1]);
#endif	DEBUG
	arc_draw(orig[0], orig[1], stpt[0], stpt[1], ept[0], ept[1]);
}


