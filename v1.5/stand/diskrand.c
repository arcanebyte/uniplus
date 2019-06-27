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

#define MAXBLKS 5000

#ifdef	DEBUG
int	debug = 0;
#endif
int rndflg ;
int blks[MAXBLKS] ;
int errcnt ;

main()
{
        register int start, end, inc;
	int fi;
        char buf[50];
	int nblks ;

loop:
	printf("\nStandalone disk2 test\n\n");
	finit();
        do  {
                printf("Device: "); gets(buf);
                fi = open(buf, 2);
# ifdef DEBUG
		printf("fdesc=%d\n", fi);
# endif
        } while (fi < 0);
	printf("\nStarting disk sector: ");
	gets(buf);
	start = atoi(buf);
	printf("Ending disk sector:   ");
	gets(buf);
	end = atoi(buf);
	printf("Disk sector increment:    ");
	gets(buf);
	inc = atoi(buf);
	if (start < 0 || end < 0 || inc <= 0) {
		printf("Illegal number\n\n");
		goto loop;
	}
	printf("Test blocks in random sequence? ") ;
	gets(buf) ;
	if( buf[0] == 'y')
	{
		printf("Number of blocks to test (%d max): ", MAXBLKS) ;
		gets(buf) ;
		if( (nblks = atoi(buf)) > MAXBLKS)
			nblks = MAXBLKS;
		rndflg = 1 ;
	}
	else
	{
		nblks = end - start + 1 ;
		rndflg = 0 ;
	}
	dtest(fi, start, end, inc, 0x00, nblks);
	dtest(fi, start, end, inc, 0xFF, nblks);
	dtest(fi, start, end, inc, 0xAA, nblks);
	dtest(fi, start, end, inc, 0x55, nblks);
	dtest(fi, start, end, inc, -1, nblks);
	dtest(fi, start, end, inc, -2, nblks);
	printf("\nDone: %d errors\n", errcnt);
	goto loop;
}

char buf1[PBSIZE], buf2[PBSIZE];
/*
 * dtest - run a disk test from sector 'start' to sector 'end'
 *	   with sector increment 'inc' and byte pattern 'pat'
 */
dtest(fi, start, end, inc, pat, nblks)
{
	register int i, bno;
	int bcnt;

	printf("\n") ;
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
	for (bcnt = 0; bcnt < nblks; bcnt++) 
	{
		bno = nxtbno(start, end, bcnt) ;
		if( bcnt % 8 == 0)
			printf("\nw%d:	", bcnt) ;
		printf("%d	", bno) ;
		if( rndflg )				/* save sequence */
			blks[bcnt] = bno ;
		if (pat == -2) {
			for (i = 0; i < PBSIZE; i++)	/* load pattern */
				buf1[i] = bno;
		}
		lseek(fi, bno*PBSIZE, 0);
		if (write(fi, buf1, PBSIZE) != PBSIZE)
			printf("Bad write on block %d\n", bno);
	}
	printf("\nChecking writes\n") ;
	for( bcnt = 0; bcnt < nblks; bcnt++)
	{
		for (i = 0; i < PBSIZE; i++)	/* clear pattern */
			buf2[i] = 0xFE;
		if( rndflg )
			bno = blks[bcnt] ;
		else
			bno = nxtbno( start, end, bcnt) ;
		if( bcnt % 8 == 0)
			printf("\nr%d:	", bcnt) ;
		printf("%d	", bno) ;
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
				errcnt++ ;
				break;
			}
	}
}

nxtbno(start, end, bcnt)
int start ;
int end ;
int bcnt ;
{
	if( rndflg )
		return( getrand(start, end)) ;
	else
		return( start + (bcnt % (end - start + 1) )) ;
}

getrand(lower, upper)
int lower, upper ;
{
	static rndinit = 0 ;

	if(!rndinit)
	{
		srand(1) ;
		rndinit = 1 ;
	}
	
/*
 * fake high order bit with additional call to rand - (since
 * rand returns [0-32767)
 */
	return( lower + (((rand() & 1 ? (1<<15) : 0) + rand()) % (upper - lower + 1))) ;
}
