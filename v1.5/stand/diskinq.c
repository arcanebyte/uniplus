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
	int nsect ;
	int fi;
        char buf[50];
	char *bufp ;

loop:
	printf("\nStandalone interactive disk inquire\n\n");
	finit();
        do  {
                printf("\nDevice:   "); gets(buf);
                fi = open(buf, 0);
# ifdef DEBUG
		printf("fdesc=%d\n", fi);
# endif
        } while (fi < 0);
	printf("\nStarting disk sector: ");
	gets(buf);
	start = atoi(buf);
	printf("Number of sectors:    ");
	gets(buf);
	nsect = atoi(buf);
	printf("Buffer location:      ");
	gets(buf);
	bufp = (char *)(atoi(buf));
	dread(fi, start, nsect, bufp);
	printf("\nDone\n");
	goto loop;
}

/*
 * dread - read nsect sectors starting at "start" into bufp.
 */
dread(fi, start, nsect, bufp)
char *bufp;
{
	lseek(fi, start*256, 0);
	read(fi, bufp, nsect*256);
}
