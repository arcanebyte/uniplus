
/* This file contains a library of routines that
 * may be used to produce graphics under System V
 * on the Triple X machine.
 */

#include <wlib.h>
#include <stdio.h>
#include "libg.h"
#include <math.h>
#include <termio.h>

#define EPSILON 20	/* equality comparison range used by draw_arc() */
#define TRUE 1
#define FALSE 0

			/* default scaling and adjustment for virtual screen */
extern double scale_x;
extern double scale_y; 
extern short adjx;
extern short adjy;


			/* angular adjustment for text */
extern short x_angle;
extern short y_angle;

			/* character positioning adjustment */
extern short ch_adjusted;

static struct termio carg ;

			/* current colors */
static int bpat_handle;
static int fpat_handle;

static upatstate pats;

openpl ()
{		/* open plotting device for writing */
#ifdef	DEBUG
printf("openpl\n");
#else
    unsigned short tmpo_flag;

    ioctl(1,TCGETA, &carg);
    tmpo_flag = carg.c_oflag;
    carg.c_oflag =0;	/* set raw mode */
    ioctl(1,TCSETAW,&carg);
    carg.c_oflag = tmpo_flag;

			/* get current forground and background handles */
    GetPatState(&pats);

			/* set default weight to MEDIUM */
    setweight(MEDIUM);

			/* set default color to GPSBLACK */
    setcolor(GPSBLACK);
#endif
}

erase ()
{		/* clear graphics area to current background color */
#ifdef	DEBUG
printf("erase\n");
#else
   ClearWindow();
#endif
}

space (lwlx,lwly,hrx,hry)
int lwlx,lwly,hrx,hry;		/* set device plotting area */
{
    scale_x = (double)((double)RANGE_X/(double)(hrx - lwlx));
    scale_y = (double)((double)RANGE_Y/(double)(hry - lwly));
    adjx = lwlx;
    adjy = lwly ;
#ifdef	DEBUG
printf("lwlx = %d, lwly = %d, hrx = %d, hry = %d\n", lwlx , lwly , hrx , hry); 
printf("scale_x = %f, scale_y = %f,adjx = %d, adjy = %d\n",
			scale_x,scale_y,adjx,adjy);
#endif	DEBUG
}

closepl ()
{			/* close plotting device stream */
#ifdef	DEBUG
printf("closepl\n");
#else
			/* reset pensize to medium */
    setweight(MEDIUM);


			/* reset foreground and background */
    SetFPat(pats.fpathandle);
    SetBPat(pats.bkpathandle);

			/* move cursor to bottom of screen */
    HomeCursor();

    ioctl(1,TCSETAW,&carg);
#endif
}

int approx_equal(a,b)
    double a,b;
{
    if (fabs(a - b) <= EPSILON)
        return(TRUE);
    else
        return(FALSE);
}

