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
        int c, i;
        char buf[50];

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering main\n");
# endif
	printf("Standalone cat\n\n");
loop:
        do {
                printf("File: ");
                gets(buf);
                i = open(buf, 0);
        } while (i <= 0);

# ifdef DEBUG
	printf("File descriptor = %d\n", i);
# endif
	printf("\n");
        while ((c = getc(i)) > 0)
                putchar(c);
	(void)close(i);
	printf("\n");
	goto loop;
}
