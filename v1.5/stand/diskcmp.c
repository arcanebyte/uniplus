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
	register from1, from2, ct;
	int i, bno, fi1, fi2;
        char buf[50], buf1[PBSIZE], buf2[PBSIZE];

loop:
	printf("\nStandalone disk compare program\n\n");
	finit();
        for (;;) {
                printf("Device 1:        "); gets(buf);
                fi1 = open(buf, 2);
		if (fi1 < 0) continue;
		printf("Starting sector: "); gets(buf);
		from1 = atoi(buf);
                printf("Device 2:        "); gets(buf);
                fi2 = open(buf, 2);
		if (fi2 < 0) {
			close(fi1);
			continue;
		}
		printf("Starting sector: "); gets(buf);
		from2 = atoi(buf);
		printf("Number of sectors: "); gets(buf);
		ct = atoi(buf);
		if (from1 < 0 || from2 < 0 || ct <= 0) {
			printf("Illegal number\n\n");
			continue;
		}
		break;
        }

	for (bno = 0; bno < ct; bno++) {
		lseek(fi1, (bno+from1)*PBSIZE, 0);
		if (read(fi1, buf1, PBSIZE) != PBSIZE) {
			printf("Bad read on block %d\n", bno+from1);
			continue;
		}
		lseek(fi2, (bno+from2)*PBSIZE, 0);
		if (read(fi2, buf2, PBSIZE) != PBSIZE) {
			printf("Bad read on block %d\n", bno+from2);
			continue;
		}
		for (i = 0; i < PBSIZE; i++) {
			if ((buf1[i] & 0xFF) != (buf2[i] & 0xFF)) {
				printf("Block %d != block %d\n",
					bno+from1, bno+from2);
				break;
			}
		}
	}
	goto loop;
}
