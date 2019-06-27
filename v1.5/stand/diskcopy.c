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

#define NMOVE	10	/* number of blocks to move at once */

char buf1[NMOVE][PBSIZE];
int ok[NMOVE];

main()
{
	register from, to, ct;
	int i, bno, fi, fo;
        char buf[50];

loop:
	printf("\nStandalone disk copy program\n\n");
	finit();
        for (;;) {
                printf("From device:     "); gets(buf);
                fi = open(buf, 0);
		if (fi < 0) continue;
		printf("Starting sector: "); gets(buf);
		from = atoi(buf);
                printf("To device:       "); gets(buf);
                fo = open(buf, 2);
		if (fo < 0) {
			close(fi);
			continue;
		}
		printf("Starting sector: "); gets(buf);
		to = atoi(buf);
		printf("Number of sectors: "); gets(buf);
		ct = atoi(buf);
		if (from < 0 || to < 0 || ct <= 0) {
			printf("Illegal number\n\n");
			continue;
		}
		break;
        }

	for (bno = 0; bno < ct; bno += NMOVE) {
		for (i = 0; i < NMOVE; i++) {
			lseek(fi, (bno+from+i)*PBSIZE, 0);
			if ((ok[i] = read(fi, &buf1[i][0], PBSIZE)) != PBSIZE) {
				printf("Bad read on block %d\n", bno+from+i);
				continue;
			}
		}
		for (i = 0; i < NMOVE; i++) {
			if (ok[i] == PBSIZE) {
				lseek(fo, (bno+to+i)*PBSIZE, 0);
				if (write(fo, &buf1[i][0], PBSIZE) != PBSIZE)
					printf("Bad write on block %d\n",
						bno+to+i);
			}
		}
	}
	goto loop;
}
