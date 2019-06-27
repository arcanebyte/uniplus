/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 * memtest
 *	standalone static memory test program
 */
#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
	register char *start, *end;
        int i;
        char buf[50];

	printf("Standalone memory test\n\n");
loop:
	printf("Starting location: ");
	gets(buf);
	if ((start = (char *) gethex(buf)) == (char *) -1) {
		printf("Illegal number\n\n");
		goto loop;
	}
	printf("Ending location:   ");
	gets(buf);
	if ((end = (char *) gethex(buf)) == (char *) -1) {
		printf("Illegal number\n\n");
		goto loop;
	}
	if (end <= start) {
		printf("end <= start\n");
		goto loop;
	}
	test1(0x00, start, end);
}

/*
 * test1(pat, start, end)
 *	test all words with a fixed pattern
 */
test1(pat, start, end)
register char pat;
char *start;
register char *end;
{
	register char *cp;
	register ct;
	int nerror;
	int anydots;
	char *addr1, *addr2;

	ct = 1;
	addr1 = addr2 = 0;
	nerror = 0;
	anydots = 0;
	for (cp = start; cp < end; cp++)	/* load pattern */
		*cp = pat;
	for (;;) {
		for (cp = start; cp < end; cp++) {	/* check pattern */
			if (*cp != pat) {
				if (addr1 == 0)
					addr1 = cp;
				addr2 = cp;
				nerror++;
				if (anydots)
					printf("\n");
				printf("Pass %d: 0x%x should be 0x%x is 0x%x\n",
					ct, cp, pat & 0377, *cp & 0377);
				*cp = pat;
				anydots = 0;
			}
		}
		if (nerror) {
			printf("%d errors from 0x%x to 0x%x\n",
				nerror, addr1, addr2);
			nerror = 0;
			addr1 = addr2 = 0;
		}
		if ((++ct % 100) == 0) {
			printf(".");
			anydots = 1;
		}
	}
}
