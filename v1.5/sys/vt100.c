/*
 * Copyright 1982 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
 * VT100 emulator
 * Called with each character destined for the console.
 * Processes control sequences and keeps track of terminal state.
 * Calls into bitmap.c actually do the I/O
 */

/* SENSESCRN resets the contrast on screen output (via the console device
 * driver) as if a key was hit.
 */

#include "sys/types.h"
#include "sys/l2.h"

#define FAST
#define SENSESCRN

#define	MAXVT_N		255	/* maximum value of any numeric parameter */
#define	MAXPARAMS	10	/* maximum number of numeric parameters */

short vt_n[MAXPARAMS];		/* numeric parameters of \E[ commands */
char vt_mparam;
char vt_tabset[88] = { 0 };
extern char bmbck, bmcolor, bmnormal;
extern char *bmscrn;
extern char kb_altkp;
short vt_maxrow = 38;
short vt_maxcol = 88;
short vt_row, vt_col;		/* cursor location (0-vt_maxrow, 0-vt_maxcol) */
short vtrow_ofs = 1;
short vtcol_ofs = 1;		/* row and column offsets */
short vt_winscrl = 1;		/* lines to scroll each time */

/* This routine interprets the characters destined for the console and
 * performs like a VT100 .  It is implemented in terms of primitives
 * defined in bitmap.c
 */

#define CBKSP	1
#define CCR	2
#define CLF	3
#define CHTAB	4
#define CESC	5
#define CBELL	6
#define CGARB	7
#define CCHAR	8

char vt_keytype[] = {
	CGARB, CGARB, CGARB, CGARB,	CGARB, CGARB, CGARB, CBELL,
	CBKSP, CHTAB, CLF,   CLF,	CLF,   CCR,   CGARB, CGARB,
	CGARB, CGARB, CGARB, CGARB,	CGARB, CGARB, CGARB, CGARB,
	CGARB, CGARB, CGARB,  CESC,	CGARB, CGARB, CGARB, CGARB,

	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,

	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,

	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CCHAR,
	CCHAR, CCHAR, CCHAR, CCHAR,	CCHAR, CCHAR, CCHAR, CGARB,
};

vt_putc (c)
register char c;
{
	extern int (*te_putc)();
	int vt_esc();
	int x;

	extern time_t lbolt;

	l2_dtrap = lbolt + l2_dtime;
	if (l2_dimmed) l2undim();

	if (c >= ' ') {
		bmputc(vt_row+vtrow_ofs,vt_col+vtcol_ofs,c);
		vt_advance();
		bminvert (vt_row+vtrow_ofs, vt_col+vtcol_ofs);
		return;
	}
	bminvert (vt_row+vtrow_ofs, vt_col+vtcol_ofs);
	switch (vt_keytype[c]) {
		case CBKSP:	
			if (vt_col > 0) 
				vt_col--; 
			break;
		case CCR:	
			vt_col = 0;
			 break;
		case CLF:	
			if (++vt_row >= vt_maxrow) {
				blt(bmscrn+90*9,bmscrn+90*9*2,9*90*38);
					vt_row -= vt_winscrl;
			}
			break;
		case CHTAB:
			for ( x = vt_col+1; x < vt_maxcol-1 ; x++ )
				if ( vt_tabset[x] == 1 )
					break;
			vt_col = x;
			break;
		case CESC:	
			te_putc = vt_esc;
			break;
		case CBELL:	
			beep();
			break;
		case CGARB:					
			break;
	}
	bminvert(vt_row+vtrow_ofs,vt_col+vtcol_ofs);
}

/* 
 * Process the escape sequences to the terminal
 */
vt_esc(c)				/* after ESC key hit */
register char c;
{
	extern int (*te_putc)();
	int vt_brck(), vt_putc();

	switch (c) {
		case '[':
			te_putc = vt_brck;	/* check E[ sequence */
			return;
		case '>':		/* disable alternate keypad */
			kb_altkp = 0;
			break;	
		case '=':		/* enable alternate keypad */
			kb_altkp = 1;
			break;	
		case 'M':		/* reverse scroll */
			
			bminvert(vt_row+vtrow_ofs,vt_col+vtcol_ofs);
			bmrscrl();
			bminvert(vt_row+vtrow_ofs,vt_col+vtcol_ofs);

			break;
		case 'H':
			vt_tabset[vt_col]=1;
			break;
	}
	te_putc = vt_putc;
	return;
}

vt_brck(c)					/* \E[ sequence checked here */
register char c;
{
	extern int (*te_putc)();
	int vt_param(), vt_attrb();

	vt_mparam = 0;
	vt_n[0] = vt_n[1] = 0;
	if (c == ';') {		/* missing 1st number - look for 2nd */
		te_putc = vt_param;
		vt_mparam++;
		return;
	}
	if (c >= '0' && c <= '9' ) {
		vt_n[0] = c - '0';
		te_putc = vt_param;
		return;
	}
	if (c == '?') {
		te_putc = vt_attrb;
		return;
	}
	vt_cmd(c);
}

vt_param(c)
register char c;
{
	register tmp;
	int vt_putc();

	if( c >= '0' && c <= '9' ) {
		tmp = (vt_n[vt_mparam] * 10) + (c - '0');
		vt_n[vt_mparam] = (tmp > MAXVT_N) ? MAXVT_N : tmp;
		return;
	}
	if ( c == ';' ) {
		if (++vt_mparam >= MAXPARAMS) {	/* too many parameters */
			te_putc = vt_putc;
			return;
		}
		vt_n[vt_mparam] = 0;
		return;
	}
	vt_cmd(c);
}

vt_cmd(c)		/* now have last char of esc sequence */
register char c;
{
	extern int (*te_putc)();
	register vt_n1 = vt_n[0];
	register vt_n2 = vt_n[1];
	register int x, y;
	int vt_putc();

	if (c == 'f')	c = 'H';
	if ((c >= 'A') && (c <= 'Z')) {
		bminvert (vt_row+vtrow_ofs, vt_col+vtcol_ofs);
		switch (c) {
		case 'A':	/* move cursor up */
			if (vt_n1 == 0)
				vt_n1 = 1;
			vt_row = (vt_n1 < vt_row) ? (vt_row - vt_n1) : 0;
			break;
		case 'B':	/* move cursor down */
			if (vt_n1 == 0)
				vt_n1 = 1;
			y = vt_row + vt_n1;
			vt_row = (y < vt_maxrow) ? y : vt_maxrow-1;
			break;
		case 'C':	/* move cursor right */
			if (vt_n1 == 0)
				vt_n1 = 1;
			y = vt_col + vt_n1;
			vt_col = (y < vt_maxcol) ? y : vt_maxcol-1;
			break;
		case 'D':	/* move cursor left */
			if (vt_n1 == 0)
				vt_n1 = 1;
			vt_col = (vt_n1 < vt_col) ? (vt_col - vt_n1) : 0;
			break;
		case 'H':	/* move cursor home */
			if (vt_n1 == 0)
				vt_row = 0;
			else if ((vt_row = vt_n1-1) >= vt_maxrow)
				vt_row = vt_maxrow - 1;
			if (vt_n2 == 0)
				vt_col = 0;
			else if ((vt_col = vt_n2-1) >= vt_maxcol)
				vt_col = vt_maxcol - 1;
			break;
		case 'J':	/* clear screen */
			if (vt_n1 == 0)
				if ((vt_row == 0) && (vt_col == 0))
					vt_n1 = 2;
			switch (vt_n1) {
			case 0:	/* clear from cursor to end */
				for (y = vt_col ; y < vt_maxcol ; y++ )
					bmputc(vt_row+vtrow_ofs,y+vtcol_ofs,' ');
				for (x = vt_row+1; x < vt_maxrow ; x++ )
					bmblank(x+vtrow_ofs);
				break;
			case 1:	/* clear from beginning to cursor */
				for (x = 0; x < vt_row ; x++ )
					bmblank(x+vtrow_ofs);
				for (y = 0; y <= vt_col ; y++ )
					bmputc(vt_row+vtrow_ofs,y+vtcol_ofs,' ');
				break;
			case 2:	/* clear entire screen */
				bmclear();
			}
			break;
		case 'K':	/* clear line */
			switch (vt_n1) {
			case 0:	/* clear from cursor to end */
				for (y = vt_col ; y < vt_maxcol ; y++ )
					bmputc(vt_row+vtrow_ofs,y+vtcol_ofs,' ');
				break;
			case 1:	/* clear from beginning to cursor */
				for (y = 0 ; y <= vt_col ; y++ )
					bmputc(vt_row+vtrow_ofs,y+vtcol_ofs,' ');
				break;
			case 2:	/* clear entire line */
				bmblank(vt_row+vtrow_ofs);
			}
			break;
		case 'L':	/* insert line(s) */
			if (vt_n1 == 0)
				vt_n1 = 1;
			if (vt_n1 > vt_maxrow - vt_row)
				vt_n1 = vt_maxrow - vt_row;
			for (x=vt_maxrow-1, y=vt_maxrow-vt_n1-1; y >= vt_row; x--, y--)
				bmcpl(x+vtrow_ofs, y+vtrow_ofs);
			for ( ; x >= vt_row ; x-- )
				bmblank(x+vtrow_ofs);
			break;
		case 'M':	/* delete line(s) */
			if (vt_n1 == 0)
				vt_n1 = 1;
			if (vt_n1 > vt_maxrow - vt_row)
				vt_n1 = vt_maxrow - vt_row;
			for (x=vt_row, y=vt_row+vt_n1; y < vt_maxrow; x++, y++)
				bmcpl(x+vtrow_ofs, y+vtrow_ofs);
			for ( ; x < vt_maxrow ; x++ )
				bmblank(x+vtrow_ofs);
			break;
		case 'P':	/* delete character(s) */
			if (vt_n1 == 0)
				vt_n1 = 1;
			if (vt_n1 > vt_maxcol - vt_col)
				vt_n1 = vt_maxcol - vt_col;
			for (x=vt_col, y=vt_col+vt_n1; y < vt_maxcol; x++, y++)
				bmmvc(vt_row+vtrow_ofs, x+vtcol_ofs,
					vt_row+vtrow_ofs, y+vtcol_ofs);
			for ( ; x < vt_maxcol ; x++ )
				bmputc(vt_row+vtrow_ofs,x+vtcol_ofs,' ');
			break;
		}
		bminvert (vt_row+vtrow_ofs, vt_col+vtcol_ofs);
	} else {
		switch (c) {
		case 'g':
			if (vt_n1 == 0)
				vt_tabset[vt_col] = 0;
			else if (vt_n1 == 3)
				for (x = 0 ; x < vt_maxcol ; x++ )
					vt_tabset[x] = 0;
			break;
		case 'm':	/* set normal display or reverse video */
			for (x = 0; x <= vt_mparam; x++)
				switch (vt_n[x]) {
				case 0:	/* turn underline off, set normal background */
					if (bmbck != bmnormal) {
						bmswitch();	/* invert font table */
						bmbck = bmnormal;
					}
					bmcolor = bmbck;	/* underline off */
					break;
				case 1: case 7:	/* set reverse image */
					if (bmbck == bmnormal) {
						bmswitch();	/* invert font table */
						bmbck = bmnormal ? 0 : -1; 
					}
					break;
				case 4:	/* set underline */
					bmcolor = bmbck ? 0 : -1; 
				}
			break;
		}
	}
	te_putc = vt_putc;
}

vt_attrb(c)		/* \E[? */
char c;
{
	int vt_atrb(), vt_putc();

	if(c >= '0' && c <= '9')
		te_putc = vt_atrb;
	else
		te_putc = vt_putc;
}

vt_atrb()
{
	int vt_putc();

	te_putc = vt_putc;
}

vt_advance()
{
	if (++vt_col >= vt_maxcol) {		/* wraps around */
		vt_col = 0;
		if (++vt_row >= vt_maxrow) {	/* on last line */
			blt(bmscrn+90*9,bmscrn+90*9*2,9*90*38);
			vt_row -= vt_winscrl;
		}
	}
}
