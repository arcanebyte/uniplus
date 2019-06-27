/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
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

#define MEMADDR ((char *)0x20000)

char file[50];
char dffrom[] = "sn(0,0)";

/*
 *	Copy the serialized boot program to a specified device.
 */
copysboot()
{
	register fp;
	register char *cp, *p;
	register bn, i;
	register struct block0 *bp;

from:
	do {
		printf("Enter device to copy from: "); gets(file);
		p = file;
		if (file[0] == 0) {
			printf("%s\n", dffrom);
			fp = open(dffrom, 2);
			p = dffrom;
		} else
			fp = open(file, 2);
	} while (fp < 0);
	cp = MEMADDR;
	for (bn=1; bn<=NBLKS; bn++) {	/* read boot program from disk */
		lseek(fp, (off_t)(bn*BSIZE), 0);
		if ((read(fp,cp,BSIZE)) != BSIZE) {
			printf("READ ERROR\n");
			while (1);
		}
		cp += BSIZE;
	}
	close(fp);
	for (cp=p ; *p && *p != '('; p++) ;
	*p = '\0';
	if (match(cp, "sn"))
		if (eject(0))
			printf("copy: eject failed\n");
to:
	do {
		printf("Enter device to copy to: "); gets(file);
		if (file[0] == 0)
			goto to;
		for (p = file; *p && *p != '('; p++) ;
		if (*p == 0)	goto to;
		*p = '\0';
		if (match(p, "sn"))
			for (i=0; i<0xC0000; i++)	;
		*p = '(';
		for (p++; *p && *p != ','; p++) ;
		if (*p == 0)	goto to;
		for (p++; *p && *p != ')'; p++)
			*p = '0';	/* force block offset to 0 */
		if (*p == 0)	goto to;
		fp = open(file, 2);
	} while (fp < 0);
	/* write special block 0 */
	for (p = file; *p && *p != '('; p++) ;
	*p = '\0';
	for (bp = block0; bp->dv_name; bp++) {
		if (match(file, bp->dv_name))
			goto gotit;
	}
	printf("%s invalid\nValid devices to copy to are\n", file);
	for (bp = block0; bp->dv_name; bp++)
		printf("  %s\n", bp->dv_name);
	close(fp);
	goto to;
gotit:
	p = (char *)(bp->bblk);
	if (match(bp->dv_name, "sn")) {	/* flag to write boot header for bn 0 */
		iob[fp-3].i_bblk = BOOTDISK;
		lseek(fp, (off_t)0, 0);
		if (write(fp,p,BSIZE) != BSIZE) {	/* write sony twice */
			printf("WRITE ERROR\n");
			while (1);
		}
	}
	lseek(fp, (off_t)0, 0);
	if (write(fp,p,BSIZE) != BSIZE) {
		printf("WRITE ERROR\n");
		while (1);
	}
	cp = MEMADDR;
	for (bn=1; bn<=NBLKS; bn++) {
		lseek(fp, (off_t)(bn*BSIZE), 0);
		if (write(fp,cp,BSIZE) != BSIZE) {
			printf("WRITE ERROR\n");
			while (1);
		}
		cp += BSIZE;
	}
	iob[fp-3].i_bblk = 0;
	close(fp);
}

static
match(s1,s2)
register char *s1,*s2;
{
	for (;;) {
		if (*s1 != *s2)
			return(0);
		if (*s1++ && *s2++)
			continue;
		else
			return(1);
	}
}
