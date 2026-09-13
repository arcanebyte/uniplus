
/* This file contains a library of routines that
 * may be used to produce graphics under System V
 * on the Triple X machine.
 */

#include <wlib.h>
#include <stdio.h>
#include "libg.h"
#include <math.h>
#include <termio.h>

#define PI 3.142
#define EPSILON 20	/* equality comparison range used by draw_arc() */

extern double scale_x,scale_y;
extern short adjx,adjy;

			/* character positioning adjustment */
extern short ch_adjusted;

extern short scale(),adjust();
double sqrt();

line (x1,y1,x2,y2)
short x1,y1,x2,y2;	/* draw line from x1,y1 to x2,y2 */
{
#ifdef	DEBUG
printf("line: %d,%d to %d,%d\n",x1,y1,x2,y2);
#endif
    move(x1,y1);
    cont(x2,y2);
}

circle (x,y,r)
short x,y,r;		/* draw a plain circle of radius r around point x,y */
{
#ifdef	DEBUG
printf("circle: at %d,%d rad %d\n",x,y,r);
#endif
			/* plain circle */
    DropPen();
    ellipse(x-r,y+r,2*r,2*r);
    RaisePen();
}

ellipse(x,y,w,h)	/* draw an ellipse in rectangle at x,y with width w
			 * and height h.
			 */
short x,y,w,h; {
#ifdef	DEBUG
printf("ellipse: %d,%d w %d h %d\n",x,y,w,h);
#endif
	scale(&x,&y);
	adjust(&x,&y);
        w = (short)((double)(w + adjx) * scale_x);
        h = (short)((double)(h + adjy) * scale_y);
#ifndef DEBUG
	FrameEll(x,y,w,h);
#endif
	ch_adjusted = 0;
}

sarc_draw (x,y,x0,y0,x1,y1)
short x,y,x0,y0,x1,y1;	/* draw an arc */
{
#ifdef	DEBUG
printf("sarc: origin at %d,%d from %d,%d to %d,%d\n",x,y,x0,y0,x1,y1);
#endif
			/* shaded arc */
    DropPen();
    draw_arc(x,y,x0,y0,x1,y1,ON);
    RaisePen();
}

arc_draw (x,y,x0,y0,x1,y1)
short x,y,x0,y0,x1,y1;	/* draw an arc */
{
#ifdef	DEBUG
printf("arc: origin at %d,%d from %d,%d to %d,%d\n",x,y,x0,y0,x1,y1);
#endif
			/* plain arc */
    DropPen();
    draw_arc(x,y,x0,y0,x1,y1,OFF);
    RaisePen();
}

move (x,y)
short x,y;		/* move to position x,y */
{
#ifdef	DEBUG
printf("move: %d,%d\n",x,y);
#endif
    scale(&x,&y);
    adjust(&x,&y);
#ifndef	DEBUG
    MoveTo(x,y);
#endif
    ch_adjusted = 0;
}

cont (x,y)
short x,y;
{			/* draw a line from current position to x,y */
			/* according to the current style chosen */
#ifdef	DEBUG
printf("cont: %d,%d\n",x,y);
#endif
    scale(&x,&y);
    adjust(&x,&y);
#ifndef	DEBUG
    DropPen();
    DrawTo(x,y);
    RaisePen();
#endif
    ch_adjusted = 0;
}

plot_point (x,y)
short x,y;
{			/* plot a point at x,y */
#ifdef	DEBUG
printf("point: %d,%d\n",x,y);
#endif
    scale(&x,&y);
    adjust(&x,&y);
#ifndef	DEBUG
    PlotPoint(x,y);
#endif
}

draw_arc(orig_x,orig_y,posx,posy,lastx,lasty,fillon)
int orig_x,orig_y,posx,posy,lastx,lasty,fillon;
		/* draw an arc, with centre at the origin given 
		and perimeter starting at posx,posy ending at lastx,lasty.
		Shade if fillon */

/* The three points passed to this routine may define either a circle or
   an ellipse. The treatment of the circle case is straightforward. In
   the case of an ellipse, we find the semi-major and minor axes, and
   then calculate the angles to use in the draw-arc escape sequence.

   Let x = posx - orig_x
       y = posy - orig_y
       X = lastx - orig_x
       Y = lasty - orig_y

   Then a^2y^2 + b^2x^2 = a^2b^2
        a^2Y^2 + b^2X^2 = a^2b^2

        a^2(y^2 - b^2) + b^2x^2 = 0
        a^2(Y^2 - b^2) + b^2X^2 = 0

   Pre-multiply and subtract :

        b^2x^2(Y^2 - b^2) - b^2X^2(y^2 - b^2) = 0

   i.e. b^2(x^2(Y^2 - b^2) - X^2(y^2 - b^2)) = 0

   Since b^2 != 0,

        x^2(Y^2 - b^2) = X^2(y^2 - b^2)

        x^2Y^2 - X^2y^2 = b^2x^2 - b^2X^2

        x^2Y^2 - X^2y^2 = b^2(x^2 - X^2)

   So b^2 = x^2Y^2 - X^2y^2  = (xY - Xy)(xY + Xy)
            ---------------    ------------------
              (x^2 - X^2)          (x^2 - X^2)

   a is found by substituting b into one of the original equations.

   Then, since in the parameterised form of the ellipse equations,

	y = b * sin(theta)                   a * y
 	                     => tan(theta) = -----
	x = a * cos(theta)                   b * x
*/

{
    short r,R;			/* scaled radii of two points */
    double x,y,X,Y;		/* scaled x and y differences */
    short tx,ty,w,h;		/* bounding box origin and dimensions */
    short a1,a2;		/* arc start and finish angles */
    int a,b;			/* ellipse semi-major and minor axes */
    short ox,oy;		/* scaled origin coordinates */
    short px,py,lx,ly;		/* scaled point coordinates */
    rectangle frect;

    /* convert everything to Triple X coordinates */

    ox = orig_x; oy = orig_y;
    px = posx; py = posy;
    lx = lastx; ly = lasty;
    scale(&ox,&oy); adjust(&ox,&oy);
    scale(&px,&py); adjust(&px,&py);
    scale(&lx,&ly); adjust(&lx,&ly);

    /* get the displacements of the points from the origin
       and the two radii */

    x = (double) (px - ox);
    y = (double) (py - oy);
    X = (double) (lx - ox);
    Y = (double) (ly - oy);
    r = (short) hypot(x,y);
    R = (short) hypot(X,Y);

#ifdef DEBUG
    printf("r = %d, R = %d\n",r,R);
    printf("x = %g y = %g X = %g Y = %g\n",x,y,X,Y);
#endif

    if (approx_equal((double) r,(double) R)){

/* the points lie on a circle */

#ifdef DEBUG
    printf("Arc circle centre %d,%d radius %d\n",ox,oy,r);
#endif DEBUG

    /* find the bounding box */

    tx = ox - r;
    ty = oy - r;
    w = h = r + r;

    /* find the start and end angles */

    a1 = (short) (atan2(y,x) * 180.0 / PI);
    a2 = (short) (atan2(Y,X) * 180.0 / PI);
    }
    else {				/* it's an ellipse */
    double b2;
    double xY = x * Y;			/* These stange variables are  */
    double Xy = X * y;			/* used to cut down the number */
    double x2 = x * x;			/* of multiplications needed   */

    /* Test to see if the ellipse has its major and minor axes  */
    /* aligned correctly and exit if not, with a message. The   */
    /* axes must be parallel to those of the screen coordinates */
    /* as the kernel cannot draw rotated ellipses.              */

    if (approx_equal(x,X) ^ approx_equal(y,Y)){
	closepl();
        fprintf(stderr,"Bad ellipse in draw_arc, cannot draw\n");
        exit(0);
    }

    /* find the semi-major and minor axes */

    if (approx_equal(x,0)){
        b = (int) fabs(y);
        b2 = b * b;
    }
    else if (approx_equal(X,0)){
        b = (int) fabs(Y);
        b2 = b * b;
    }
    else{
        b2 = (xY - Xy) * (xY + Xy) / (x2 - X * X);
        b = (int) sqrt(b2);
    }

    if (approx_equal(y,0))
        a = (int) fabs(x);
    else if (approx_equal(Y,0))
        a = (int) fabs(X);
    else if (!approx_equal(x,0))
        a = (int) sqrt(b2 * x2 / (b2 - y * y));
    else
	a = (int) sqrt(b2 * X * X / (b2 - Y * Y));

#ifdef DEBUG
    printf("Arc ellipse centre = %d,%d\n",ox,oy);
    printf("a = %d b = %d\n",a,b);
#endif

    /* find the bounding box */

    tx = (short) ox - a;
    ty = (short) oy - b;
    w = (short) a + a;
    h = (short) b + b;

    /* find the start and end angles */

    a1 = (short) (atan2(a * y,b * x) * 180.0 / PI);
    a2 = (short) (atan2(a * Y,b * X) * 180.0 / PI);
    }

#ifdef DEBUG
    printf("tx = %d ty = %d w = %d h = %d a1 = %d a2 = %d\n",
           tx,ty,w,h,a1,a2);
#else

    /* Now we know where the arc has to be, draw it */

    frect.x = tx; frect.y = ty;
    frect.width = w; frect.height = h;

	/* a1 and a2 are used the wrong way rounnd because the
	   Triple X coordinate system is upside down relative to
	   gps so that the sense of rotation is reversed */
    if (fillon == OFF)
	FrameArc(a2,a1,&frect);
    else
	FillArc(a2,a1,&frect);
#endif DEBUG
    ch_adjusted = 0;
}

