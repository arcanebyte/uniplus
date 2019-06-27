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
	printf("\nStandalone disk test\n\n");
	finit();
        do  {
                printf("Device: "); gets(buf);
                fi = open(buf, 2);
# ifdef DEBUG
		printf("fdesc=%d\n", fi);
# endif
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
	dtest(fi, start, end, inc, 0x00);
	dtest(fi, start, end, inc, 0xFF);
	dtest(fi, start, end, inc, 0xAA);
	dtest(fi, start, end, inc, 0x55);
	dtest(fi, start, end, inc, -1);
	dtest(fi, start, end, inc, -2);
	printf("\nDone\n");
	goto loop;
}

char buf1[PBSIZE], buf2[PBSIZE];
/*
 * dtest - run a disk test from sector 'start' to sector 'end'
 *	   with sector increment 'inc' and byte pattern 'pat'
 */
dtest(fi, start, end, inc, pat)
{
	register int i, bno;

	if (pat == -1) {
		printf("PATTERN = RAMP\n");
		for (i = 0; i < PBSIZE; i++)	/* load pattern */
			buf1[i] = i;
	} else if (pat == -2) {
		printf("PATTERN = BLOCK NUMBER\n");
	} else {
		printf("PATTERN = %x\n", pat);
		for (i = 0; i < PBSIZE; i++)	/* load pattern */
			buf1[i] = pat;
	}
	for (bno = start; bno <= end; bno += inc) {
		if (pat == -2) {
			for (i = 0; i < PBSIZE; i++)	/* load pattern */
				buf1[i] = bno;
		}
		lseek(fi, bno*PBSIZE, 0);
		if (write(fi, buf1, PBSIZE) != PBSIZE)
			printf("Bad write on block %d\n", bno);
	}
	for (bno = start; bno <= end; bno += inc) {
		for (i = 0; i < PBSIZE; i++)	/* clear pattern */
			buf2[i] = 0xFE;
		lseek(fi, bno*PBSIZE, 0);
		if (read(fi, buf2, PBSIZE) != PBSIZE) {
			printf("Bad read on block %d\n", bno);
			continue;
		}
		/* check pattern */
		if (pat == -2) {
			for (i = 0; i < PBSIZE; i++)	/* load pattern */
				buf1[i] = bno;
		}
		for (i = 0; i < PBSIZE; i++)
			if (buf1[i] != buf2[i]) {
				printf("Bad read compare on block %d", bno);
				printf(" index %d expected %x got %x\n",
					i, buf1[i], buf2[i]);
				break;
			}
	}
}
