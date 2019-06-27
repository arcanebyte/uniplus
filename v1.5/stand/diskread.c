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

char rewrite;
char buf[50];
#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
        register int start, end, inc;
	int fi;

loop:
	printf("\nStandalone read disk test\n\n");
	finit();
        do  {
                printf("Device: "); gets(buf);
                fi = open(buf, 2);
        } while (fi < 0);
	printf("\nStarting disk sector:     ");
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
	printf("Rewrite data?:  ");
	gets(buf);
	if (buf[0] == 'y')
		rewrite = 1;
	else
		rewrite = 0;
	dtest(fi, start, end, inc);
	printf("\n\007\007\007\007\007Done\n");
	goto loop;
}

/*
 * dtest - do from sector 'start' to sector 'end'
 *	   with sector increment 'inc'
 */
dtest(fi, start, end, inc)
{
	register int bno;
	char buf2[PBSIZE];

	for (bno = start; bno <= end; bno += inc) {
		lseek(fi, bno*PBSIZE, 0);
		if (read(fi, buf2, PBSIZE) != PBSIZE) {
			printf("\007Bad read on block %d\n", bno);
			if (rewrite) {
				printf("overwrite data?:");
				gets(buf);
				if (buf[0] != 'y')
					continue;
			}
		}
		if (rewrite) {
			lseek(fi, bno*PBSIZE, 0);
			if (write(fi, buf2, PBSIZE) != PBSIZE)
				printf("Bad write on block %d\n", bno);
		}
	}
}
