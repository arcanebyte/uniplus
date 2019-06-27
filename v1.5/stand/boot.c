/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "saio.h"

char line[100];
char file[100];
caddr_t start;
char dfboot[] = "sn(0,201)unix";
char dffs[] = "sn(0,201)";

extern short maxrow;
extern short maxcol;
extern short adm_row, adm_col;
extern short row_ofs, col_ofs;
extern short winscrl;

#define BOOTDEV  ((char *)(0x1B3))
char bdev;

main()
{
	register fd;
	register char *p;
	register i;

	bdev = *BOOTDEV;
	maxrow = 13;
	maxcol = 54;
	row_ofs = 8;
	col_ofs =25;
	winscrl = 1;
	adm_row = adm_col = 0;
	sercheck(line);			/* verify serial number */

	printf("\fStandalone boot\n\n");
loop:
	do {
		printf(": "); gets(line);
		if (strncmp(line, "eject", 5) == 0) {
			if (eject(0))
				printf("boot: diskette eject failed\n");
			goto loop;
		}
		if (strncmp(line, "copy", 4) == 0) {
			copysboot();
			printf("boot: copy done\nEnter file to boot from");
			goto loop;
		}
		file[0] = 0;
		if (line[0] == 0) {
			append(file, dfboot);
		} else {
			for (p = line; *p && *p != '('; p++) ;
			if (*p == 0) {
				append(file, dffs);
			}
			append(file, line);
		}
		fd = open(file, 0);
	} while (fd < 0);
	printf("%s\n", file);
	if (copyunix(fd) == 0)
		goto loop;
	*BOOTDEV = mkbdev(&iob[fd-3]);	/* reset boot device flag */
	if (strcmp(file, "sn(0,201)sunix") == 0) {
		close(fd);
		if (eject(0))
			printf("boot: diskette eject failed\n");
	retry:	printf("Insert the Sony root filesystem\n");
		printf("Type RETURN to start\n");
		gets(line);
		for (i=0; i<0xC0000; i++)	;
		if ((fd = open("sn(0,0)", 0)) < 0)
			goto retry;
	}
	((int (*)())start)();
}

append(base, add)
char *base, *add;
{
	while (*base)
		base++;
	do {
		*base++ = *add;
	} while (*add++);
}

copyunix(io)
register io;
{
	register caddr_t addr;
	register count;
	long txtsiz, datsiz, bsssiz;
	long magic;

	(void)lseek(io, (off_t)0, 0);
	magic = getw(io);
	txtsiz = getw(io);
	datsiz = getw(io);
	bsssiz = getw(io);

	switch (magic) {
	case 0407:
		/*
		 * space over the header. We do this instead of seeking
		 * because the input might be a tape which doesn't know 
		 * how to seek.
		 */
		(void)getw(io);
		(void)getw(io);
		(void)getw(io);
		start = addr = (caddr_t)getw(io);	/* starting address */
		printf("Loading at 0x%x:", addr);
		printf(" %d", txtsiz);
		for (count = 0; count < txtsiz; count++)
			*addr++ = getc(io);
		printf("+%d", datsiz);
		for (count = 0; count < datsiz; count++)
			*addr++ = getc(io);
		printf("+%d", bsssiz);
		clrseg(addr, bsssiz);
		printf("\n");
		return(1);
	default:
		printf("Can't load type %o files\n", magic);
		return(0);
	}
}

clrseg(addr, size)
register caddr_t addr;
register long size;
{
	while (size-- > 0)
		*addr++ = 0;
}

mkbdev(ip)
	register struct iob *ip;
{
	if (ip->i_device) {
		if (ip->i_unit >= 1) return 1;
		return 0;
	} else
		return 2 + ip->i_unit;
}
