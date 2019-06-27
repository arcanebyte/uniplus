/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include "sys/param.h"

main()
{
	int fi;
        char buf[50];

loop:
	printf("\nStandalone disk formatter\n\n");
	finit();
        do  {
                printf("device: "); gets(buf);
                fi = open(buf, 2);
        } while (fi < 0);
	printf("Type return to format:");
	gets(buf);
	write(fi, 0, BSIZE);
	goto loop;
}
