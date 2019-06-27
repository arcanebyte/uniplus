/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
	register char *loc1, *loc2;
	register nbytes;
        char buf[50];

	printf("Standalone memory compare\n\n");
loop:
	printf("Starting location 1: ");
	gets(buf);
	if ((loc1 = (char *) gethex(buf)) == (char *) -1) {
		printf("Illegal number\n\n");
		goto loop;
	}
	printf("Starting location 2: ");
	gets(buf);
	if ((loc2 = (char *) gethex(buf)) == (char *) -1) {
		printf("Illegal number\n\n");
		goto loop;
	}
	printf("Number of bytes:     ");
	gets(buf);
	if ((nbytes = gethex(buf)) == -1) {
		printf("Illegal number\n\n");
		goto loop;
	}
	cmp(loc1, loc2, nbytes);
	printf("Done\n\n");
	goto loop;
}

cmp(loc1, loc2, nbytes)
register char *loc1, *loc2;
register nbytes;
{
	register int i;

	for (i = 0; i < nbytes; i++) {
		if (*loc1 != *loc2)
			printf("0x%x  0x%x  0x%x\n",
				loc1, *loc1 & 0377, *loc2 & 0377);
		loc1++;
		loc2++;
	}
}
