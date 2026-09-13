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

#include <ctype.h>
#include "gpl.h"
#include "gsl.h"
#include <stdio.h>
#include <setjmp.h>
#include "drawdefs.h"

#ifdef	DEBUG
FILE *debug;
#endif	DEBUG

struct command action;
struct command *actptr;
jmp_buf eofjmp;		/* longjmp this when eof is reached */
int g_cmd;
int g_cnt;
int *ptptr;	/* next point to be used */

tidy_up(sig) {
	closepl();
	exit(sig);
}

main(argc, argv)
int argc;
char *argv[]; {
	/*
	 * If there is an argument, try to open it as the file to be read.
	 * Otherwise use the standard input.
	 */
	int region;		/* expanded region drawn */
	int lx = XYMIN,ly = XYMIN;

#ifdef DEBUG
	register i;
	debug = fopen("debug", "w");
#endif DEBUG

			/* first deal with any flags */
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'e') {
		region =  atoi(&argv[1][2]);
		argc--;
		argv++;

		if (region)
			region -= 1;	/* start regions from zero */
		else
			region = DEF_REGION -1;	/* default region to expand */

				/* calculate adjustment */
		lx = (XDIM/COLUMNS * (region%5)) + XYMIN;
		ly = (YDIM/ROWS * (region/5)) + XYMIN;

				/* scale as appropriate */
		space(lx,ly,lx + (XDIM/COLUMNS),ly + (YDIM/ROWS));
	}
	else
	 				/* default scaling and position */
		space(lx,ly,lx + XDIM,ly + YDIM);

	if (argc > 1) {
		if ( freopen(argv[1], "r", stdin) == 0)
			exit(1);
	}

	signal(1, tidy_up);
	signal(2, tidy_up);
	signal(3, tidy_up);
	openpl();

	if (!setjmp(eofjmp)) {
		for (;;) {
#ifdef	DEBUG
for (i=200000; --i;);
#endif	DEBUG
			cmdinit();
			rdcomm();
			excomm();
		}
	}
	tidy_up(0);
}

cmdinit() {
	ptptr = action.aptr = action.array;
	action.tptr = action.text;
	actptr = &action;
}

int *getpoint() {		/* return a pointer to the next pair of x,y */
				/* and move on the pointer */
	int *tmpptr = ptptr;

	ptptr = &ptptr[2];	/* start of next pair of x,y */

	return(tmpptr);
}

npoints() {
	return((actptr->aptr - actptr->array) / 2);
}



rdcomm() {
/*
 * read a GPS command.
 */
	short i;

	cmdin();
#ifdef	DEBUG
fprintf(debug, "command = 0x%x, count = %d\n", g_cmd, g_cnt);
#endif	DEBUG
	switch (g_cmd) {
	case LINES:
#ifdef DEBUG
fprintf(debug,"command = LINES\n");
#endif
	case ARCS:
		for (i=g_cnt - 2; i > 0; i -= 2)
			pntin();
		swin();
#ifdef DEBUG
if (g_cmd == ARCS) fprintf(debug,"command = ARCS\n");
#endif
		break;
	case COMMENT:
		strin(g_cnt - 1);
#ifdef DEBUG
fprintf(debug,"command = COMMENT\n");
#endif
		break;
	case TEXT:
		pntin();	/* uses 2 words */
		swin();
		soin();
		strin(g_cnt - 5);
#ifdef DEBUG
fprintf(debug,"command = TEXT\n");
#endif
		break;
	case ALPHA:
		pntin();
		strin(g_cnt - 3);
#ifdef DEBUG
fprintf(debug,"command = ALPHA\n");
#endif
		break;
	default:
		/*
		 * For an unrecognised command, read in the bytes but discard
		 * them.
		 */
		strin(g_cnt - 1);
#ifdef DEBUG
fprintf(debug,"command = UNRECOGNISED\n");
#endif
		break;
	}
}


short rdword() {
/*
 * A GPS file is organised as 2 byte words, the low word coming first which is
 * the opposite way from that expected by a 68000. This routine reads a word
 * and reverses the order of the bytes.
 */
	union {
		short res;
		struct {
			char byte2;
			char byte1;
		} inbyte;
	} conv;

	conv.inbyte.byte1 = inchar();
	conv.inbyte.byte2 = inchar();
	return(conv.res);
}

inchar() {
	int c;
	if ((c = getchar()) == EOF)
		longjmp(eofjmp);
	return(c);
}


cmdin() {
	union cmdword c;

	c.cma.pntw = rdword();
	actptr->cmd = g_cmd = c.cmc.cm;
	actptr->cnt = g_cnt = c.cmc.cn;
}

pntin() {
	union cmdword p1;
	union cmdword p2;

	p1.cma.pntw = rdword();
	p2.cma.pntw = rdword();
	if (actptr->aptr < &actptr->array[MAXLINES * 2 - 1]) {
		*(actptr->aptr)++ = p1.cma.pntw;
		*(actptr->aptr)++ = p2.cma.pntw;
	}
}


strin(n)
register n; {
	union cmdword s;

	while (n--) {
		s.cma.pntw = rdword();
		if (actptr->tptr < &actptr->text[TEXLEN - 1]) {
			*(actptr->tptr)++ = s.cmb.cmbyte1;
		}
		if (actptr->tptr < &actptr->text[TEXLEN - 1]) {
			*(actptr->tptr)++ = s.cmb.cmbyte2;
		}
	}
	*(actptr->tptr) = 0;
}

swin() {
	union styleword s;
	register struct command *aptr = actptr;

	s.sta.in = rdword();
	aptr->color = s.stb.col;
	aptr->weight = s.stb.wt;
	aptr->style = s.stb.st;
	aptr->font = s.stc.fon;
}

soin() {
	union szorient s;

	s.sza.in = rdword();
	actptr->trot = s.szb.tr;
	actptr->tsize = s.szb.ts;
}


