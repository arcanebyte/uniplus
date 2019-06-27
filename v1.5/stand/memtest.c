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
 *	standalone memory test program
 */
#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
	extern char begin[], end[];
	register char *start, *last;
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
	if ((last = (char *) gethex(buf)) == (char *) -1) {
		printf("Illegal number\n\n");
		goto loop;
	}
	if (last <= start) {
		printf("last <= start\n");
		goto loop;
	}
	if (last >= begin && start <= end+0x8000) {
		printf("that would clobber you (%x-%x)\n", begin, end+0x8000);
		goto loop;
	}
	for (i = 0; i < 10; i++) {
		printf("00 ");
		test1(0x00, start, last);
		printf("55 ");
		test1(0x55, start, last);
		printf("aa ");
		test1(0xaa, start, last);
		printf("ff ");
		test1(0xff, start, last);
		printf("checker ");
		checker(start, last);
		printf("interact5 ");
		interact(5, start, last);
		if (i != 0) {
			printf("interact11 ");
			interact(11, start, last);
		}
		run(start, last);
		printf("\007End of pass %d\n", i+1);
	}
	printf("Done\n\n");
	goto loop;
}

/*
 * test1(pat, start, last)
 *	test all words with a fixed pattern
 */
test1(pat, start, last)
register char pat;
char *start;
register char *last;
{
	register char *cp;

	for (cp = start; cp < last; cp++)	/* load pattern */
		*cp = pat;
	for (cp = start; cp < last; cp++) {	/* check pattern */
		if (*cp != pat)
			printf("0x%x  should be 0x%x  is 0x%x\n",
				cp, pat & 0377, *cp & 0377);
	}
}

/*
 * checker(start, last)
 *	test with a checkerboard pattern
 */
checker(start, last)
register char *start, *last;
{
	register char *cp;

	for (cp = start; cp < last; ) {	/* load pattern */
		*cp++ = 0x55;
		*cp++ = 0xaa;
	}
	for (cp = start; cp < last; ) {	/* check pattern */
		if (*cp++ != 0x55)
			printf("0x%x  should be 0x%x  is 0x%x\n",
				cp-1, 0x55, *(cp-1) & 0377);
		if (*cp++ != 0xaa)
			printf("0x%x  should be 0x%x  is 0x%x\n",
				cp-1, 0xaa, *(cp-1) & 0377);
	}
	for (cp = start; cp < last; ) {	/* load complement pattern */
		*cp++ = 0xaa;
		*cp++ = 0x55;
	}
	for (cp = start; cp < last; ) {	/* check pattern */
		if (*cp++ != 0xaa)
			printf("0x%x  should be 0x%x  is 0x%x\n",
				cp-1, 0xaa, *(cp-1) & 0377);
		if (*cp++ != 0x55)
			printf("0x%x  should be 0x%x  is 0x%x\n",
				cp-1, 0x55, *(cp-1) & 0377);
	}
}

/*
 * interact(mod, start, last)
 *	test interaction between words
 */
interact(mod, start, last)
register int mod;
register char *start, *last;
{
	register char *cp;
	register int i;
	int is;

	for (is = 0; is < mod; is++) {
		for (cp = start; cp < last; )
			*cp++ = 0;
		for (cp = &start[is]; cp < last; cp += mod)
			*cp = 0xff;
		i = 0;
		for (cp = start; cp < last; cp++)
			if ((i++ % mod) != is) {
				if (*cp != 0)
				printf("0x%x  should be 0x%x  is 0x%x\n",
					cp, 0, *cp & 0377);
			}
			else {
				if (*cp != 0xff)
				printf("0x%x  should be 0x%x  is 0x%x\n",
					cp, 0xff, *cp & 0377);
			}
		for (cp = start; cp < last; )
			*cp++ = 0xff;
		for (cp = &start[is]; cp < last; cp += mod)
			*cp = 0;
		i = 0;
		for (cp = start; cp < last; cp++)
			if ((i++ % mod) != is) {
				if (*cp != 0xff)
				printf("0x%x  should be 0x%x  is 0x%x\n",
					cp, 0xff, *cp & 0377);
			}
			else {
				if (*cp != 0)
				printf("0x%x  should be 0x%x  is 0x%x\n",
					cp, 0, *cp & 0377);
			}
	}
}

#define NOP	0x4E71
#define RTS	0x4E75

run(start, last)
register short *start, *last;
{
	register short *p;

	for (p = start; p < last; p++)
		*p = NOP;
	*(last-1) = RTS;
	printf("starting run");
	((int (*)())(char *)start)();
	printf(" -- done\n");
}
