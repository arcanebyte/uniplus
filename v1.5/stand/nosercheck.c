typedef int (*pfi)();
char snum[] = "0123456789012";
sercheck(bp)
	char *bp;
{
	register char *cp;
	register int i;
	pfi rom = (pfi)0;

#ifndef NOSERCHECK
	mkser(bp);
	cp = snum;		/* (char *)0x47E00; */
	for (i=0; i<5; i++)
		if (cp[i] != bp[i]) {
			printf("Sorry, but this is not the right computer.\n");
			for (i=0xA0000; i>0; i--) ;
			rom();
		}

#else NOSERCHECK
	cp = snum;
#endif NOSERCHECK
	for (i=0; i<13; i++) *cp++ = 0;
}
