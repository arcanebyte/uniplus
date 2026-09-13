#ifndef lint
static char sccsid[] = "@(#)installd.c	1.0 (Sri-Tsc) 5/11/83";
#endif

/*
 * installd
 *
 * installd is the program that should be invoked when install request
 * is being made from another system. This program will be started up by
 * the exec daemon. It will read and write to standard input and output.
 * Here is a description of the "protocol" with which this program will
 * communicate to install:
 *
 *  install		installd
 *
 *  filename ---------->	eventual name of file
 *  file size --------->	size in 8 bit bytes
 *  option ------------> (consists of the characters c for copy
 *						     b copy w/backup
 *						     k checksum
 *						     a for append
 *						     r for remove
 *         <----------- 0 for OK; 1 for some error
 *				  error text string follows.
 *
 * a & b & c:
 *  file size --------->
 * 	   <----------- 0 for OK; 1 for error
 *  file contents ----->	file size bytes
 *         <----------- 0 for OK; 1 for error
 *  file protection --->	for chmod
 *         <----------- 0 for OK; 1 for error
 *
 * for k:
 *	   <----------- sum output line
 *
 * for r:
 *	   <----------- OK; 1 for error
 *
 *  This will be performed in a loop until stdin reaches EOF.
 */

#include <stdio.h>

/* character options from rinstall */

#define	APPEND	'a'		/* append info */
#define	BACKUP_COPY	'b'	/* copy but make backup if possible */
#define	CHECKSUM	'k'	/* checksum files */
#define	COPY	'c'		/* default thing to do */
#define	REMOVE	'r'		/* remove files */

char *mktemp();
long atol();

char buffer[BUFSIZ];	/* character input buffer */
char file_name[BUFSIZ];	/* file name */
char option;		/* what we have to do */
long file_size;		/* number of bytes in file */
unsigned file_mode;	/* file protection mode */
char tempfile[] = "/tmp/insXXXXXX";	/* temp. file template */
char *tptr;		/* ptr to file name */
int tfid;		/* temp. file descriptor */

main(argc, argv)
int argc;
char **argv;
{
	char *ptr;
int f;

	if (argc > 1) {
		fprintf(stderr, "included: no arguements required.\n");
		exit(1);
	}

	while(read(0, buffer, BUFSIZ)) { /* fetch file name, size, option */
		ptr = buffer;
		while (*ptr != '\n')
			ptr++;
		*ptr++ = NULL;
		strcpy(file_name, buffer);	/* and save it */
		option = *ptr;			/* save function */
		buffer[0] = 0;			/* ack */
		write(1, buffer, 1);
		switch (option) {

		case APPEND:			/* append file */
		case BACKUP_COPY:		/* copy w/backup */
		case COPY:			/* copy file */
			if (read(0, buffer, BUFSIZ) <= 0) {
				error("file mode","premature EOF or file read error");
				goto another;
			}
			file_size = atol(buffer); /* get number of bytes */
			buffer[0] = 0;			/* ack */
			write(1, buffer, 1);
			tptr = mktemp(tempfile);/* create a temp. file name */
			if ((tfid = creat(tptr, 0700)) == -1) {
				error("creat", "could not create temp. file");
				goto another;
			}
			copyfile();
			if (read(0, buffer, BUFSIZ) <= 0) {
				error("file mode","premature EOF or file read error");
				goto another;
			}
			file_mode = atoi(buffer);
			if (!access(file_name, 0) && option != APPEND)
				if (option == BACKUP_COPY) {
					sprintf(buffer,"mv %s %s-\n", file_name, file_name);
					if (system(buffer)) {
						error("could not do command", buffer);
						goto another;
					}
				}
				else 
					if (unlink(file_name)) {
						error(file_name, "could not unlink file");
						goto another;
					}
			if (option == APPEND)
				sprintf(buffer, "cat %s >> %s; rm %s\n", tptr, file_name, tptr);
			else
				sprintf(buffer,"mv %s %s\n", tptr, file_name);
			if (system(buffer)) {		/* do the move */
				error("could not do command", buffer);
				goto another;
			}
			chmod(file_name,file_mode);
			buffer[0] = 0;
			write(1, buffer, 1);
			break;

		case CHECKSUM:			/* checksum */
			sprintf(buffer, "sum %s\n", file_name);
			system(buffer);
			break;

		case REMOVE:			/* remove */
			if (unlink(file_name)) {
				error(file_name, "could not unlink file");
				goto another;
			}
			buffer[0] = 0;
			write(1, buffer, 1);
			break;
		}
another: ;
	}
}

/*
copyfile
copyfile will copy file_size many bytes from stdin to a temporarily created
file. The filename will be passed back via buffer.
*/

copyfile()
{
	long cnt;				/* file size counter */
	int i;


	cnt = file_size;		/* count down input bytes */
	while (cnt > 0) {
		i = read(0, buffer, BUFSIZ);
		write(tfid, buffer, i);
		cnt =- i;
	}
	close(tfid);
	buffer[0] = 0;
	write(1, buffer, 1);
	strcpy(buffer, tptr);
}

/*
error(option, string)
char *option, *string;
This routine is called when some error condition has been encountered.
option contains an identifier message, while string contains the actual
error message. Before message is printed, a non-null character is output
first, then the string error message.
*/

error(option, string)
char *option, *string;
{
	char buf[BUFSIZ];


	buf[0] = 1;
	write(1, buf, 1);	/* nak */
	sprintf(buf, "installd: %s: %s\n", option, string);
	write(1, buf, strlen(buf)+1);
}
