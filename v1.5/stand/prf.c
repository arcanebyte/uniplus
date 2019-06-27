/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/sysmacros.h>
#include	<sys/inode.h>
#include	<saio.h>

#define LCASEONLY
/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
/* VARARGS1 */
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned *adx;
	char *s;

# ifdef PRINTFSTALL
# ifdef DEBUG
	if (debug & D_SLOWO)
		for (c = 0; c < 70000; c++) ;
# endif
		for (c = 0; c < 13300; c++) ;
# endif PRINTFSTALL
	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn(*adx, (unsigned)(c=='o'? 8: (c=='x'? 16:10)));
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	} else if (c == 'D') {
		printn(*adx, (unsigned)10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	} else if (c == 'c')
		putchar((int)*adx);
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
unsigned n;
unsigned b;
{
	register unsigned a;

	if(a = n/b)
		printn(a, b);
	putchar("0123456789ABCDEF"[(int)(n%b)]);
}

gets(buf)
char	*buf;
{
register char *lp;
register c;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering gets\n");
# endif
	lp = buf;
	for (;;) {
		c = getchar() & 0177;
# ifdef LCASEONLY
		if (c>='A' && c<='Z')
			c -= 'A' - 'a';
# endif LCASEONLY
		if (lp != buf && *(lp-1) == '\\') {
			lp--;
# ifdef LCASEONLY
			if (c>='a' && c<='z') {
				c += 'A' - 'a';
				goto store;
			}
# endif LCASEONLY
			switch ( c) {
			case '(':
				c = '{';
				break;
			case ')':
				c = '}';
				break;
			case '!':
				c = '|';
				break;
			case '^':
				c = '~';
				break;
			case '\'':
				c = '`';
				break;
			}
		}
	store:
		switch(c) {
		case '\n':
		case '\r':
			c = '\n';
			*lp++ = '\0';
# ifdef DEBUG
			if (debug & D_INOUT)
				printf("Leaving gets\n");
# endif
			goto leave;
		case '\b':
		case '#':
			lp--;
			if (lp < buf)
				lp = buf;
			continue;
		case '@':
			lp = buf;
			putchar('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
leave:
	;
}
