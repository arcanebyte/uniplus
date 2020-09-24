#include "xmodem.h"

/*  Print Help Message  */
help()
	{
	printf("\nUsage:  \n\txmodem ");
	printf("-[rb!rt!sb!st][options] filename\n");
	printf("\nMajor Commands --");
	printf("\n\trb <-- Receive Binary");
	printf("\n\trt <-- Receive Text");
	printf("\n\tsb <-- Send Binary");
	printf("\n\tst <-- Send Text");
	printf("\nOptions --");
	printf("\n\ty  <-- Use YMODEM Batch Mode on transmit");
	printf("\n\tm  <-- Use MODEM7 Batch Mode on transmit");
	printf("\n\tk  <-- Use 1K packets on transmit");
	printf("\n\tc  <-- Select CRC mode on receive");
	printf("\n\tt  <-- Indicate a TOO BUSY Unix system");
	printf("\n\td  <-- Delete xmodem.log file before starting");
	printf("\n\tl  <-- (ell) Turn OFF Log File Entries");
	printf("\n\tx  <-- Include copious debugging information in log file");
	printf("\n");
	}

/* get type of transmission requested (text or binary) */
gettype(ichar)
char ichar;
	{
	if (ichar == 't' || ichar == 'T') return('t');
	if (ichar == 'b' || ichar == 'B') return('b');
	error("Invalid Send/Receive Parameter - not t or b", FALSE);
	return(0);
	}

/* print error message and exit; if mode == TRUE, restore normal tty modes */
error(msg, mode)
char *msg;
int mode;
	{
	if (mode)
		restoremodes(TRUE);  /* put back normal tty modes */
	printf("\r\n%s\n", msg);
	if ((LOGFLAG || DEBUG) && (LOGFP != NULL))
		{   
		fprintf(LOGFP, "XMODEM Fatal Error:  %s\n", msg);
	    	fclose(LOGFP);
		}
	exit(-1);
	}


/* Construct a proper (i.e. pretty) sector count for messages */

char
*sectdisp(recvsectcnt, bufsize, plus1)
long recvsectcnt;
int bufsize, plus1;
	{
	static char string[20];
	if (plus1)
		recvsectcnt += (bufsize == 128) ? 1 : 8;
	if (bufsize == 128 || recvsectcnt == 0)
		sprintf (string, "%d", recvsectcnt);
	else
		sprintf (string, "%d-%d", recvsectcnt-7, recvsectcnt);
	return(string);
	}

/* type out debugging info */
xmdebug(str)
char *str;
	{
	if (DEBUG && (LOGFP != NULL))
		fprintf(LOGFP,"DEBUG: '%s'\n",str);
	}

/* print elapsed time and rate of transfer in logfile */

int quant[] = { 60, 60, 24};	
char sep[3][10] = { "second", "minute", "hour" };

prtime (numsect, seconds)
long numsect;
time_t seconds;

{
	register int i;
	register int Seconds;
	int nums[3];
	int rate;

	if (!LOGFLAG || numsect == 0)
		return(0);

	Seconds = (int)seconds;
	Seconds = (Seconds > 0) ? Seconds : 0;

	rate = (Seconds != 0) ? 128 * numsect/Seconds : 0;

	for (i=0; i<3; i++) {
		nums[i] = (Seconds % quant[i]);
		Seconds /= quant[i];
	}

	fprintf (LOGFP, "%ld Sectors Transfered in ", numsect);

	if (rate == 0)
		fprintf (LOGFP, "0 seconds");
	else
		while (--i >= 0)
			if (nums[i])
				fprintf (LOGFP, "%d %s%c ", nums[i], &sep[i][0],
					nums[i] == 1 ? ' ' : 's');
	fprintf (LOGFP, "\n");

	if (rate != 0)
		fprintf (LOGFP, "Transfer Rate = %d Characters per Second\n", rate);

	return(0);
}

/* Print elapsed time estimate */

projtime (numsect, fd)
long numsect;
FILE *fd;
	{
	register int i;
	register int seconds;
	int nums[3];

	if (numsect == 0)
		return (0);

/* constant below should really be 1280; reduced to 90% to account for time lost in overhead */

	seconds = 1422 * numsect / ttyspeed + 1;

	for (i=0; i<3; i++) {
		nums[i] = (seconds % quant[i]);
		seconds /= quant[i];
	}

	fprintf (fd, "Estimated transmission time ");

	while (--i >= 0)
		if (nums[i])
			fprintf (fd, "%d %s%c ", nums[i], &sep[i][0],
				nums[i] == 1 ? ' ' : 's');
	fprintf (fd, "\n");
	return (0);
	}
