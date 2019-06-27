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

# define NUM	16	/* number of items per line */
#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
	register char *cp;
	register int i, n;
	char type;
	int bno, fi;
	char buf[PBSIZE];

	finit();
loop:
	printf("\nStandalone od\n\n");
	for (;;) {
		printf("Device: ");
		gets(buf);
		if ((fi = open(buf, 0)) < 0)
			continue;
		printf("type(o or x):       ");
		gets(buf);
		type = buf[0];
		printf("Starting block:     ");
		gets(buf);
		bno = atoi(buf);
		printf("\nStarting block number %d\n", bno);
		break;
	}

	for (;;) {
		lseek(fi, bno*PBSIZE, 0);
		if (read(fi, buf, PBSIZE) != PBSIZE)
			break;
		cp = &buf[0];
		i = 0;
		while (cp < &buf[PBSIZE]) {
			n = *cp++ & 0377;
			n = (n << 8) | (*cp++ & 0377);
			if ((i % NUM) == 0)
				printf("0%o ", bno*PBSIZE+i+i);
			switch(type) {
			case 'x':
				printf(" %x", n);
				break;
			default:
				printf(" 0%o", n);
				break;
			}
			if ((i++ % NUM) == NUM-1) {
				printf("\n");
			}
		}
		printf("stop?           ");
		gets(buf);
		if (buf[0] == 'y')
			break;
		bno++;
	}
	close(fi);
	printf("\n");
	goto loop;
}
