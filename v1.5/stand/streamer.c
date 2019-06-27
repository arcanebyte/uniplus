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
#include "streamer.h"
#include "sys/priam.h"

extern struct results reslt;
char getsbuf[50];
char filename[50];
char pbuf[512];		/* buffer for tape use */
char *opbuf[] = {
	"rewind",
	"erase",
	"read file mark", 
	"write file mark",
	"advance file mark[s]",
	"retension tape",
	""
};
int errno;

struct tphdr {
	char date[32];
	short tapnum;
	int  blkno;
};

#define MEMADDR ((char *)0x20000)

main()
{
		/* This sets up the terminal emulator to use only the
		 * window created by the boot ROM.
		 */
	maxrow = 13;
	maxcol = 54;
	row_ofs = 8;
	col_ofs =25;
	winscrl = 1;
	adm_row = adm_col = 0;

	printf("     Streaming Tape Utility\n\n");
start:
	printf("Type 'b' to backup disk to tape,\n");
	printf("     'r' to restore disk from tape,\n");
/*	printf("     'f' to format (Sunol only), or\n");*/
	printf("     'c' to copy this diskette\n");
	printf("     't' to get other tape operations\n");
	printf("     'e' to eject the diskette\n");
	/*printf("     'p' to power down the LISA\n");*/
retry:
	printf(": ");
	gets(getsbuf);
	switch (getsbuf[0]) {
	case 'B': case 'b':
		backup();
		goto start;
	case 'R': case 'r':
		restore();
		goto start;
	case 'F': case 'f':
		format();
		goto start;
	case 'C': case 'c':
		copyboot();
		goto start;
	case 'T': case 't':
		tapeop();
		goto start;
	case 'E': case 'e':
		if (eject(0))
			printf("Unable to eject the Sony diskette, check for a hardware problem\n");
		goto start;
	case 'P': case 'p':
		powerdown();
		/* should never return */
	default:
		printf("'%c' is invalid - please try again\n",
			getsbuf[0]);
		goto retry;
	}
}

backup()
{
	int tfp, dfp, scr, comm, *iptr;
	register char *p, *p1;
	register struct streamer *bp;
	register struct devsw *dp;
	register dbn, dblks, tblks, nblks;
	register struct results *rp = &reslt;
	struct tphdr *sptr = (struct tphdr *)pbuf;
	short tapenr = 1;
	char junk[10];

tfrom:
	do {
		printf("Enter tape device to backup to: "); gets(filename);
		if (filename[0] == 0)
			goto tfrom;
		tfp = open(filename, 2);
	} while (tfp <= 0);
	for (p = filename; *p && *p != '('; p++) ;
	*p = '\0';
	for (bp = streamer; bp->dv_name; bp++) {	/* find streamer info */
		if (match(filename, bp->dv_name))
			goto dfrom;
	}
	printf("%s invalid\nValid tape devices are\n", filename);
	for (bp = streamer; bp->dv_name; bp++)
		printf("  %s\n", bp->dv_name);
	close(tfp);
	goto tfrom;
dfrom:
	do {
		printf("Enter device to backup from: "); gets(filename);
		if (filename[0] == 0)
			goto dfrom;
		dfp = open(filename, 2);
	} while (dfp <= 0);
	for (p = filename; *p && *p != '('; p++) ;
	*p = '\0';
	for (dp = devsw; dp->dv_name; dp++) {	/* find disk info */
		if (match(filename, dp->dv_name))
			goto gotit;
	}
	printf("%s invalid\nValid disk devices are\n", filename);
	for (bp = streamer; bp->dv_name; bp++)
		printf("  %s\n", bp->dv_name);
	close(dfp);
	goto dfrom;
gotit:
	p1 = p;
	p1++;
	for(;*p1 != ',';p1++)	;
	p1++;
	for(p = junk;*p1 != ')';*p++ = *p1++)	;
	*p = '\0';
	dbn = atoi(junk);
	if(dbn < 101)
		dbn = 101;		/* skip over boot blocks */
retry:
	printf("mount tape and hit return ");
	gets(getsbuf);
	if ((*bp->rewind)(tfp)) {	 /* rewind the tape */
		printf("Error rewinding the tape, do you want to try again ?\n");
		printf("Enter yes or no ('y' or 'n'): ");
		gets(getsbuf);
		if(getsbuf[0] != 'n' || getsbuf[0] != 'N')
		    goto retry;
		else {
			printf("Aborting backup\n");
			goto out;
		}
	}
	printf("Please enter today's date and hit return: ");
	gets(pbuf);
	sptr->tapnum = tapenr;
	sptr->blkno = dbn;
	if((scr = write(tfp, pbuf, PBSIZE)) != PBSIZE) {
		printf("ERROR writing header block, aborting\n");
		goto out;
	}
	comm = ISSUE;
	while (1) {
		rp = (struct results *)	(*bp->totape)(tfp, dfp, dbn, comm, dblks);
#define RESUMABLE 0x03
#define ALLDONE   0x0D
		if (rp->errcode < 0) {
			printf("ERROR encountered, aborting\n");
			abortpkt();
			goto out;
		}
		if ((*bp->weof)(tfp)) {		/* write end-of-file mark */
			printf("Error in writing end of file on tape, aborting\n");
			abortpkt();
			goto out;
		}
		if ((*bp->rewind)(tfp)) {		/* rewind the tape */
			printf("Error in rewinding tape, aborting\n");
			abortpkt();
			goto out;
		}
		if(rp->errcode == RESUMABLE) {
			sptr->tapnum = ++tapenr;
retry1:
			printf("Change tapes and hit return ");
			gets(getsbuf);
			if ((*bp->rewind)(tfp)) {	 /* rewind the tape */
				printf("Error rewinding the tape, do you want to try again ?\n");
				printf("Enter yes or no ('y' or 'n'): ");
				gets(getsbuf);
				if(getsbuf[0] != 'n' || getsbuf[0] != 'N')
				    goto retry1;
				else {
					printf("Aborting backup\n");
					abortpkt();
					goto out;
				}
			}
			if((scr = write(tfp, pbuf, PBSIZE)) != PBSIZE) {
				printf("ERROR writing header block, aborting\n");
				abortpkt();
				goto out;
			}
		comm = PMPKTRES;
		} else if (rp->errcode == ALLDONE){
			printf("Backup complete\n");
			goto out;
		} else  {
		       printf("THIS SHOULD NEVER HAPPEN IN BACKUP-UNKNOWN CONDITION\n");
		       goto out;
		}
	}
out:
	close(dfp);
	close(tfp);
}

restore()
{
	int tfp, dfp, scr, comm;
	register char *p, *p1;
	register struct streamer *bp;
	register struct devsw *dp;
	register dblks, tblks, nblks;
	register struct results *rp = &reslt;
	struct tphdr *s1, *s2;
	int leng, dbn;
	short tapenr = 1;
	char junk[50];

tfrom:
	do {
		printf("Enter tape device to restore from: "); gets(filename);
		if (filename[0] == 0)
			goto tfrom;
		tfp = open(filename, 2);
	} while (tfp <= 0);
	for (p = filename; *p && *p != '('; p++) ;
	*p = '\0';
	for (bp = streamer; bp->dv_name; bp++) {	/* find streamer info */
		if (match(filename, bp->dv_name))
			goto dfrom;
	}
	printf("%s invalid\nValid tape devices are\n", filename);
	for (bp = streamer; bp->dv_name; bp++)
		printf("  %s\n", bp->dv_name);
	close(tfp);
	goto tfrom;
dfrom:
	do {
		printf("Enter device to restore to: "); gets(filename);
		if (filename[0] == 0)
			goto dfrom;
		dfp = open(filename, 2);
	} while (dfp <= 0);
	for (p = filename; *p && *p != '('; p++) ;
	*p = '\0';
	for (dp = devsw; dp->dv_name; dp++) {	/* find disk info */
		if (match(filename, dp->dv_name))
			goto gotit;
	}
	printf("%s invalid\nValid disk devices are\n", filename);
	for (bp = streamer; bp->dv_name; bp++)
		printf("  %s\n", bp->dv_name);
	close(dfp);
	goto dfrom;
gotit:
again:
retry:
	printf("mount tape and hit return ");
	gets(getsbuf);
	if ((*bp->rewind)(tfp)) {	 /* rewind the tape */
		printf("Error rewinding the tape, do you want to try again ?\n");
		printf("Enter yes or no ('y' or 'n'): ");
		gets(getsbuf);
		if(getsbuf[0] != 'n' || getsbuf[0] != 'N')
		    goto retry;
		else {
			printf("Aborting restore\n");
			goto out;
		}
	}
	if((scr = read(tfp, pbuf, PBSIZE)) != PBSIZE) {
		printf("ERROR reading header block, aborting\n");
		goto out;
	}
	for(leng = 0;leng < 50; leng++)
		junk[leng] = pbuf[leng];
	s1 = (struct tphdr *)junk;
	s2 = (struct tphdr *)pbuf;
	if(s1->tapnum != tapenr) {
		printf("Tape number out of sequence, tape %d mounted\n",s1->tapnum);
		printf("Please mount the correct tape\n");
		goto again;
	}
	dbn = s1->blkno;
	comm = ISSUE;
	while (1) {
		rp = (struct results *)	(*bp->frtape)(tfp, dfp, dbn, comm, dblks);
#define RESUMABLE 0x03
#define ALLDONE   0x0D
		if (rp->errcode < 0) {
			printf("ERROR encountered, aborting\n");
			abortpkt();
			goto out;
		}
		if ((*bp->rewind)(tfp)) {		/* rewind the tape */
			printf("Error in rewinding tape, aborting\n");
			abortpkt();
			goto out;
		}
		if(rp->errcode == RESUMABLE) {
			s1->tapnum = ++tapenr;
yetagain:
			printf("change tapes and hit return ");
			gets(getsbuf);
			if ((*bp->rewind)(tfp)) {	 /* rewind the tape */
				printf("Error rewinding the tape, do you want to try again ?\n");
				printf("Enter yes or no ('y' or 'n'): ");
				gets(getsbuf);
				if(getsbuf[0] != 'n' || getsbuf[0] != 'N')
				    goto yetagain;
				else {
					printf("Aborting restore\n");
					abortpkt();
					goto out;
				}
			}
			(*bp->rewind)(tfp);		/* rewind the tape */
			if((scr = read(tfp, pbuf, PBSIZE)) != PBSIZE) {
				printf("ERROR reading header block, aborting\n");
				abortpkt();
				goto out;
			}
			if((s1->tapnum != s2->tapnum) && !match(pbuf, junk)) {
			        printf("Tape out of sequence, ");
				goto yetagain;
			}
		comm = PMPKTRES;
		} else if (rp->errcode == ALLDONE){
			printf("Restore complete\n");
			goto out;
		} else  {
		       printf("THIS SHOULD NEVER HAPPEN IN RESTORE-UNKNOWN CONDITION\n");
		       abortpkt();
		       goto out;
		}
	}
out:
	close(dfp);
	close(tfp);
}


format()
{
	printf("formatting not implemented yet\n");
}

copyboot()
{
	register i, bn;
	int fp;
	register char *cp;
	char *file = "sn(0,0)";

        if ((fp = open(file,2)) <= 0) {
		printf("Failed to open Sony\n");
		return;
	}
	printf("Reading diskette into memory\n");
	bn = 0;
	cp = MEMADDR;
	for (i=0; i<NBLKS+1; i++) {	/* read this program from disk */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if ((read(fp,cp,BSIZE)) != BSIZE) {
			printf("READ ERROR\n");
			return;
		}
		cp += BSIZE;
	}
again:
	iob[fp-3].i_bblk = 0;
	close(fp);
	if (eject(0)) {
		printf("Unable to eject the Sony diskette, check for a hardware problem\n");
		return;
	}
	printf("Insert diskette, then type return. ");
	gets(getsbuf);
	for (i=0; i<0xC0000; i++)	;
        if ((fp = open(file,2)) <= 0) {
		printf("Failed to open Sony\n");
		return;
	}
	iob[fp-3].i_bblk = BOOTDISK;	/* flag to write boot header for bn 0 */
	bn = 0;
	cp = MEMADDR;
	lseek(fp, (off_t)0, 0);		/* ignores write without this extra 1 */
	if ((write(fp,cp,BSIZE)) != BSIZE) {
		printf("WRITE ERROR\n");
		return;
	}
	for (i=0; i<NBLKS+1; i++) {	/* write both programs to disk */
		lseek(fp, (off_t)(bn++ *BSIZE), 0);
		if (write(fp,cp,BSIZE) != BSIZE) {
			printf("WRITE ERROR\n");
			return;
		}
		cp += BSIZE;
	}
	printf("Do you want to continue copying? ('y' or 'n') ");
	gets(getsbuf);
	if (getsbuf[0] != 'n')
		goto again;
	iob[fp-3].i_bblk = 0;
	close(fp);
}

tapeop()
{
	int tfp, markno; register int i;
	register char *p;
	register struct streamer *bp;
	char skipto[5];

tfrom:
	do {
		printf("Enter tape device: "); gets(filename);
		if (filename[0] == 0)
			goto tfrom;
		tfp = open(filename, 2);
	} while (tfp < 0);
	for (p = filename; *p && *p != '('; p++) ;
	*p = '\0';
	for (bp = streamer; bp->dv_name; bp++) {	/* find streamer info */
		if (match(filename, bp->dv_name))
			goto top;
	}
	printf("%s invalid\nValid tape devices are\n", filename);
	for (bp = streamer; bp->dv_name; bp++)
		printf("  %s\n", bp->dv_name);
	close(tfp);
	goto tfrom;
top:
	printf("Valid tape operations are\n");
	i = 0;
	while (*(p = opbuf[i++]) != NULL)
	    printf("\t%s\n", p);
	do {
	    printf("Enter tape operation (first word only): "); gets(getsbuf);
	    if (getsbuf[0] == 0)
		goto top;
	} while (getsbuf[0] == 0);
	for (p = getsbuf; *p && (*p != ' ' || *p != '\n'); p++) ;
	p = '\0';
gotit:
	if (!strcmp(getsbuf, "rewind"))		/* Look for requested op */
	    (*bp->rewind)(tfp);
	else if (!strcmp(getsbuf, "write"))
	    (*bp->weof)(tfp);
	else if (!strcmp(getsbuf, "read"))
	    (*bp->reof)(tfp);
	else if (!strcmp(getsbuf, "erase"))
	    (*bp->erase)(tfp);
	else if (!strcmp(getsbuf, "advance")) {
	    printf("enter mark to skip to: ");  gets(skipto);
	    markno = atol(skipto);
	    (*bp->advance)(tfp, skipto);
	} else if (!strcmp(getsbuf, "retension"))
	    (*bp->reten)(tfp);
	else {
	    printf("Invalid tape operation.\n");
	    goto top;
	}
	close(tfp);
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

powerdown() {}
