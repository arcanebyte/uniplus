/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#undef DEBUG
extern short maxrow;
extern short maxcol;
extern short adm_row, adm_col;
extern short row_ofs, col_ofs;
extern short winscrl;

#include "sys/types.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "saio.h"

unsigned long ivec[4];
char getsbuf[50];
char sernum[50];
char *file = "sn(0,0)";
#define MEMADDR ((char *)0x20000)
	/* This sets a variable `bsadr' to be the address of where in the boot
	 * programs initialized data space the serial number should be placed.
	 */
#include "snaddr"
extern char begin;		/* start of this program and the boot program */

main()
{
	register i;

		/* This sets up the terminal emulator to use only the
		 * window created by the boot ROM.
		 */
	maxrow = 13;
	maxcol = 54;
	row_ofs = 8;
	col_ofs =25;
	winscrl = 1;
	adm_row = adm_col = 0;

	printf("          UniSoft Corporation\n");
	printf("   LISA UniPlus+ Serialization Program\n\n");
start:
	printf("Do you want to make a boot diskette (serialize)?\n");
	printf("('y' or 'n') ");
retry:
	gets(getsbuf);
	switch (getsbuf[0]) {
	case 'N': case 'n':
		printf("Use a different diskette\n");
		if (eject(0))
			while(1);
		for (i=0xA0000; i>0; i--) ;
		rom_mon((long *)0,"RESTART",0);		/* no return */
	case 'C': case 'c':
		copyboot();
		goto start;
	case 'Y': case 'y':
		serial();
		rom_mon((long *)0,"RESTART",0);		/* no return */
	default:
		printf("'%c' is invalid - please enter 'y' or 'n' ",
			getsbuf[0]);
		goto retry;
	}
}

copyboot()
{
	register i, bn;
	int fp;
	register char *cp;

        if ((fp = open(file,2)) <= 0) {
		printf("Failed to open Sony\n");
		while(1);
	}
	bn = 0;
	cp = MEMADDR;
	for (i=0; i<(NBLKS*2)+1; i++) {	/* read both programs from disk */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if ((read(fp,cp,BSIZE)) != BSIZE) {
			printf("READ ERROR\n");
			while (1);
		}
		cp += BSIZE;
	}
again:
	iob[fp-3].i_bblk = 0;
	close(fp);
	if (eject(0))
		while(1);
	printf("Insert diskette, then type return. ");
	gets(getsbuf);
	for (i=0; i<0xC0000; i++)	;
        if ((fp = open(file,2)) <= 0) {
		printf("Failed to open Sony\n");
		while(1);
	}
	iob[fp-3].i_bblk = BOOTDISK;	/* flag to write boot header for bn 0 */
	bn = 0;
	cp = MEMADDR;
	lseek(fp, (off_t)0, 0);		/* ignores write without this extra 1 */
	if ((write(fp,cp,BSIZE)) != BSIZE) {
		printf("WRITE ERROR\n");
		while (1);
	}
	for (i=0; i<(NBLKS*2)+1; i++) {	/* write both programs to disk */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if (write(fp,cp,BSIZE) != BSIZE) {
			printf("WRITE ERROR\n");
			while (1);
		}
		cp += BSIZE;
	}
	printf("Do you want to continue? ('y' or 'n') ");
	gets(getsbuf);
	if (getsbuf[0] != 'n')
		goto again;
}

serial()
{
	register i, bn;
	int j, fp;
	register char *cp;

		/* get the serial number from low memory
		 * this is the word to pass along.
		 */
	mkser(sernum);		/* Load sernum with the encrypted serial # */
	mkcode(sernum, ivec);	/* convert it into the 3 longs to send */

	j = 0x40000;
	do {
		for (i=0; i<j; i++) ;		/* time delay */
		i += j;				/* which keeps getting longer */
		if (i > j) j = i;
		printf("\nTHE REGISTRATION NUMBERS ARE:\n\n");
		pdate(ivec[0]);
		pdate(ivec[1]);
		pdate(ivec[2]);
		printf("\n\n");
		printf("TO CONTINUE, ENTER THE AUTHORIZATION NUMBER: ");
		gets(getsbuf);
		ivec[3] = gdate(getsbuf);
	} while (chkser(sernum,ivec[3]));

	printf("\nThank You.\n\n");

        if ((fp = open(file,2)) <= 0) {
		printf("Failed to open Sony\n");
		while(1);
	}
	bn = SBOOT;
	cp = MEMADDR;
	for (i=0; i<NBLKS; i++) {	/* read boot program from disk */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if ((read(fp,cp,BSIZE)) != BSIZE) {
			printf("READ ERROR\n");
			while (1);
		}
		cp += BSIZE;
	}
	bsadr = bsadr - (char *)(&begin) + MEMADDR;
	cp = sernum;
	for (i=0; i<13; i++)
		*bsadr++ = *cp++;
	bn = 1;
	cp = MEMADDR;
	for (i=0; i<NBLKS; i++) {	/* write bootable boot to disk */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if (write(fp,cp,BSIZE) != BSIZE) {
			printf("WRITE ERROR\n");
			while (1);
		}
		cp += BSIZE;
	}
	cp = MEMADDR;
	for (i=0; i<512; i++)
		*cp++ = 0;
	bn = SBOOT;
	cp = MEMADDR;
	for (i=0; i<NBLKS; i++) {	/* overwrite boot program */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if (write(fp,cp,BSIZE) != BSIZE) {
			printf("WRITE ERROR\n");
			while (1);
		}
	}
}
