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
	register int start;
	int fi;
        char buf[50];

loop:
	printf("\nStandalone disk fix\n\n");
	finit();
        do  {
                printf("device: "); gets(buf);
                fi = open(buf, 2);
# ifdef DEBUG
		printf("fdesc=%d\n", fi);
# endif
        } while (fi < 0);
	for (;;) {
		printf("\nDisk sector: ");
		gets(buf);
		start = atoi(buf);
		if (start < 0)
			break;
		dfix(fi, start);
	}
	printf("\nDone\n");
	goto loop;
}

char buf1[PBSIZE];
dfix(fi, bno)
{
	register int i, byte, value;
        char buf[50];

	lseek(fi, bno*PBSIZE, 0);
	if (read(fi, buf1, PBSIZE) != PBSIZE) {
		printf("Bad read on block %d\n", bno);
		return;
	}
	for ( i = 0; i < PBSIZE; i++) {
		if (i % 16 == 0)
			printf("\n");
		if (i == 256)
			gets(buf);
		printf(" %x", buf1[i] & 0xFF);
	}
	printf("\n");
	for (;;) {
		printf("byte: ");
		gets(buf);
		byte = atoi(buf);
		if (byte < 0 || byte >=PBSIZE)
			break;
		printf("value: ");
		gets(buf);
		value = atoi(buf);
		if (value < 0 || value > 255)
			break;
		buf1[byte] = value;
	}
	lseek(fi, bno*PBSIZE, 0);
	if (write(fi, buf1, PBSIZE) != PBSIZE)
		printf("Bad write on block %d\n", bno);
}
