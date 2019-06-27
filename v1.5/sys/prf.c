/* @(#)prf.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/conf.h"

/*
 * In case console is off,
 * panicstr contains argument to last call to panic.
 */
char	*panicstr;

/*
 * Scaled down version of C Library printf.
 * Only %c %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much suspended.
 * Printf should not be used for chit-chat.
 */
/* VARARGS1 */
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned int *adx;
	register int b, i, any;
	char *s;

#ifdef PRINTFSTALL
	for (c = 0; c < PRINTFSTALL; c++)
		;
#endif PRINTFSTALL
	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		(*putchar)(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if(c == 'c')
		(*putchar)(*adx);
	else if (c == 'b') {
		b	= *adx++;
		s	= (char *) *adx;
		printn((long) b, *s++);
		any	= 0;
		if (b) {
			(*putchar)('<');
			while (i = *s++) {
				if (b & (1 << (i - 1))) {
					if (any)
						(*putchar)(',');
					any	= 1;
					for (; (c = *s) > 32; s++)
						(*putchar)(c);
				}
				else
					for (; *s > 32; s++)
						;
			}
			if (any)
				(*putchar)('>');
		}
	}
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			(*putchar)(c);
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

printn(n, b)
long n;
register b;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		(*putchar)('-');
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		(*putchar)("0123456789ABCDEF"[d[i]]);
	}
}

/*
 * Panic is called on unresolvable fatal errors.
 * It syncs, prints "panic: mesg" and then loops.
 */
panic(s)
char *s;
{
	if (s && panicstr)
		printf("Double panic: %s\n", s);
	else {
		if (s)
			panicstr = s;
		update();
		printf("panic: %s\n", panicstr?panicstr:"???");
	}
	for(;;)
		idle();
}

/*
 * prdev prints a warning message.
 * dev is a block special device argument.
 */
prdev(str, dev)
char *str;
dev_t dev;
{
	register maj;

	maj = bmajor(dev);
	if (maj >= bdevcnt) {
		printf("%s on bad dev %o(8)\n", str, dev);
		return;
	}
	(*bdevsw[maj].d_print)(minor(dev), str);
}

/*
 * prcom prints a diagnostic from a device driver.
 * prt is device dependent print routine.
 */
prcom(prt, bp, er1, er2)
int (*prt)();
register struct buf *bp;
{
	(*prt)(bp->b_dev, "\nDevice error");
	printf("bn = %D er = %o,%o\n", bp->b_blkno, er1, er2);
}
