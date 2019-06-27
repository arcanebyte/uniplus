#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/inode.h"
#include "saio.h"
long bmcolor = 0;	/* white (0) or black (-1) background */
char *bmscrn;		/* pointer to screen */
long icon[] = {  0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
		 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F,
};
/* Standalone console interface
 */
/*
 * Copyright 1984 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
 * Console Interface
 * getlocal checks the COPS interrupt flag register looking for a pending
 * character code.  If it finds one then it takes the appropriate action.
 * Only when a down keycode for a transitable character is hit does it
 * return the ascii value, otherwise it returns zero.
 * getchar calls getlocal until it returns a character.
 * getchar echos, getlocal does not.
 */

#include "sys/cops.h"
#include "sys/keyboard.h"
#include "sys/bmfont.h"

int	consinit;

coinit()
{
	register struct device_e *p = COPSADDR;

	p->e_pcr = 9;
	p->e_acr = 1;
	p->e_ier = 0x7F;
/* 	printf("pcr=%x, acr=%x, ddra=%x, ddrb=%x, ier=%x, irb=%x\n",
		p->e_pcr & 0xFF, p->e_acr & 0xFF, p->e_ddra & 0xFF,
		p->e_ddrb & 0xFF, p->e_ifr & 0xFF, p->e_irb & 0xFF); */
	consinit = 1;
}

char *co_cvtab = ToLA;		/* pointer to appropriate conv table */
	/* This proceedure is used to send commands to the Keyboard COPS
	 * (ie. `enable keyboard')
	 */
int keytype;			/* type of keyboard */
getlocal()
{
	register struct device_e *p = COPSADDR;
	register char i;
	static int state;	/* State for muli-byte COPS sequences */

	if (!consinit)
		coinit();

	i = p->e_ifr;
	if ((i & FCA1) == 0) {
		/* printf("%x ",i & 0xFF); */
		return 0;
	}
	i = p->e_ira & 0xFF;

	if (!state) {			/* IDLE LOOP */
		if (!i) {
			/* printf("Mouse state:"); */
			state = 2;	/* mouse data follows */
		} else if (i == 0x80) {
			state = -1;
		} else if ((i & 0x7F) > 0x1F) {
			return keycode(i);
		} /* else
			printf("Plug code x%x\n",i); */
	} else {
		if (state == -1) {
			state = 0;
			if (i <= 0xDF) {	/* i is new keyboard ID code */
				keytype = i;
			} else if (i <= 0xEF) {		/* read clock data */
				printf(" CLOCK READ\n");
				state = 5;
			} else {
				if ((i & 0xFF) == 0xFB)
					rom_mon(icon,"RESTART",0);
				/* printf(" ??? [%x] ???\n",i); */
			}
		} else {
			/* printf(" %2.2x",i&0xFF); */
			if (--state == 0)
				/* printf("\n") */ ;
		}
	}
	return 0;
}

#define COLOCKFLG 1
#define COCTRLFLG 2
#define COSHFTFLG 4
keycode(c)
	register int c;
{
	register int i;
	static char coctrl, colock, coshft;

/* printf("cointr[%x]\n",c&0xFF); */

	if (c & 0x80) {				/* key went down */
		c &= 0x7F;			/* knock out high bit */
		if (c < Lck) {		/* it is really a character */
			if ((i=co_cvtab[c]) == Imp) {	/* cvt keycode to char*/
				printf("Got invalid down-keycode `x%x'\n",c);
				return 0;
			}
			if (coctrl) i &= 0x1F;
			return i;
		} else if (c > Lck) {	
			if (c == Sft) coshft = 1; 	/* Shift key */
			else coctrl = 1;		/* Command key */
		} else
			colock = 1;			/* Alpha lock */
	} else {				/* key went up */
		c &= 0x7F;
		if (c > Lck) {
			if (c == Cmd) coctrl = 0;	/* ctrl key */
			else coshft = 0;		/* shift key */
		} else
			colock = 0;			/* Alpha lock */
	}

/* Some shift key changed so we must reset the conversion table
 */
	co_cvtab = coshft ? ToUA		/* uppercase everything */
			: (colock ? ToCC	/* uppercase alpha only */
			: ToLA);		/* lowercase ascii */
	return 0;
}

/*
 * Adm3a emulator
 * Called with each character destined for the console.
 * Processes control sequences and keeps track of terminal state.
 * Calls into bitmap.c actually do the I/O
 */

short maxrow = 33;
short maxcol = 81;
short adm_row = 22;
short adm_col = 0;		/* cursor location (0-maxrow, 0-maxcol) */
short row_ofs = 4, col_ofs =4;	/* row and column offsets */
short winscrl = 8;		/* lines to scroll each time */

/* This routine interprets the characters destined for the console and
 * performs like a Lear Adm3a.  It is implemented in terms of primitives
 * defined in bitmap.c
 */
adm_putc (c)
	register char c;
{
	static int flag;		/* in cursor relocation sequence */
	char *bminit();			/* one time screen initialization */

	if (!bmscrn) bminit();;
	c &= 0x7F;
	if (flag) {			/* process cursor relocation */
		flag++;			/* setup to pick up next character */
		if (flag == 2) {
			if (c != '=') goto reset;
		} else if (flag == 3) {
			if ((c -= ' ') >= 0 && c <= maxrow) adm_row = c;
		} else {
			if ((c -= ' ') >= 0 && c <= maxcol) {
				adm_col = c;
			}
		reset:
			flag = 0;
			bminvert(adm_row+row_ofs,adm_col+col_ofs);
		}
		return;
	}

	if (c >= ' ') {
		c &= 0x7F;
		bmputc(adm_row+row_ofs,adm_col+col_ofs,c);
		adm_advance();
	} else {
		bminvert (adm_row+row_ofs, adm_col+col_ofs);
		if (c == 8) {		/* Backspace */
			if (adm_col > 0) adm_col--;
		} else if (c == 13) {	/* Carriage return */
			adm_col = 0;
		} else if (c == 10) {	/* Line feed */
			if (adm_row++ >= maxrow) {
				if (row_ofs > 0)
					bmwin(winscrl,row_ofs-1,row_ofs+maxrow+1,col_ofs-1,col_ofs+maxcol+1);
				else
					bmwin(winscrl,row_ofs,row_ofs+maxrow,col_ofs,col_ofs+maxcol);
				adm_row -= winscrl;
			}
		} else if (c == 26) {	/* ^Z means clear screen and home */
			adm_clear();
		} else if (c == 12) {	/* ^L is non-destructive space */
			adm_advance();
		} else if (c == 11) {	/* ^K is up a line, same column */
			if (adm_row > 0) {
				adm_row--;
			}
		} else if (c == 30) {	/* ^^ is home */
			adm_row = 0;
			adm_col = 0;
		} else if (c == 0x1B) {	/* Escape character */
			flag = 1;
			return;
		}
	}
	bminvert(adm_row+row_ofs,adm_col+col_ofs);
}

adm_advance()
{
	if (adm_col++ >= maxcol) {		/* wraps around */
		adm_col = 0;
		if (adm_row++ >= maxrow) {	/* on last line */
			if (row_ofs > 0)
				bmwin(winscrl,row_ofs-1,row_ofs+maxrow+1,col_ofs-1,col_ofs+maxcol+1);
			else
				bmwin(winscrl,row_ofs,row_ofs+maxrow,col_ofs,col_ofs+maxcol);
			adm_row -= winscrl;
		}
	}
}

adm_clear()
{
	if (row_ofs)
		bmwin(0,row_ofs-1,row_ofs+maxrow+1,col_ofs-1,col_ofs+maxcol+1);
	else
		bmwin(0,row_ofs,row_ofs+maxrow,col_ofs,col_ofs+maxcol);
	adm_row = adm_col = 0;
}
/*
 * Bitmap Display Driver
 *
 * The screen is 720 by 364 raster units.  A comfortable pixel size is
 * 9 by 10 giving 80 columns (9 wide) and 36 lines (10 deep).  The
 * extra 4 raster rows are unused.  Unfortunately this arrangement is
 * very expensive in processor time since pixels lie across byte boundaries.
 *
 * For the time being the pixel size used will be 8 by 9 providing a
 * screen area of 90 columns, and 40 lines.
 *
 * This is the low level display driver intended to simplify the task
 * of writing higher level screen emulators by simulating just the common
 * aspects of all crts in the following functions:
 *
 *	bminit ()		clears entire display
 *	bminvert (row, col)	invert character at row, column
 *	bmputc (r, c, char)	put the character at row, column
 *	bmwin (cmd, t, b, l, r) modifies window (top,bot,left,right);
 *				cmd == 1 means scroll up a line
 *				cmd == 0 means clear that area
 */

#define BPL	90
#define V_RESO	9
#define H_RESO	8


/* Note: since the last four words must be black (-1) to prevent the
 * retrace lines from showing we get away with one loop (0 - 363).
 * If a white screen is used the low bit of the last word written must
 * be a one so the loop below will have to change.
 */
char *
bminit ()		/* one time screen initialization */
{
/* register short i = ((720 * 364) >> 5) -1;/* amount of screen in longs */
	register long *p;

	p = (long *)(TOTMEM - 0x8000);	/* setup logical screen address */
	bmscrn = (char *)p;

/* So we fit in the space left by the boot rom */

	VIDADDR = ((long)p + MEMSTART) >> 15;

/*****
	do {
		*p++ = -1;
	} while (--i != -1);
*****/
	if (winscrl != 1)
		bmwin(0,row_ofs+adm_row-1,row_ofs+maxrow+1,col_ofs-1,col_ofs+maxcol+1);
	else
		bmwin(0,row_ofs-1,row_ofs+maxrow+1,col_ofs-1,col_ofs+maxcol+1);
	bminvert(row_ofs,col_ofs);
	return bmscrn;
}

/* Modify a section of the display
 * Currently supported commands are 0 for clear, and 1 for scroll up.
 */
bmwin (cmd, tr, br, lc, rc)
	short cmd, br, lc, rc;
	register short tr;
{
	register short wrap, j, i;
	short cols, rows;
	register char *pc, *ec;

	if (!(pc = bmscrn))
		pc = bminit();

/* Calculate number of logical lines to operate on */
	rows = br - tr + 1;

/* Calculate number of columns (bytes) accross screen */
	cols = rc - lc + 1;

/* Wrap is an offset to be added to scan line pointers at the end of
 * each line to bring them around to the start of the next line.
 * It is the number of columns not being scrolled (usually zero)
 */
	wrap = BPL - cols;		/* wrap around offset */

/* Pc points at the upper area of the screen (ie, that will receive the 
 * scroll data.
 */
	pc += tr * (short)(BPL * V_RESO) + lc;

/* I is the number of scan lines to operate on.  ie. the number of scan
 * lines in the window minus the number of lines to scroll.
 */
	i = (short)(rows - cmd) * (short)(V_RESO) - 1; /* scan lines */

/* Here we do the scrolling if it is indicated */
	if (cmd) {			/* scroll command lines */

/* Ec is is set to point at the first line to scroll up */
		ec = pc + cmd * ((short)(BPL) * (short)(V_RESO));

		do {			/* scroll scan line at a time */
			j = cols - 1;		/* column counter */
			do *pc++ = *ec++;	/* optimizes to dbra loop */
			while (--j != -1);
			pc += wrap;		/* skip to next scan line */
			ec += wrap;
		} while (--i != -1);

		i = (short)(V_RESO) * cmd - 1;	/* reset for clear */
	}
	tr = bmcolor;
	do {			/* clear */
		j = cols - 1;
		do *pc++ = tr;
		while (--j != -1);
		pc += wrap;
	} while (--i != -1);
	return;
}

bminvert (r, c)			/* reverse video char under cursor */
	register short r, c;
{
	register char *p = bmscrn;
	register short i;

	if (!p)
		p = bminit();
	p += (r * (BPL * V_RESO)) + c;
	i = V_RESO - 1;
	do {
		*p = ~*p;
		p += BPL;		/* 1 scan line */
	} while (--i != -1);
}
		
bmputc (r, c, k)			/* draw a char at location r, c */
	register short r, c;
	char k;
{
	register char *p = bmscrn;
	register char *f = &bmfont[0];
	register short i;

	if (!p)
		p = bminit();

	p += (r * (BPL * V_RESO)) + c;
	f += (k - ' ') << 3;

	i = FONTVERT - 1;		/* bytes in font table for one char */
	if (bmcolor) {
		do {
			*p = ~*f++;	/* should probably reverse font table*/
			p += BPL;	/* 1 scan line == 90 bytes */
		} while (--i != -1);
	} else {
		do {
			*p = *f++;
			p += BPL;	/* 1 scan line == 90 bytes */
		} while (--i != -1);
	}
	*p = bmcolor;			/* blank row under cursor */
}
