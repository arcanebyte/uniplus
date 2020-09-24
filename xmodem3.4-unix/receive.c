#include "xmodem.h"

/**  receive a file  **/

/* returns TRUE if in the midst of a batch transfer */
/* returns FALSE if no more files are coming */

/* This routine is one HUGE do-while loop with far to many indented levels.
 * I chose this route to facilitate error processing and to avoid GOTOs.
 * Given the troubles I've had keeping the nested IF statements straight,
 * I was probably mistaken...
 */

rfile(name)
char *name;
    {

    char *sectdisp();
    char *cpm_unix();
    char *strcpy();
    char *ctime();
    time_t time();

    int fd,     /* file descriptor for created file */
    checksum,   /* packet checksum */
    firstchar,  /* first character of a packet */
    sectnum,    /* number of last received packet (modulo 128) */
    sectcurr,   /* second byte of packet--should be packet number (mod 128) */
    sectcomp,   /* third byte of packet--should be complement of sectcurr */
    tmode,      /* text mode if true, binary mode if false */
    errors,     /* running count of errors (reset when 1st packet starts */
    errorflag,  /* set true when packet (or first char of putative packet) is invalid */
    fatalerror, /* set within main "read-packet" Do-While when bad error found */
    inchecksum, /* incoming checksum or CRC */
    expsect,    /* expected number of sectors (YMODEM batch) */
    firstwait,  /* seconds to wait for first character in a packet */
    bufsize;    /* packet size (128 or 1024) */
    long recvsectcnt;   /* running sector count (128 byte sectors) */
    long modtime;   /* Unix style file mod time from YMODEM header */
    int filemode; /* Unix style file mode from YMODEM header */
    long readbackup;    /* "backup" value for characters read in file */
    time_t timep[2];    /* used in setting mod time of received file */
    char *p;	/* generic pointer */
    int bufctr; /* number of real chars in read packet */
    unsigned char *nameptr; /* ptr in filename for MODEM7 protocol */
    time_t start;   /* starting time of transfer */
    int openflag = FALSE;   /* is file open for writing? */

    logit("----\nXMODEM File Receive Function\n");
    if (CRCMODE)
        logit("CRC mode requested\n");

    BATCH = FALSE;          /* don't know if really are in batch mode ! */
    fatalerror = FALSE;
    firstwait = WAITFIRST;  /* For first packet, wait short time */
    sectnum = errors = recvsectcnt = 0;
    bufsize = 128;
    modtime = 0l; filemode = 0;
    filelength = 0l; fileread =0l; CHECKLENGTH = FALSE;

    tmode = (XMITTYPE == 't') ? TRUE : FALSE;

    /* start up transfer */
    if (CRCMODE)        
	{
        sendbyte(CRCCHR);
	if (LONGPACK && !MDM7BAT)
	    sendbyte(KCHR);
	}
    else
        sendbyte(NAK);


    do                  /* start of MAIN Do-While loop to read packets */
        {   
        errorflag = FALSE;
        do              /* start by reading first byte in packet */
            {
            firstchar = readbyte(firstwait);
            } 
            while ((firstchar != SOH) 
                && (firstchar != STX) 
                && (firstchar != EOT) 
                && (firstchar != ACK || recvsectcnt > 0) 
                && (firstchar != TIMEOUT) 
                && (firstchar != CAN || recvsectcnt > 0));

        if (firstchar == EOT)           /* check for REAL EOT */
            {
			sendbyte(NAK);				/* NAK the EOT */
			if ((firstchar = readbyte(3)) != EOT)	/* check next character */
				{
				logit("Spurious EOT detected; ignored\n");
				if ((firstchar == SOH) || (firstchar == STX) ||
					(firstchar == ACK && recvsectcnt == 0) ||
					(firstchar == CAN && recvsectcnt == 0) ||
					(firstchar == TIMEOUT))
						break;
				else
					{
					firstchar = 0;
					errorflag = TRUE;
					}
				}
            }

        if (firstchar == TIMEOUT)       /* timeout? */
            {  
            if (recvsectcnt > 0)
                logitarg("Timeout on Sector %s\n", sectdisp(recvsectcnt,bufsize,1));
            errorflag = TRUE;
            }

        if (firstchar == CAN)           /* bailing out? (only at beginning) */
            {
            if ((readbyte(3) & 0x7f) == CAN)
                error("Reception canceled at user's request",TRUE);
            else
                {
                errorflag = TRUE;
                logit("Received single CAN character\n");
                }
            }

        if (firstchar == ACK)           /* MODEM7 batch? (only at beginning) */
            {
            int i,c; 

            logit("MODEM7 Batch Protocol\n");
            nameptr = buff;
            checksum = 0;

            for (i=0; i<NAMSIZ; i++)
                {
                c = readbyte(3);

                if (c == CAN)
                    {
                    if (readbyte(3) == CAN)
                        error("Program Canceled by User", TRUE);
                    else
                        {
                        logit("Received single CAN character in MODEM7 filename\n");
                        errorflag = TRUE;
                        break;
                        }
                    }

                if (c == EOT && i == 0)
                    {
		    sendbyte(NAK);	/* NAK the EOT to force verification */
		    if ((c=readbyte(3)) != EOT)
			{
			logit("Spurious EOT detected in MODEM7 filename\n");
			errorflag = TRUE;
			break;
			}
                    sendbyte(ACK);			/* acknowledge EOT */
                    while (readbyte(1) != TIMEOUT)	/* flush garbage */
                        ;
                    logit("MODEM7 Batch Receive Complete\n");
                    return (FALSE);
                    }

                if (c == TIMEOUT)
                    {
                    logit("Timeout waiting for MODEM7 filename character\n");
                    errorflag = TRUE;
                    break;
                    }

                if (c == BAD_NAME)
                    {
                    logit("Error during MODEM7 filename transfer\n");
                    errorflag = TRUE;
                    break;
                    }

                *nameptr++ = c;
                checksum += c;
                sendbyte(ACK);
                }

            if (!errorflag)
                {
                c = readbyte(3);
                if (c == CTRLZ)     /* OK; end of string found */
                    {
                    sendbyte(checksum + CTRLZ);
                    if (readbyte(15) == ACK)     /* file name found! */
                        {
                        xmdebug("MODEM7 file name OK");
                        *nameptr = '\000';	/* unixify the file name */
                        name = cpm_unix(buff);
                        BATCH = TRUE;
                        logitarg("MODEM7 file name: %s\n", name);
			errors = 0;		/* restart crc handshake */
			sleep(2);		/* give other side a chance */
                        }
                    else
                        {
                        logit("Checksum error in MODEM7 filename\n");
                        errorflag = TRUE;
                        }
                    }
                else
                    {
                    logit("Length error in MODEM7 fielname\n");
                    errorflag = TRUE;
                    }
                }
            }
            

        if (firstchar == SOH || firstchar == STX)  /* start reading packet */
            {
            bufsize = (firstchar == SOH) ? 128 : 1024;

            if (recvsectcnt == 0)           /* 1st data packet, initialize */
                {
                if (bufsize == 1024)
                    logit("1K packet mode chosen\n");
                start = time((time_t *) 0);
                errors = 0;
                firstwait = 5;
                }

	    sectcurr = readbyte(3);
	    sectcomp = readbyte(3);
		if ((sectcurr + sectcomp) == 0xff)  /* is packet number checksum correct? */
		    {  
			if (sectcurr == ((sectnum+1) & 0xff))   /* is packet number correct? */
			    {  
			    if (DEBUG)
			        fprintf(LOGFP,"DEBUG: packet %d started\n", sectnum);

            /* Read, process and calculate checksum for a buffer of data */

                            readbackup = fileread;
			    if (readbuf(bufsize, 1, tmode, recvsectcnt, &checksum, &bufctr) != TIMEOUT) 
                                {

		    /* verify checksum or CRC */

				if (CRCMODE) 
                                    {
				    checksum &= 0xffff;
				    inchecksum = readbyte(3);  /* get 16-bit CRC */
				    inchecksum = (inchecksum<<8) | readbyte(3);
				    }
                        
				else
				    inchecksum = readbyte(3);  /* get simple 8-bit checksum */

				if (inchecksum == checksum) /* good checksum, hence good packet */
				    {  
				    xmdebug("checksum ok");
				    errors = 0;
				    recvsectcnt += (bufsize == 128) ? 1 : 8;
				    sectnum = sectcurr; 

				    if (!openflag)      /* open output file if necessary */
					{
					openflag = TRUE;
					if ((fd = creat(name, CREATMODE)) < 0)
					    {
					    sendbyte(CAN); sendbyte(CAN); sendbyte(CAN);
					    error("Can't create file for receive", TRUE);
					    }
                                        if (!BATCH)
					    logitarg("File Name: %s\n", name);
					}

				    if (write(fd, (char *) buff, bufctr) != bufctr)
					{
					close(fd);
					unlink(name);
					error("File Write Error", TRUE);
					}
				    else
					sendbyte(ACK);      /* ACK the received packet */
				    }

            /* Start handling various errors and special conditions */

				else        /* bad checksum */
				    {  
				    logitarg("Checksum Error on Sector %s:  ", sectdisp(recvsectcnt,bufsize,1));
				    logitarg("sent=%x  ", inchecksum);
				    logitarg("recvd=%x\n", checksum);
                                    fileread = readbackup;
				    errorflag = TRUE;
				    }
                                }

			    else    /* read timeout */
				{
				logitarg("Timeout while reading sector %s\n",sectdisp(recvsectcnt,bufsize,1));
                                fileread = readbackup;
				errorflag = TRUE;
				}
			    }

                        else        /* sector number is wrong OR Ymodem filename */
                            { 
			    if (sectcurr == 0 && recvsectcnt == 0)  /* Ymodem file-name packet */
				{
				logit("YMODEM Batch Protocol\n");

				/* Read and process a file-name packet */

				if (readbuf(bufsize, 1, FALSE, recvsectcnt, &checksum, &bufctr) != TIMEOUT) 
                                    {

				    /* verify checksum or CRC */

				    if (CRCMODE) 
                                        {
					checksum &= 0xffff;
					inchecksum = readbyte(3);  /* get 16-bit CRC */
					inchecksum = (inchecksum<<8) | readbyte(3);
				        }
                        
				    else
					inchecksum = readbyte(3);  /* get simple 8-bit checksum */

				    if (inchecksum == checksum) /* good checksum, hence good filename */
					{
					xmdebug("checksum ok");
					strcpy(name, (char *)buff);
                                        expsect = ((buff[bufsize-1]<<8) | buff[bufsize-2]);
					BATCH = TRUE;
					if (strlen(name) == 0)  /* check for no more files */
					    {
						sendbyte(ACK);      /* ACK the packet */
					    logit("YMODEM Batch Receive Complete\n");
					    return (FALSE);
					    }
					unixify(name);       /* make filename canonical */

                                        /* read rest of YMODEM header */
					p = (char *)buff + strlen((char *)buff) + 1;
					sscanf(p, "%ld%lo%o", &filelength, &modtime, &filemode);
					logitarg("YMODEM file name: %s\n", name);
                                        fileread = 0l;
                                        if (filelength)
                                            {
                                            CHECKLENGTH = TRUE;
                                            logitarg("YMODEM file size: %ld\n", filelength);
                                            }
                                        else
					    logitarg("YMODEM estimated file length %d sectors\n", expsect);
                                        if (modtime)
                                            {
                                            logitarg("YMODEM file date: %s", ctime(&modtime));
                                            }
                                        if (filemode)
                                            logitarg("YMODEM file mode: %o", filemode);

					sendbyte(ACK);      /* ACK the packet */
					}

				    else                /* bad filename checksum */
					{
					logit("checksum error on filename sector\n");
					errorflag = TRUE;
					}
				    }
				else
				    {
				    logit("Timeout while reading filename packet\n");
				    errorflag = TRUE;
                                    }
				}

			    else if (sectcurr == sectnum)   /* duplicate sector? */
				{  
				logitarg("Duplicate sector %s flushed\n", sectdisp(recvsectcnt,bufsize,0));
				while(readbyte(3) != TIMEOUT)
				    ;
				sendbyte(ACK);
				}
			    else                /* no, real phase error */
				{
				logitarg("Phase Error - Expected packet is %s\n", sectdisp(recvsectcnt,bufsize,1));
				errorflag = TRUE;
				fatalerror = TRUE;
				}
			    }
		    }

		else        /* bad packet number checksum */
		    {  
		    logitarg("Header Sector Number Error on Sector %s\n", sectdisp(recvsectcnt, bufsize,1));
		    errorflag = TRUE;
		    }

	    }           /* END reading packet loop */
    
	if ((errorflag && !fatalerror) ||		/* check on errors or batch transfers */
	  ((recvsectcnt == 0) && (firstchar != EOT)))	
	    {  
	    if (errorflag)
	        errors++;
	    if (recvsectcnt != 0)
		while (readbyte(3) != TIMEOUT)  /* wait for line to settle if not beginning */
		    ;

	    if (recvsectcnt == 0 && errors >= STERRORMAX)
		fatalerror = TRUE;
	    else if (CRCMODE && recvsectcnt == 0 && errors == CRCSWMAX)
		{
		CRCMODE = FALSE;
		logit("Sender not accepting CRC request, changing to checksum\n");
		sendbyte(NAK);
		}
	    else if (!CRCMODE && recvsectcnt == 0 && errors == CRCSWMAX)
		{
		CRCMODE = TRUE;
		logit("Sender not accepting checksum request, changing to CRC\n");
		sendbyte(CRCCHR);
		}
	    else if (CRCMODE && recvsectcnt == 0)
		sendbyte(CRCCHR);
	    else
		sendbyte(NAK);
	    }

        if (errors > ERRORMAX && recvsectcnt != 0)  /* over error limit? */
            fatalerror = TRUE;
	}
        while ((firstchar != EOT) && !fatalerror);   /* end of MAIN Do-While */

	if ((firstchar == EOT) && !fatalerror)  /* normal exit? */
	    {
	    if (openflag)       /* close the file */
	        close(fd);
	    sendbyte(ACK);	/* ACK the EOT */
	    logit("Receive Complete\n");
	    prtime (recvsectcnt, time((time_t *) 0) - start);

            if (openflag && modtime)   /* set file modification time */
                {
                timep[0] = time((time_t *) 0);
                timep[1] = modtime;
                utime(name, timep);
                }
        
	    if (BATCH)          /* send appropriate return code */
		return(TRUE);
	    else
		return(FALSE);
	    }
	else                /* no, error exit */
	    { 
	    if (recvsectcnt != 0)
		sendbyte(CAN); sendbyte(CAN); sendbyte(CAN); sendbyte(CAN); sendbyte(CAN);
		if (openflag)
		    {
		    close(fd);
		    unlink(name);
		    }
		error("ABORTED -- Too Many Errors--deleting file", TRUE);
		return (FALSE);
	    }
    }
