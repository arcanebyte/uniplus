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
	register int start, end, inc;
	int fi;
        char buf[50];

loop:
	printf("\nStandalone streaming backup \n\n");
	finit();
        do  {
                printf("Device: "); gets(buf);
                fi = open(buf, 2);
# ifdef DEBUG
		printf("fdesc=%d\n", fi);
# endif
        } while (fi < 0);
	printf("\nInsert Cassette ");

	gets(buf);
	start = atoi(buf);
	printf("Ending disk sector:       ");
	gets(buf);
	end = atoi(buf);
	printf("Disk sector increment:    ");
	gets(buf);
	inc = atoi(buf);
	if (start < 0 || end < 0 || inc <= 0) {
		printf("Illegal number\n\n");
		goto loop;
	}
	printf("\nDone\n");
	goto loop;
}

	for (bno = start; bno <= end; bno += inc) {
		lseek(fi, bno*PBSIZE, 0);
		if (read(fi, buf2, PBSIZE) != PBSIZE) {
			printf("Bad read on block %d\n", bno);
			continue;
		}

	for (bno = start; bno <= end; bno += inc) {
		lseek(fi, bno*PBSIZE, 0);
		if (write(fi, buf1, PBSIZE) != PBSIZE)
			printf("Bad write on block %d\n", bno);
	}
}
