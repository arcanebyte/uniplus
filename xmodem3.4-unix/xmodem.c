/*
 *  XMODEM -- Implements the Christensen XMODEM protocol, 
 *            for packetized file up/downloading.    
 *
 *	I have tried to keep the 4.2isms (select system call, 4.2BSD/v7 tty
 *	structures, gettimeofday system call, etc.) in the source file
 *	getput.c; but I make no guarantees.  Also, I have made no attempt to
 *	keep variable names under 7 characters (although a cursory check
 *	shows that all variables are unique within 7 first characters).
 *	See the README file for some notes on SYS V adaptations.
 *	The program has been successfully run on VAXes (4.3BSD) and SUN-3s
 *	(2.x/3.x) against MEX-PC and ZCOMM/DSZ.
 *
 *   -- Based on UMODEM 3.5 by Lauren Weinstein, Richard Conn, and others.
 *
 *  XMODEM Version 1.0  - by Brian Kantor, UCSD (3/84)
 *
 *  Version 2.0 (CRC-16 and Modem7 batch file transfer) -- Steve Grandi, NOAO (5/85)
 *
 *  Version 2.1 (1K packets) -- Steve Grandi, NOAO (7/85)
 *
 *  Version 2.2 (general clean-ups and multi-character read speed-ups) -- Steve Grandi, NOAO (9/85)
 *
 *  Version 2.3 (napping while reading packet; split into several source files) -- Steve Grandi, NOAO (1/86)
 *
 *  Version 3.0 (Ymodem batch receive; associated changes) -- Steve Grandi, NOAO (2/86)
 *
 *  Version 3.1 (Ymodem batch send; associated changes) -- Steve Grandi, NOAO (8/86)
 *
 *  Version 3.2 (general cleanups) -- Steve Grandi, NOAO (9/86)
 *
 *  Released to the world (1/87)
 *
 *  Version 3.3 (see update.doc) -- Steve Grandi, NOAO (5/87)
 *
 *  Version 3.4 (see update.doc) -- Steve Grandi, NOAO (10/87)
 *
 *  Released to the world (1/88)
 *
 *  Please send bug fixes, additions and comments to:
 *	{ihnp4,hao}!noao!grandi   grandi@noao.arizona.edu
 */

#include "xmodem.h"

main(argc, argv)
int argc;
char **argv;
{
	char *getenv();
	FILE *fopen();
	char *unix_cpm();
	char *strcpy();
	char *strcat();
	
	char *fname = filename;		/* convenient place to stash file names */
	char *logfile = "xmodem.log";	/* Name of LOG File */
	
	char *stamptime();		/* for timestamp */

	char *defname = "xmodem.in";	/* default file name if none given */

	struct stat filestatbuf;	/* file status info */

	int index;
	char flag;
	long expsect;

	/* initialize option flags */

	XMITTYPE = 't';		/* assume text transfer */
	DEBUG = FALSE;		/* keep debugging info in log */
	RECVFLAG = FALSE;	/* not receive */
	SENDFLAG = FALSE;	/* not send either */
	BATCH = FALSE;		/* nor batch */
	CRCMODE = FALSE;	/* use checksums for now */
	DELFLAG = FALSE;	/* don't delete old log file */
	LOGFLAG = TRUE;		/* keep log */
	LONGPACK = FALSE; 	/* do not use long packets on transmit */
	MDM7BAT = FALSE;	/* no MODEM7 batch mode */
	YMDMBAT = FALSE;	/* no YMODEM batch mode */
	TOOBUSY = FALSE;	/* not too busy for sleeping in packet read */

	printf("XMODEM Version %d.%d", VERSION/10, VERSION%10);
	printf(" -- UNIX-Microcomputer Remote File Transfer Facility\n");

	if (argc == 1)
		{
		help();
		exit(-1);
		}

	index = 0;		/* set index for flag loop */

	while ((flag = argv[1][index++]) != '\0')
	    switch (flag) {
		case '-' : break;
		case 'X' :
		case 'x' : DEBUG = TRUE;  /* turn on debugging log */
			   break;
		case 'C' :
		case 'c' : CRCMODE = TRUE; /* enable CRC on receive */
			   break;
		case 'D' :
		case 'd' : DELFLAG = TRUE;  /* delete log file */
			   break;
		case 'L' :
		case 'l' : LOGFLAG = FALSE;  /* turn off log  */
			   break;
		case 'm' :
		case 'M' : MDM7BAT = TRUE;  /* turn on MODEM7 batch protocol */
			   BATCH   = TRUE;
			   break;
		case 'y' :
		case 'Y' : YMDMBAT = TRUE;  /* turn on YMODEM batch protocol */
			   BATCH   = TRUE;
			   break;
		case 'k' :
		case 'K' : LONGPACK = TRUE;  /* use 1K packets on transmit */
			   break;
		case 't' :
		case 'T' : TOOBUSY = TRUE;  /* turn off sleeping */
			   break;
		case 'R' :
		case 'r' : RECVFLAG = TRUE;  /* receive file */
			   XMITTYPE = gettype(argv[1][index++]);  /* get t/b */
			   break;
		case 'S' :
		case 's' : SENDFLAG = TRUE;  /* send file */
			   XMITTYPE = gettype(argv[1][index++]);
			   break;
		default  : printf ("Invalid Flag %c ignored\n", flag);
			   break;
	   }

	if (DEBUG)
		LOGFLAG = TRUE;

	if (LOGFLAG)
	   { 
	     if ((fname = getenv("HOME")) == 0)	/* Get HOME variable */
		error("Fatal - Can't get Environment!", FALSE);
	     fname = strcat(fname, "/");
	     fname = strcat(fname, logfile);
	     if (!DELFLAG)
		LOGFP = fopen(fname, "a");  /* append to LOG file */
	     else
		LOGFP = fopen(fname, "w");  /* new LOG file */
	     if (!LOGFP)
		error("Fatal - Can't Open Log File", FALSE);

	     fprintf(LOGFP,"\n++++++++  %s", stamptime());
	     fprintf(LOGFP,"XMODEM Version %d.%d\n", VERSION/10, VERSION%10);
	     fprintf(LOGFP,"Command line: %s %s", argv[0], argv[1]);
	     for (index=2; index<argc; ++index)
		fprintf(LOGFP, " %s", argv[index]);
	     fprintf(LOGFP, "\n");
	   }

	getspeed();		/* get tty-speed for time estimates */

	if (RECVFLAG && SENDFLAG)
		error("Fatal - Both Send and Receive Functions Specified", FALSE);

	if (MDM7BAT && YMDMBAT)
		error("Fatal - Both YMODEM and MODEM7 Batch Protocols Specified", FALSE);

	if (!RECVFLAG && !SENDFLAG)
		error("Fatal - Either Send or Receive Function must be chosen!",FALSE);
	
	if (SENDFLAG && argc==2)
		error("Fatal - No file specified to send",FALSE);

	if (RECVFLAG && argc==2)
		{
		/* assume we really want CRC-16 in batch, unless we specify MODEM7 mode */ 
		CRCMODE = MDM7BAT ? FALSE : TRUE;
		printf("Ready for BATCH RECEIVE");
		printf(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
		printf("Send several Control-X characters to cancel\n");
		logit("Batch Receive Started");
		logitarg(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
		strcpy(fname, defname);
		}

	if (RECVFLAG && argc>2)
		{
		if(open(argv[2], 0) != -1)  /* check for overwriting */
			{
			logit("Warning -- Target File Exists and is Being Overwritten\n");
			printf("Warning -- Target File Exists and is Being Overwritten\n");
			}
		printf("Ready to RECEIVE File %s", argv[2]);
		printf(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
		printf("Send several Control-X characters to cancel\n");
		logitarg("Receiving in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
		strcpy(fname,argv[2]);
		}

	if (RECVFLAG)
		{  
		setmodes();		/* set tty modes for transfer */

		while(rfile(fname) != FALSE);  /* receive files */

		restoremodes(FALSE);	/* restore normal tty modes */

		sleep(2);		/* give other side time to return to terminal mode */
		exit(0);
		}

	if (SENDFLAG && BATCH) 
		{
		if (YMDMBAT)
			{
			printf("Ready to YMODEM BATCH SEND");
			printf(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
			logit("YMODEM Batch Send Started");
			logitarg(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
			}
		else if (MDM7BAT)
			{
			printf("Ready to MODEM7 BATCH SEND");
			printf(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
			logit("MODEM7 Batch Send Started");
			logitarg(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
			}
		printf("Send several Control-X characters to cancel\n");

		setmodes();
		for (index=2; index<argc; index++) {
			if (stat(argv[index], &filestatbuf) < 0) {
				logitarg("\nFile %s not found\n", argv[index]);
				continue;
			}
			sfile(argv[index]);
		}
		sfile("");
		restoremodes(FALSE);

		logit("Batch Send Complete\n");
		sleep(2);
		exit (0);
		}

	if (SENDFLAG && !BATCH) 
		{
		if (stat(argv[2], &filestatbuf) < 0)
			error("Can't find requested file", FALSE);
		expsect = (filestatbuf.st_size/128)+1;
			
		printf("File %s Ready to SEND", argv[2]);
		printf(" in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");
		printf("Estimated File Size %ldK, %ld Sectors, %ld Bytes\n",
	    	  (filestatbuf.st_size/1024)+1, expsect,
	  	  filestatbuf.st_size);
		projtime(expsect, stdout);
		printf("Send several Control-X characters to cancel\n");
		logitarg("Sending in %s mode\n", (XMITTYPE == 't') ? "text" : "binary");

		setmodes();
		sfile(argv[2]);
		restoremodes(FALSE);

		sleep(2);
		exit(0);
		}
}
