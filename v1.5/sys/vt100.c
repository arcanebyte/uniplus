/*
 * Copyright 1982 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
 * VT100 emulator
 * Called with each character destined for the console.
 * Processes control sequences and keeps track of terminal state.
 * Calls into bitmap.c actually do the I/O
 *
 * Lisa, September 2026: reworked into one state machine so that more of
 * a VT100's behaviour, and some of xterm's, works on the console:
 *  - tab stops every 8 columns at power-on, as on a VT100;
 *  - deferred wrap: writing the last column leaves the cursor there until
 *    the next printable character (termcap xn);
 *  - reverse wrap: backspace at column 0 goes to the end of the line
 *    above, and backspace just after a deferred wrap stays put, so erasing
 *    input that wrapped works (termcap bw);
 *  - scrolling regions (ESC [ t ; b r), origin mode (?6), autowrap (?7),
 *    insert mode (4), cursor visibility (?25), application cursor keys (?1);
 *  - ESC 7 / ESC 8 and ESC [ s / ESC [ u save and restore the cursor,
 *    ESC D index, ESC E next line, ESC M reverse index at the top of the
 *    region (it now blanks the new top line), ESC c reset;
 *  - ESC [ @ insert characters, X erase characters, G/` and d absolute
 *    column and row, E/F next and previous line, S/T scroll, a/e
 *    relative moves;
 *  - SGR 22, 24 and 27 turn bold/reverse and underline off;
 *  - replies to ESC [ c, ESC Z (device attributes) and ESC [ 5/6 n
 *    (status, cursor position), fed back as console input;
 *  - any parameter list, private marker or intermediate character is
 *    parsed to the end of the sequence, and ESC ( ESC ) ESC # sequences
 *    are swallowed, so unknown sequences no longer leave characters on
 *    the screen; CAN and SUB abandon a sequence.
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

#define	NROWS		38	/* text rows and columns inside the border */
#define	NCOLS		88
#define	LINEBYTES	(90*9)	/* bytes in one character row of the bitmap */

/* parser states */
#define	S_NORM		0
#define	S_ESC		1	/* after ESC */
#define	S_CSI		2	/* after ESC [ */
#define	S_SKIP		3	/* swallow one more character */

short vt_n[MAXPARAMS];		/* numeric parameters of \E[ commands */
char vt_mparam;
char vt_tabset[88] = {		/* tab stops, every 8 columns at power-on */
	0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0,  1,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0,
	1,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0,  1,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0,
	1,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0,  1,0,0,0,0,0,0,0
};
extern char bmbck, bmcolor, bmnormal;
extern char *bmscrn;
extern char kb_altkp;
extern char kb_ckm;
short vt_maxrow = NROWS;
short vt_maxcol = NCOLS;
short vt_row, vt_col;		/* cursor location (0-vt_maxrow, 0-vt_maxcol) */
short vtrow_ofs = 1;
short vtcol_ofs = 1;		/* row and column offsets */
short vt_winscrl = 1;		/* lines to scroll each time */

static char vt_state;		/* S_NORM, S_ESC, S_CSI or S_SKIP */
static char vt_priv;		/* private marker (? < = >) of this sequence */
static char vt_inter;		/* sequence has an intermediate character */
static char vt_wrap;		/* last column written, wrap before next char */
static char vt_awm = 1;		/* autowrap mode (?7) */
static char vt_irm;		/* insert mode (4) */
static char vt_decom;		/* origin mode (?6) */
static char vt_vis = 1;		/* cursor visible (?25) */
static char vt_curon;		/* cursor is drawn at vt_crow, vt_ccol */
static short vt_crow, vt_ccol;
static short vt_top = 0;	/* scrolling region */
static short vt_bot = NROWS-1;
static short vt_srow, vt_scol;	/* saved cursor (ESC 7) */
static char vt_srev, vt_sul, vt_swrap, vt_sdecom;

static int vt_coff(), vt_con(), vt_ctrl(), vt_char(), vt_lf(), vt_ri();
static int vt_scrup(), vt_scrdn(), vt_ich(), vt_sgr(), vt_reply();
static int vt_reset(), vt_dmode();
static char *vt_num();
static short vt_rmin(), vt_rmax();
int vt_esc(), vt_brck(), vt_cmd(), vt_tabdf();

/* This routine interprets the characters destined for the console and
 * performs like a VT100 .  It is implemented in terms of primitives
 * defined in bitmap.c
 */
vt_putc (c)
register char c;
{
	extern time_t lbolt;

	l2_dtrap = lbolt + l2_dtime;
	if (l2_dimmed) l2undim();

	vt_coff();
	if (c == 030 || c == 032)		/* CAN, SUB: abandon a sequence */
		vt_state = S_NORM;
	else if (c == 033)
		vt_state = S_ESC;
	else if (c < ' ' || c == 0177)		/* controls act inside sequences too */
		vt_ctrl(c);
	else switch (vt_state) {
		case S_NORM:
			vt_char(c);
			break;
		case S_ESC:
			vt_esc(c);
			break;
		case S_CSI:
			vt_brck(c);
			break;
		default:
			vt_state = S_NORM;
			break;
	}
	vt_con();
}

/* cursor off and on: the cursor is the character cell drawn inverted */
static
vt_coff()
{
	if (vt_curon) {
		bminvert(vt_crow+vtrow_ofs, vt_ccol+vtcol_ofs);
		vt_curon = 0;
	}
}

static
vt_con()
{
	if (vt_vis && !vt_curon) {
		vt_crow = vt_row;
		vt_ccol = vt_col;
		bminvert(vt_crow+vtrow_ofs, vt_ccol+vtcol_ofs);
		vt_curon = 1;
	}
}

static
vt_ctrl(c)
register char c;
{
	register int x;

	switch (c) {
		case 07:
			beep();
			break;
		case 010:		/* backspace, with reverse wrap */
			if (vt_wrap && vt_awm) {
				vt_wrap = 0;
				break;
			}
			vt_wrap = 0;
			if (vt_col > 0)
				vt_col--;
			else if (vt_awm && vt_row > 0) {
				vt_row--;
				vt_col = NCOLS-1;
			}
			break;
		case 011:
			vt_wrap = 0;
			for (x = vt_col+1; x < NCOLS-1; x++)
				if (vt_tabset[x])
					break;
			vt_col = (x < NCOLS) ? x : NCOLS-1;
			break;
		case 012:
		case 013:
		case 014:
			vt_wrap = 0;
			vt_lf();
			break;
		case 015:
			vt_wrap = 0;
			vt_col = 0;
			break;
	}
}

/* a printable character */
static
vt_char(c)
char c;
{
	if (vt_wrap) {
		vt_wrap = 0;
		vt_col = 0;
		vt_lf();
	}
	if (vt_irm)
		vt_ich(1);
	bmputc(vt_row+vtrow_ofs, vt_col+vtcol_ofs, c);
	if (vt_col < NCOLS-1)
		vt_col++;
	else if (vt_awm)
		vt_wrap = 1;
}

/* line feed: down a row, scrolling at the bottom of the region */
static
vt_lf()
{
	if (vt_row == vt_bot)
		vt_scrup(vt_top, vt_bot, 1);
	else if (vt_row < NROWS-1)
		vt_row++;
}

/* reverse line feed: up a row, scrolling down at the top of the region */
static
vt_ri()
{
	if (vt_row == vt_top)
		vt_scrdn(vt_top, vt_bot, 1);
	else if (vt_row > 0)
		vt_row--;
}

/* scroll rows t..b up n lines, blanking at the bottom */
static
vt_scrup(t, b, n)
register short t, b, n;
{
	register short r;

	if (n > b - t + 1)
		n = b - t + 1;
	if (t == 0 && b == NROWS-1 && n == 1) {	/* whole screen, as before */
		blt(bmscrn+LINEBYTES, bmscrn+LINEBYTES*2, LINEBYTES*NROWS);
		return;
	}
	for (r = t; r + n <= b; r++)
		bmcpl(r+vtrow_ofs, r+n+vtrow_ofs);
	for ( ; r <= b; r++)
		bmblank(r+vtrow_ofs);
}

/* scroll rows t..b down n lines, blanking at the top */
static
vt_scrdn(t, b, n)
register short t, b, n;
{
	register short r;

	if (n > b - t + 1)
		n = b - t + 1;
	for (r = b; r - n >= t; r--)
		bmcpl(r+vtrow_ofs, r-n+vtrow_ofs);
	for ( ; r >= t; r--)
		bmblank(r+vtrow_ofs);
}

/* insert n blank characters at the cursor, shifting the rest right */
static
vt_ich(n)
register short n;
{
	register short x;

	if (n > NCOLS - vt_col)
		n = NCOLS - vt_col;
	for (x = NCOLS-1; x >= vt_col + n; x--)
		bmmvc(vt_row+vtrow_ofs, x+vtcol_ofs, vt_row+vtrow_ofs, x-n+vtcol_ofs);
	for (x = vt_col; x < vt_col + n; x++)
		bmputc(vt_row+vtrow_ofs, x+vtcol_ofs, ' ');
}

/* the attributes: reverse (also used for bold) and underline */
static
vt_sgr(rev, ul)
int rev, ul;
{
	if (rev != (bmbck != bmnormal)) {
		bmswitch();			/* invert font table */
		bmbck = rev ? (bmnormal ? 0 : -1) : bmnormal;
	}
	bmcolor = ul ? (bmbck ? 0 : -1) : bmbck;
}

/* feed a reply back as console input */
static
vt_reply(s)
register char *s;
{
	extern char kb_chrbuf;
	register int x;

	while (*s) {
		x = spl7();
		kb_chrbuf = *s++;
		cointr(0);
		splx(x);
	}
}

/* decimal number into p, returning the end */
static char *
vt_num(p, n)
register char *p;
register int n;
{
	char d[6];
	register int i = 0;

	do {
		d[i++] = '0' + n % 10;
		n /= 10;
	} while (n && i < 6);
	while (i > 0)
		*p++ = d[--i];
	return (p);
}

/* default tab stops, every 8 columns (also for reinit.c) */
vt_tabdf()
{
	register int x;

	for (x = 0; x < NCOLS; x++)
		vt_tabset[x] = (x && (x % 8) == 0);
}

/* full reset (ESC c) */
static
vt_reset()
{
	vt_sgr(0, 0);
	bmclear();
	vt_curon = 0;
	vt_row = vt_col = 0;
	vt_top = 0;
	vt_bot = NROWS-1;
	vt_wrap = vt_irm = vt_decom = 0;
	vt_awm = vt_vis = 1;
	vt_srow = vt_scol = 0;
	vt_srev = vt_sul = vt_swrap = vt_sdecom = 0;
	vt_tabdf();
	kb_altkp = 0;
	kb_ckm = 0;
}

/*
 * Process the escape sequences to the terminal
 */
vt_esc(c)				/* after ESC key hit */
register char c;
{
	register int i;

	vt_state = S_NORM;
	switch (c) {
		case '[':
			vt_state = S_CSI;	/* check E[ sequence */
			vt_mparam = 0;
			for (i = 0; i < MAXPARAMS; i++)
				vt_n[i] = 0;
			vt_priv = vt_inter = 0;
			return;
		case '>':		/* disable alternate keypad */
			kb_altkp = 0;
			break;
		case '=':		/* enable alternate keypad */
			kb_altkp = 1;
			break;
		case 'M':		/* reverse index */
			vt_wrap = 0;
			vt_ri();
			break;
		case 'D':		/* index */
			vt_wrap = 0;
			vt_lf();
			break;
		case 'E':		/* next line */
			vt_wrap = 0;
			vt_col = 0;
			vt_lf();
			break;
		case 'H':
			vt_tabset[vt_col]=1;
			break;
		case '7':		/* save cursor */
			vt_srow = vt_row;
			vt_scol = vt_col;
			vt_srev = (bmbck != bmnormal);
			vt_sul = (bmcolor != bmbck);
			vt_swrap = vt_wrap;
			vt_sdecom = vt_decom;
			break;
		case '8':		/* restore cursor */
			vt_row = vt_srow;
			vt_col = vt_scol;
			vt_wrap = vt_swrap;
			vt_decom = vt_sdecom;
			vt_sgr(vt_srev, vt_sul);
			break;
		case 'c':		/* reset */
			vt_reset();
			break;
		case 'Z':		/* identify, as ESC [ c */
			vt_reply("\033[?1;2c");
			break;
		case '(':		/* character sets and ESC # tests: */
		case ')':		/* swallow the next character */
		case '*':
		case '+':
		case '#':
			vt_state = S_SKIP;
			break;
	}
}

vt_brck(c)					/* \E[ sequence checked here */
register char c;
{
	register tmp;

	if (c >= '0' && c <= '9') {
		tmp = (vt_n[vt_mparam] * 10) + (c - '0');
		vt_n[vt_mparam] = (tmp > MAXVT_N) ? MAXVT_N : tmp;
		return;
	}
	if (c == ';') {
		if (vt_mparam < MAXPARAMS-1)
			vt_mparam++;
		return;
	}
	if (c >= '<' && c <= '?') {		/* private marker */
		vt_priv = c;
		return;
	}
	if (c >= ' ' && c <= '/') {		/* intermediate */
		vt_inter = 1;
		return;
	}
	vt_state = S_NORM;
	if (c >= '@' && c <= '~' && !vt_inter)
		vt_cmd(c);
}

/* the first and last rows the cursor may be put on (origin mode) */
static short
vt_rmin()
{
	return (vt_decom ? vt_top : 0);
}

static short
vt_rmax()
{
	return (vt_decom ? vt_bot : NROWS-1);
}

vt_cmd(c)		/* now have last char of esc sequence */
register char c;
{
	register vt_n1 = vt_n[0];
	register vt_n2 = vt_n[1];
	register int x, y;
	int n;
	char buf[16], *p;

	if (vt_priv) {
		if (c == 'h' || c == 'l')
			vt_dmode(c == 'h');
		return;
	}
	n = vt_n1 ? vt_n1 : 1;		/* count, default 1 */
	switch (c) {
	case 'A':	/* move cursor up */
		vt_wrap = 0;
		y = (vt_row >= vt_top) ? vt_top : 0;
		vt_row = (vt_row - n > y) ? vt_row - n : y;
		break;
	case 'B':	/* move cursor down */
	case 'e':
		vt_wrap = 0;
		y = (vt_row <= vt_bot) ? vt_bot : NROWS-1;
		vt_row = (vt_row + n < y) ? vt_row + n : y;
		break;
	case 'C':	/* move cursor right */
	case 'a':
		vt_wrap = 0;
		vt_col = (vt_col + n < NCOLS) ? vt_col + n : NCOLS-1;
		break;
	case 'D':	/* move cursor left */
		vt_wrap = 0;
		vt_col = (vt_col - n > 0) ? vt_col - n : 0;
		break;
	case 'E':	/* next line(s) */
		vt_wrap = 0;
		vt_col = 0;
		y = (vt_row <= vt_bot) ? vt_bot : NROWS-1;
		vt_row = (vt_row + n < y) ? vt_row + n : y;
		break;
	case 'F':	/* previous line(s) */
		vt_wrap = 0;
		vt_col = 0;
		y = (vt_row >= vt_top) ? vt_top : 0;
		vt_row = (vt_row - n > y) ? vt_row - n : y;
		break;
	case 'G':	/* column */
	case '`':
		vt_wrap = 0;
		vt_col = (n <= NCOLS) ? n - 1 : NCOLS-1;
		break;
	case 'd':	/* row */
		vt_wrap = 0;
		y = vt_rmin() + n - 1;
		vt_row = (y <= vt_rmax()) ? y : vt_rmax();
		break;
	case 'H':	/* move cursor home */
	case 'f':
		vt_wrap = 0;
		y = vt_rmin() + (vt_n1 ? vt_n1 - 1 : 0);
		vt_row = (y <= vt_rmax()) ? y : vt_rmax();
		x = vt_n2 ? vt_n2 - 1 : 0;
		vt_col = (x < NCOLS) ? x : NCOLS-1;
		break;
	case 'J':	/* clear screen */
		vt_wrap = 0;
		if (vt_n1 == 0)
			if ((vt_row == 0) && (vt_col == 0))
				vt_n1 = 2;
		switch (vt_n1) {
		case 0:	/* clear from cursor to end */
			for (y = vt_col ; y < NCOLS ; y++ )
				bmputc(vt_row+vtrow_ofs,y+vtcol_ofs,' ');
			for (x = vt_row+1; x < NROWS ; x++ )
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
		vt_wrap = 0;
		switch (vt_n1) {
		case 0:	/* clear from cursor to end */
			for (y = vt_col ; y < NCOLS ; y++ )
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
		vt_wrap = 0;
		if (vt_row >= vt_top && vt_row <= vt_bot)
			vt_scrdn(vt_row, vt_bot, n);
		break;
	case 'M':	/* delete line(s) */
		vt_wrap = 0;
		if (vt_row >= vt_top && vt_row <= vt_bot)
			vt_scrup(vt_row, vt_bot, n);
		break;
	case 'P':	/* delete character(s) */
		vt_wrap = 0;
		if (n > NCOLS - vt_col)
			n = NCOLS - vt_col;
		for (x=vt_col, y=vt_col+n; y < NCOLS; x++, y++)
			bmmvc(vt_row+vtrow_ofs, x+vtcol_ofs,
				vt_row+vtrow_ofs, y+vtcol_ofs);
		for ( ; x < NCOLS ; x++ )
			bmputc(vt_row+vtrow_ofs,x+vtcol_ofs,' ');
		break;
	case '@':	/* insert character(s) */
		vt_wrap = 0;
		vt_ich(n);
		break;
	case 'X':	/* erase character(s) */
		vt_wrap = 0;
		for (x = vt_col; x < NCOLS && x < vt_col + n; x++)
			bmputc(vt_row+vtrow_ofs,x+vtcol_ofs,' ');
		break;
	case 'S':	/* scroll up */
		vt_scrup(vt_top, vt_bot, n);
		break;
	case 'T':	/* scroll down */
		vt_scrdn(vt_top, vt_bot, n);
		break;
	case 'r':	/* scrolling region */
		x = vt_n1 ? vt_n1 - 1 : 0;
		y = vt_n2 ? vt_n2 - 1 : NROWS-1;
		if (y > NROWS-1)
			y = NROWS-1;
		if (x < y) {
			vt_top = x;
			vt_bot = y;
			vt_wrap = 0;
			vt_row = vt_rmin();
			vt_col = 0;
		}
		break;
	case 's':	/* save cursor */
		vt_esc('7');
		break;
	case 'u':	/* restore cursor */
		vt_esc('8');
		break;
	case 'g':
		if (vt_n1 == 0)
			vt_tabset[vt_col] = 0;
		else if (vt_n1 == 3)
			for (x = 0 ; x < NCOLS ; x++ )
				vt_tabset[x] = 0;
		break;
	case 'h':	/* ANSI modes: 4 is insert */
	case 'l':
		for (x = 0; x <= vt_mparam; x++)
			if (vt_n[x] == 4)
				vt_irm = (c == 'h');
		break;
	case 'm':	/* set normal display or reverse video */
		for (x = 0; x <= vt_mparam; x++)
			switch (vt_n[x]) {
			case 0:	/* all off */
				vt_sgr(0, 0);
				break;
			case 1: case 7:	/* reverse image (bold too) */
				vt_sgr(1, bmcolor != bmbck);
				break;
			case 4:	/* underline */
				vt_sgr(bmbck != bmnormal, 1);
				break;
			case 22: case 27:	/* reverse (and bold) off */
				vt_sgr(0, bmcolor != bmbck);
				break;
			case 24:	/* underline off */
				vt_sgr(bmbck != bmnormal, 0);
				break;
			}
		break;
	case 'n':	/* device status */
		if (vt_n1 == 5)
			vt_reply("\033[0n");
		else if (vt_n1 == 6) {
			p = buf;
			*p++ = '\033';
			*p++ = '[';
			p = vt_num(p, vt_row - vt_rmin() + 1);
			*p++ = ';';
			p = vt_num(p, vt_col + 1);
			*p++ = 'R';
			*p = '\0';
			vt_reply(buf);
		}
		break;
	case 'c':	/* device attributes: a VT100 with advanced video */
		if (vt_n1 == 0)
			vt_reply("\033[?1;2c");
		break;
	}
}

/* DEC private modes, ESC [ ? n h and l */
static
vt_dmode(on)
int on;
{
	register int x;

	for (x = 0; x <= vt_mparam; x++)
		switch (vt_n[x]) {
		case 1:		/* application cursor keys */
			kb_ckm = on;
			break;
		case 6:		/* origin mode */
			vt_decom = on;
			vt_wrap = 0;
			vt_row = vt_rmin();
			vt_col = 0;
			break;
		case 7:		/* autowrap */
			vt_awm = on;
			if (!on)
				vt_wrap = 0;
			break;
		case 25:	/* cursor visible */
			vt_vis = on;
			break;
		}
}
