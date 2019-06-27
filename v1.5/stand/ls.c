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
#include	<sys/dir.h>

char line[100];

#ifdef	DEBUG
int	debug = 0;
#endif

main()
{
        int i;

# ifdef DEBUG
	debug = 0;
	if (debug & D_INOUT)
		printf("Entering main\n");
# endif
	printf("Standalone ls\n");
loop:
	printf("\n");
        do  {
                printf(": "); gets(line);
                i = open(line, 0);
        } while (i < 0);

        ls(i);
	close(i);
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Returning to loop\n");
# endif
	goto loop;
}

ls(io)
register io;
{
        struct direct d;
        register i;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering ls\n");
# endif
        while (read(io, (char *)&d, sizeof d) == sizeof d) {
                if (d.d_ino == 0)
                        continue;
                printf("%d\t", d.d_ino);
                for (i=0; i<DIRSIZ; i++) {
                        if (d.d_name[i] == 0)
                                break;
                        printf("%c", d.d_name[i]);
                }
                printf("\n");
        }
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving ls\n");
# endif
}
