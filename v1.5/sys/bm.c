/*
 * Copyright 1982 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
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
#define FAST
#define NODEBUG
#include <sys/mmu.h>
#include <sys/types.h>
#include <sys/local.h>
#include <sys/bmfont.h>

char *bmscrn;		/* pointer to screen -- initialized in bminit */
char bmcolor;		/* underline color; same as bmbck for no underlining */
char bmbck;		/* current background color; 0 for white, -1 for black */
char bmnormal;		/* normal background color; 0 for white, -1 for black */
char bmfirst = 1;	/* is this the first boot? (not true for a restart) */

bminit ()		/* one time screen initialization (called from mch.s) */
{
	register short i;		/* amount of screen in longs */
	register long *p;
	extern long bblank[];

	*MEMEND -= SCRNSIZE;			/* lop off mem for screen */
	bmscrn = *MEMEND;			/* logical screen address */
	VIDADDR = ((int)(bmscrn+*MEMBASE)>>15);	/* init screen addr latch */

	i = ((SCRNSIZE - (MAXCOL * MAXROW * V_RESO)) >> 2) - 1;
	p = (long *)(bmscrn + (MAXCOL * MAXROW * V_RESO));

	do {				/* so retrace lines won't show */
		*p++ = -1;
	} while (--i != -1);
	bmcolor = 0;	/* bmcolor switches in bminit if bmbck is -1 */
	bmbck = -1;
	bmnormal = -1;
	bmclear();
	if (bmfirst) {
		bmfirst = 0;
		bmswitch();
		bmclear();
	} else {
		bmcolor = -1;
		for (i=0; i<16; i++)
			bblank[i] = -1;
		bmclear();
	}
}

bmswitch()
{
	bmcolor = (bmcolor ? 0 : -1 );
	bmifont();
}

bmifont()		/* invert the font table (set or reset rev. video) */
{
	register short i;		/* amount of screen in longs */
	register char *f, g;
	extern long bblank[];

	i = sizeof(bmfont) - 1;
	f = bmfont;
	do {
		g = ~*f;
		*f++ = g;
	}
	while (--i != -1);
	for (i=0; i<16; i++)
		bblank[i] = ( bblank[i] ? 0 : -1);
}

bmsinv()		/* invert the entire screen */
{
	register short i;		/* amount of screen in chars */
	register char *p, q;

	i = MAXCOL * MAXROW * V_RESO;
	p = bmscrn;

	do {
		q = ~*p;
		*p++ = q;
	} while (--i != 0);
}

#ifndef FAST
/* Modify a section of the display
 * Cmd is 0 to clear the area, positive to scroll up that many lines
 * or negative to scroll down that many.
 */
bmwin (cmd, tr, br, lc, rc)
	short cmd, br, lc, rc;
	register short tr;
{
	register short wrap, j, i;
	short cols, rows;
	register char *pc, *ec;
	register char color = bmcolor;

	pc = bmscrn;

#ifndef NODEBUG
	if (tr<0||tr>=MAXROW||br>=MAXCOL||br<tr
	    ||lc<0||lc>=MAXCOL||rc>=MAXCOL||rc<=lc) {
		printf("bmwin(%s,%d,%d,%d,%d) is illegal\n",
			cmd?"scroll":"clear", tr, br, lc, rc);
		return;
	}
#endif

/* Calculate number of logical lines to operate on */
	rows = br - tr + 1;

/* Calculate number of columns (bytes) accross screen */
	cols = rc - lc + 1;

/* Wrap is an offset to be added to scan line pointers at the end of
 * each line to bring them around to the start of the next line.
 * It is the number of columns not being scrolled (usually zero)
 */
	wrap = BPL - cols;		/* wrap around offset */

/* Pc points at the upper area of the screen (ie, that which will
 * receive the scroll data).
 */
	pc += tr * (short)(BPL * V_RESO) + lc;

/* I is the number of scan lines to operate on.  ie. the number of
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
	do {			/* clear */
		j = cols - 1;
		do *pc++ = color;
		while (--j != -1);
		pc += wrap;
	} while (--i != -1);
	return;
}
#endif

bminvert (r, c)			/* reverse video char under cursor */
	register short r, c;
{
	register char *p = bmscrn;
	register short i;

	p += (r * (BPL * V_RESO)) + c;
	i = V_RESO;
	do {
		*p = ~*p;
		p += BPL;		/* 1 scan line */
	} while (--i != 0);
}
		
bmmvc (dr, dc, sr, sc)			/* copy char at (sr,sc) to (dr,dc) */
register short dr, dc;
register short sr, sc;
{
	register char *dest, *src;
	register short i;

	dest = bmscrn + (dr * (BPL * V_RESO)) + dc;
	src = bmscrn + (sr * (BPL * V_RESO)) + sc;
	i = V_RESO;
	do {
		*dest = *src;
		dest += BPL;		/* 1 scan line */
		src += BPL;		/* 1 scan line */
	} while (--i != 0);
}

#ifndef FAST
bmcpl(dl, sl)				/* copy line sl to dl */
register dl, sl;
{
	register char *dest, *src;
	register short i;

	dest = bmscrn + (dl * (BPL * V_RESO));
	src = bmscrn + (sl * (BPL * V_RESO));
	i = BPL * V_RESO;
	do {
		*dest = *src;
		dest++;			/* 1 scan line */
		src++;			/* 1 scan line */
	} while (--i != 0);
}
#else FAST
bmcpl(dl, sl)				/* copy line sl to dl */
register dl, sl;
{
	register char *dest, *src;

	dest = bmscrn + (dl * (BPL * V_RESO));	/* register a5 */
	src = bmscrn + (sl * (BPL * V_RESO));	/* register a4 */

	/* Use the 13 registers a6, a3-a0 and d7-d0 to copy 52 bytes at a time. */
	asm(" moveml	#0xFFFF,sp@- ");	/* save all the registers */
	asm(" moveml	a4@,#0x4FFF ");		/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@ ");
	asm(" moveml	a4@(52),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(52) ");
	asm(" moveml	a4@(104),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(104) ");
	asm(" moveml	a4@(156),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(156) ");
	asm(" moveml	a4@(208),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(208) ");
	asm(" moveml	a4@(260),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(260) ");
	asm(" moveml	a4@(312),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(312) ");
	asm(" moveml	a4@(364),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(364) ");
	asm(" moveml	a4@(416),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(416) ");
	asm(" moveml	a4@(468),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(468) ");
	asm(" moveml	a4@(520),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(520) ");
	asm(" moveml	a4@(572),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(572) ");
	asm(" moveml	a4@(624),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(624) ");
	asm(" moveml	a4@(676),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(676) ");
	asm(" moveml	a4@(728),#0x4FFF ");	/* move 52 bytes */
	asm(" moveml	#0x4FFF,a5@(728) ");
	asm(" moveml	a4@(780),#0x7F ");	/* move 28 more bytes */
	asm(" moveml	#0x7F,a5@(780) ");
	asm(" movw	a4@(808),a5@(808) ");	/* move last 2 bytes */
	asm(" moveml	sp@+,#0xFFFF ");
#ifdef lint
	if (dest)
		return;
	else if (src)
		return;
#endif lint
}
#endif FAST

#ifndef FAST
bmblank(dl)				/* blank line dl */
register dl;
{
	register char *dest;
	register short i;

	dest = bmscrn + (dl * (BPL * V_RESO));
	i = BPL * V_RESO;
	do {
		*dest = bmnormal;
		dest++;			/* 1 scan line */
	} while (--i != 0);
}
#else FAST
bmblank(dl)				/* blank line dl */
register dl;
{
	register char *dest, *a4;	/* NOTUSED */
	extern long bblank[];

	dest = bmscrn + (dl * (BPL * V_RESO));	/* register a5 */
	asm(" moveml	#0xFFFF,sp@- ");	/* save all the registers */
	a4 = (char *)bblank;
	asm(" moveml	a4@,#0x4FFF ");		/* set a6,a3-a0,d7-d0 to blanks */
	asm(" movl	a6,a4 ");		/* set a4 to blanks */

	asm(" moveml	#0x5FFF,a5@ ");		/* move 56 bytes */
	asm(" moveml	#0x5FFF,a5@(56) ");
	asm(" moveml	#0x5FFF,a5@(112) ");
	asm(" moveml	#0x5FFF,a5@(168) ");
	asm(" moveml	#0x5FFF,a5@(224) ");
	asm(" moveml	#0x5FFF,a5@(280) ");
	asm(" moveml	#0x5FFF,a5@(336) ");
	asm(" moveml	#0x5FFF,a5@(392) ");
	asm(" moveml	#0x5FFF,a5@(448) ");
	asm(" moveml	#0x5FFF,a5@(504) ");
	asm(" moveml	#0x5FFF,a5@(560) ");
	asm(" moveml	#0x5FFF,a5@(616) ");
	asm(" moveml	#0x5FFF,a5@(672) ");
	asm(" moveml	#0x5FFF,a5@(728) ");
	asm(" moveml	#0x3F,a5@(784) ");	/* move another 24 bytes */
	asm(" movw	_bblank,a5@(808) ");	/* move last 2 bytes */
	asm(" moveml	sp@+,#0xFFFF ");
#ifdef lint
	if (dest)
		return;
	else if (a4)
		return;
#endif lint
}
#endif FAST

bmputc (r, c, k)			/* draw a char at location r, c */
	register short r, c;
	char k;
{
	register char *p = bmscrn;
	register char *f = &bmfont[0];
	register short i;

#ifndef NODEBUG
	if (r < 0 || c < 0 || r >= MAXROW || c >= MAXCOL) {
		printf("bmputc(%d, %d, `%c') is illegal\n",r,c,k);
		return;
	}
#endif

	p += (r * (BPL * V_RESO)) + c;
	f += (k - ' ') << 3;

	i = FONTVERT - 1;		/* bytes in font table for one char */
	do {
		*p = *f++;
		p += BPL;		/* 1 scan line == 90 bytes */
	} while (--i != -1);
	*p = bmcolor;			/* blank row under cursor */
}

long bblank[16];
int savesp;

bmclear()
{
	int x;

#ifdef lint
	savesp++;
#endif
	x = spl7();
	asm(" moveml	#0xFFFF,sp@- ");
	asm(" movl	sp,_savesp ");
	asm(" moveml	_bblank,#0xFFFF ");
	asm(" movl	_bmscrn,sp ");
	asm(" addl	#32400,sp ");
	asm("1$: moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" moveml	#0xFFFE,sp@- ");
	asm(" cmpl	_bmscrn,sp ");
	asm(" bne	1$ ");
	asm(" movl	_savesp,sp ");
	asm(" moveml	sp@+,#0xFFFF ");
	splx(x);
}

#ifndef FAST
bmrscrl()
{
	register i = 29969;
	register char *dest, *src;

	dest = bmscrn+30779;
	src = bmscrn+29969;
	do {
		*dest-- = *src--;
	} while (--i != -1);
}
#else FAST
/*
 * move 37 lines of 9 lines of pixels times 90 columns
 * which is 29970 bytes to bmscrn+29970 from
 * bmscrn+ 30780
 * use all of the registers except a6,a5,d0 and a7
 */
bmrscrl()
{
	asm(" moveml	#0xFFFF,sp@- ");	/* save all the registers */
	asm(" movl	_bmscrn,a6 ");
	asm(" movl	_bmscrn,a5 ");
	asm(" addl	#30732,a5 ");		/* destination - 48 bytes */
	asm(" addl	#29922,a6 ");		/* source - 48 bytes */
	asm(" movl	#77,d0 ");		/* 77 times -1 */
	asm("1$: moveml	a6@,#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@ ");
	asm(" moveml	a6@(-48),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-48) ");
	asm(" moveml	a6@(-96),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-96) ");
	asm(" moveml	a6@(-144),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-144) ");
	asm(" moveml	a6@(-192),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-192) ");
	asm(" moveml	a6@(-240),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-240) ");
	asm(" moveml	a6@(-288),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-288) ");
	asm(" moveml	a6@(-336),#0x1FFE ");
	asm(" moveml	#0x1FFE,a5@(-336) ");
	asm(" subw 	#384,a6 ");
	asm(" subw 	#384,a5 ");

	asm(" dbra	d0,1$ ");
	asm(" moveml	a6@(32),#0x1E ");	/* move 16 more bytes) */
	asm(" moveml	#0x1E,a5@(32) ");
	asm(" movw	a6@(30),a5@(30) ");
	asm(" moveml	sp@+,#0xFFFF ");
}
#endif FAST
