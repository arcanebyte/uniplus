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

#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
	int fi;
        char buf[50];
	char *p, *conv();
	int from, to;

	printf("\nStandalone disk test\n\n");
	finit();
        do  {
                printf("Device: "); gets(buf);
                fi = open(buf, 2);
        } while (fi < 0);
loop:
	printf("Command: c(clear),r(read),w(write) <from> <to> ");
	gets(buf);
	switch (buf[0]) {
		case 'c': case 'r': case 'w':	break;
		default:	goto loop;
	}
	p = conv(&buf[1], &from);
	if (from < 0)
		goto loop;
	p = conv(p, &to);
	if (to < 0)
		goto loop;
	dtest(fi, buf[0], from, to);
	goto loop;
}

char *
conv(p, v)
register char *p;
int *v;
{
	register i = 0;

	while (*p && *p < '0' || *p > '9')
		p++;
	while (*p >= '0' && *p <= '9')
		i = i * 10 + *p++ - '0';
	*v = i;
	return (p);
}

#define IBS	(BSIZE>>2)
#define IBS2	(BSIZE>>3)

dtest(fi, op, from, to)
{
	int buf[IBS];
	register int i, bno;
	int a, b;

	lseek(fi, from*BSIZE, 0);
	switch(op) {

	case 'c':
		printf("CLEAR %d-%d\n", from, to);
		for (i = 0; i < IBS; i++)
			buf[i] = -1;
		for (bno = from; bno <= to; bno++)
			if (write(fi, buf, BSIZE) != BSIZE)
				printf("Bad write on block %d\n", bno);
		break;
	case 'w':
		printf("WRITE %d-%d\n", from, to);
		for (bno = from; bno <= to; bno++) {
			for (i = 0; i < IBS; i++)
				buf[i] = bno;
			if (write(fi, buf, BSIZE) != BSIZE)
				printf("Bad write on block %d\n", bno);
		}
		break;
	case 'r':
		printf("READ %d-%d\n", from, to);
		for (bno = from; bno <= to; bno++) {
			if (read(fi, buf, BSIZE) != BSIZE) {
				printf("Bad read on block %d\n", bno);
				break;
			}
			a = buf[0];
			for (i = 1; i < IBS2; i++)
				if (a != buf[i]) {
					a = -2;
					break;
				}
			b = buf[IBS2];
			for (i = IBS2+1; i < IBS; i++)
				if (b != buf[i]) {
					b = -2;
					break;
				}
			if (a == b) {
				if (a == -2)
					printf("%d=X ", bno);
				else if (a == -1)
					printf("%d=C ", bno);
				else if (a != bno)
					printf("%d=%d ", bno, a);
			} else {
				printf("%d=", bno);
				if (a == -2)
					printf("X,");
				else if (a == -1)
					printf("C,");
				else
					printf("%d,", bno, a);
				if (b == -2)
					printf("X ");
				else if (b == -1)
					printf("C ");
				else
					printf("%d ", bno, a);
			}
		}
		break;
	}
}
