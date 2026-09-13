			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 1.00\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *
 *   0.11		Fix angle determination in draw_arc().
 *			Fix bad ellipse determination in draw_arc().
 *			Both fixes in libxxx4.c
 *			NP 26/9/85
 * RELEASED as 1.00
 *
 */

/* This file contains a library of routines that
 * may be used to produce graphics under System V
 * on the Triple X machine.
 */

#include <wlib.h>
#include <stdio.h>
#include "libg.h"
#include <math.h>
#include <termio.h>

			/* default scaling and adjustment for virtual screen */
double scale_x = 1;
double scale_y = 1; 
short adjx = 0;
short adjy = 0;

scale(x,y)		/* apply scaling to x and y values for virtual */
short *x,*y;{
    *x = (short)((double)(*x - adjx) * scale_x) & ~ 4;
    *y = (short)((double)(*y - adjy) * scale_y) & ~ 8;
}


adjust(x,y)		/* adjust the x and y values for triple x */
short *x,*y;{
    *y = (LOWLEFTY - *y);
}

