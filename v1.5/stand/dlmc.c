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

#define OK	'0'
#define BAD	'1'

#define CHKSUM			/* to enable checksum verification */

#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
	register char *cp;
	register c1, c2;
        int val, first, chksum, bno, i, j, csum, psum;
        char buf[BSIZE];

	printf("Standalone disk load utility\n\n");
loop:
        do {
                printf("Device: ");
                gets(buf);
                i = open(buf, 1);
		if (i <= 0)
			printf("Can't open %s\n", buf);
        } while (i <= 0);

# ifdef DEBUG
	printf("File descriptor = %d\n", i);
# endif
	printf("\n");
	wputremote('\n');		/* start transmitting */
	bno = 0;
	for (;;) {
		first = 0;
	goahead:
		for (cp = &buf[0]; cp < &buf[BSIZE]; cp++) {
			c1 = wgetremote();
			if (c1 < 'a' || c1 > 'p')
				break;
			c2 = wgetremote();
			if (c2 < 'a' || c2 > 'p')
				break;
			val = (((c1 - 'a') << 4) | (c2 - 'a')) & 0xFF;
			*cp = val;
			if (!first++) {
				chksum = val;
				csum = 0;
				goto goahead;
			} else {
				psum = csum << 1;
				csum = (psum & 0xFF)+val+((psum >> 8) & 0xFF);
			}
		}
		csum &= 0xFF;
		if (c1 == '\n' || c2 == '\n') {	/* empty block signals end */
			if (cp != &buf[0]) {
				printf("short block -- trying to resync\n");
				goto retry;
			}
#ifdef DEBUG
			if (debug & D_RW)
				printf("End: c1=%o c2=%o\n", c1, c2);
#endif
			break;
		} else if (cp != &buf[BSIZE]) {
			if (cp == buf) {
				if (c1 == 'Z' && (wgetremote() == '\n')) {
					while (cp < &buf[BSIZE])
						*cp++ = '\0';
					printf("%d: ZERO\n", bno);
					goto zeroblock;
				}
			}
			printf("Got bogus char (x%x or x%x) @ %d\n", c1, c2, cp-buf);
		flush:
			while ((c1 = wgetremote()) != '\n')
				;
			goto retry;
		}
#ifdef DEBUG
		if (debug & D_RW)
			printf("Got block: count=%d\n", cp-buf);
#endif
		if ((c1 = wgetremote()) != '\n') {
			printf("Char(x%x) not newline\n",c1);
			goto retry;
#ifdef CHKSUM
		} else if (chksum != csum) {
			printf("Chksum x%x instead of x%x\n",csum,chksum);
		retry:
			wputremote(BAD);
#endif
		} else {
			printf("%d: ", bno);
			cp = &buf[0];
			traceit(cp);
			traceit(cp+2);
			traceit(cp+4);
			traceit(cp+6);
			printf("\n");
		zeroblock:
			lseek(i, bno*BSIZE, 0);
#ifdef DEBUG
			if (debug & D_RW)
				printf("Lseek done\n");
#endif
			if ((c1 = write(i, buf, BSIZE)) != BSIZE)
				printf("write error: wrote %d instead of %d\n",
					c1, bno);
#ifdef DEBUG
			if (debug & D_RW)
				printf("Write done\n");
#endif
for (j=0; j<10000; j++) ; /* give disk time */
			wputremote(OK);
			bno++;
		}
		wputremote('\n');
	}
	close(i);
	goto loop;
}

traceit(cp)
register char *cp;
{
	register int n;

	n = *cp++ & 0377;
	n = (n << 8) | (*cp++ & 0377);
	printf("%x ", n);
}
