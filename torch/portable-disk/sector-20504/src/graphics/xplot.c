			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 0.10\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *
 */

			/* ttorch is a program that interprets graphic
			commands from the standard input and writes to the
			standard output to produce the appropriate shapes
			for the commands given.	*/
#include <stdio.h>
#include <signal.h>
#include <wlib.h>
			/* plotting instructions code */
#define MOVE 'm'
#define CONT 'n'
#define POINT 'p'
#define DRAWLINE 'l'
#define LABEL 't'
#define ERASE 'e'
#define LINEMOD 'f'
#define SPACE 's'
#define CIRCLE 'c'
#define ARC 'a'
#define NEWLINE '\n'

wrapup() {
    closepl();
    exit(1);
}


main(argc, argv)
int argc;
char *argv[];
{
    short x,y,x0,y0,x1,y1;	/* often used x,y variables */
    char func;		/* function select variable */
    char labstr[80];	/* label buffer */

    openpl();
    signal(SIGINT, wrapup);
    signal(SIGQUIT, wrapup);
    signal(SIGTERM, wrapup);
    while( read(0,&func,1) > 0)
	switch(func)	{
	    case MOVE :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		move(x,y);
		break;

	    case CONT :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		cont(x,y);
		break;

	    case POINT :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		plot_point(x,y);
		break;

	    case DRAWLINE :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		read(0,&x1,2);
		read(0,&y1,2);
		swap(&x1);
		swap(&y1);
		line(x,y,x1,y1);
		break;

	    case LABEL :
		{
		    char *p = labstr;

		    do
		    {
			read(0,p,1);
			if(*p == NEWLINE)
			    *p = NULL;
		    } while (*p++);

		    label(labstr);
		    break;
		}

	    case ERASE :
		erase();
		break;

	    case LINEMOD :
		{
		    char *p = labstr;

		    do
		    {
			read(0,p,1);
			if(*p == NEWLINE)
			    *p = NULL;
		    } while (*p++);

		    linemod(labstr);
		    break;
		}

	    case SPACE :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		read(0,&x1,2);
		read(0,&y1,2);
		swap(&x1);
		swap(&y1);
		space(x,y,x1,y1);
		break;

	    case CIRCLE :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		read(0,&x1,2);	/* radius */
		swap(&x1);
		circle(x,y,x1);
		break;

	    case ARC :
		read(0,&x,2);
		read(0,&y,2);
		swap(&x);
		swap(&y);
		read(0,&x0,2);
		read(0,&y0,2);
		swap(&x0);
		swap(&y0);
		read(0,&x1,2);
		read(0,&y1,2);
		swap(&x1);
		swap(&y1);
		arc_draw(x,y,x0,y0,x1,y1);
		break;

	}
    /* end of while */
    closepl();
    exit(0);
}

swap(x) 
short *x; {
	char tmp;
/*
 * swap the bytes of a short
 */
	tmp = *(char *)x;
	*(char *)x = *((char *)x + 1);
	*((char *)x + 1) = tmp;
}
