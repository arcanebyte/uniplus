#ifndef lint
static char sccsid[] = "@(#)rinstall.c	1.0 (Sri-Tsc) 5/10/83";
#endif

/*
 * rinstall [-ssystems] [-afile] [-bcrv] file1 [file2] ...
 *
 * rinstall will take a file name and copy it to the same directory
 * on all of the other systems mentioned. The -s flag can be followed
 * by a comma seperated list of systems to install to. Otherwise,
 * rinstall will look for the environment variable call "systems" which
 * should contain a comma seperated list of system names. If that name
 * does not exist, an assumed default systems will be used.
 *
 * The -a flag will append the file to the file list. - will read from
 *  standard input.
 * The -c flag will return checksum information (from the sum program)
 *  on all files.
 * The -b flag will cause the a copy of the original, if it exists, to
 *  be move to its original name with a "-" appended to it.
 * The -r flag will remove the given files.
 * The -v flag will cause verbose output.
 *
 */

#define SYSTEMS "tsca,tscb,prmh,joyce"
#define INSTALL_DEMON "/etc/installd"
#define INSTALL_LOG "/usr/lib/installd.log"

/* characters to send to installd */

#define	APPEND	'a'		/* append info */
#define	BACKUP_COPY	'b'	/* copy but make backup if possible */
#define	CHECKSUM	'k'	/* checksum files */
#define	COPY	'c'		/* default thing to do */
#define	REMOVE	'r'		/* remove files */

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/in.h>

char *getenv();			/* returns a ptr to environment value */
struct passwd *getpwuid();
FILE *fopen();
char *ctime();
long time();
char *mktemp();

int rem;			/* remote socket descriptor */
int aflag = 0;			/* append option */
char *aptr;			/* ptr to append file */
int cflag = 0;			/* checksum info */
int bflag = 0;			/* backup flag */
int rflag = 0;			/* remove flag */
int vflag = 0;			/* verbose flag */
int argc2;			/* hold argc for counting */
char **argv2;			/* and argv too */
char tempfile[] = "/tmp/rinsXXXXXX";	/* temp. file template */
int gotinput = 0;		/* say we get his input for -a- */

main(argc, argv)
char **argv;
int argc;
{
	char *sptr = 0;		/* ptr to systems to install to */
	char *hptr;		/* ptr to beginning of singe host name */
	char buffer[BUFSIZ];	/* place to hold system name temporarily */
	struct passwd *pwd;	/* given passwd entry */
	int loc;		/* local file file descriptor */
	struct stat fstatus;	/* where file info will go */
	int n;			/* integer temporary */
	char sysname[14];	/* where system name will go */
	FILE *logfile;		/* where audit trail will go */
	long clock;		/* put time in here */
	char option;		/* what to do */
	char *tptr;		/* ptr to temp file name */
	char c;			/* place to put a character */
	long ready;		/* */

	while (--argc > 0 && **++argv == '-') {

		switch ((*argv)[1]) {

		case 'a':			/* append option */
			aptr =(*argv)+2;	/* ptr to file name */
			aflag = 1;
			break;
		case 'c':			/* run "sum" program */
			cflag = 1;
			break;
		case 'b':			/* mv backup copy */
			bflag = 1;
			break;
		case 'r':			/* remove files */
			rflag = 1;
			break;
		case 's':
			sptr = (*argv)+2;	/* ptr to rest of string */
			break;
		case 'v':
			vflag = 1;		/* make a pest of ourself */
			break;
		default:
			goto badflg;
		}
	}
	if (!argc) {
badflg:		fprintf(stderr,"usage: rinstall [-bcrv][-afile][-ssys1,sys2...] file1 [file2] ...\n");
		exit(1);
	}
	if ((rflag&&aflag)||(rflag&&cflag)||(cflag&&aflag)) {
		fprintf(stderr, "rinstall: conflicting flags\n");
		exit(1);
	}
	pwd = getpwuid(getuid());
	if (pwd == NULL) {
		fprintf(stderr, "rinstall: who are you?\n");
		exit(1);
	}

	if (!sptr) 		/* were we supplied systems */
		if (!(sptr = getenv("SYSTEMS")))
			sptr = SYSTEMS; /* use our default */
		
			if (aflag)
				option = APPEND;	/* want to append */
			else
				if (bflag)
					option = BACKUP_COPY;/*copy w/backup*/
				else
					if (cflag)
						option = CHECKSUM;
					else
						if (rflag)
							option = REMOVE;
						else
							option = COPY;
	if (vflag)
		fprintf(stdout, "Installing to %s\n", sptr);
	while (*sptr) {	/* for each system we want to send to */
		n = 0;
		while (*sptr != ',' && *sptr)
			sysname[n++] = *sptr++;
		if (*sptr == ',')
			sptr++;
		sysname[n] = NULL;
		hptr = sysname;
		rem = rcmd(&hptr, IPPORT_CMDSERVER, pwd->pw_name,
			    pwd->pw_name, INSTALL_DEMON, 0);
		if (rem == -1)
			exit(1);
		if ((logfile = fopen(INSTALL_LOG, "a")) != NULL) {
			fprintf(logfile, "%s ", pwd->pw_name);
			if (aflag) fprintf(logfile, "appended %s to ", aptr);
			else if (cflag) fprintf(logfile, "checksummed ");
			else if (rflag) fprintf(logfile, "removed ");
			else if (bflag) fprintf(logfile, "copied w/backup to ");
			else fprintf(logfile, "copied to ");
		}
		argc2 = argc;
		argv2 = argv;
		while (argc2--) {		/* process each file name */
			if (**argv2 != '/') {
				fprintf(stderr, "rinstall: %s: must use absolute path name.\n", *argv2);
				exit(1);
			}
			else
				if (logfile != NULL)
					fprintf(logfile, "%s ", *argv2);
			argv2++;
		}
		if (logfile != NULL) {
			time(&clock);
			fprintf(logfile,"on %s at %s",sysname, ctime(&clock));
			fclose(logfile);
		}
		argc2 = argc;
		argv2 = argv;
		while (argc2--) {		/* process each file name */
			if (vflag) {
				if (aflag)
					fprintf(stdout, "Appending %s to",(strcmp(aptr, "-") ? aptr : "(stdin)"));
				else if (cflag)
					fprintf(stdout, "Checksum of");
				else if (rflag)
					fprintf(stdout, "Removing");
				else if (bflag)
					fprintf(stdout, "Copying w/backup to");
				else fprintf(stdout, "Copying to");
				fprintf(stdout, " %s at %s.\n", *argv2, sysname);
			}
			sprintf(buffer, "%s\n%c", *argv2, option);
			write(rem, buffer, strlen(buffer)+1);
			read(rem, buffer, BUFSIZ);	/* get result */
			if (buffer[0])		/* problem */
				problem(1);	/* yes, go handle it */
			if (option == COPY || option == BACKUP_COPY|| option == APPEND) {
				if (option == APPEND) {
					if (!strcmp(aptr, "-")) {
						if (!gotinput) {
							gotinput = 1;	
							tptr = mktemp(tempfile);
							if ((logfile = fopen(tptr, "w")) == NULL) {
								fprintf(stderr, "rinstall: could not create temp. file\n");
								exit(1);
							}
							fprintf(stdout, "Enter text followed by control-d (^D):\n");
							fflush(stdout);
							while((c = getchar()) != EOF)
								putc(c, logfile);
							fclose(logfile);
						}
						loc = open(tptr, 0);
					}
					else
						loc = open(aptr, 0);
				}
				else
					loc = open(*argv2, 0);  /* open file read */
				if (loc < 0) {
					fprintf(stderr, "rinstall: couldn't open %s to read.\n", (option==APPEND ? aptr : *argv2));
					exit(1);
				}
				if (fstat(loc, &fstatus)) {
					fprintf(stderr, "rinstall: could not get file status: %s\n", *argv2);
					exit(1);
				}
				sprintf(buffer, "%D",fstatus.st_size);
				write(rem, buffer, strlen(buffer)+1);
				read(rem, buffer, BUFSIZ);
				if (buffer[0])		/* problem */
					problem(1);
				while (n = read(loc, buffer, BUFSIZ))
					write(rem, buffer, n);
				close(loc);
				read(rem, buffer, BUFSIZ); /* get result */
				if (buffer[0])		/* problem */
					problem(1);	/*yes, go handle it */
				sprintf(buffer, "%u", fstatus.st_mode&07777);
				write(rem, buffer, strlen(buffer)+1);
				read(rem, buffer, BUFSIZ); /* get result */
				if (buffer[0])		/* problem */
					problem(1);	/*yes, go handle it */
				if (option == APPEND && !strcmp(aptr, "-"))
					unlink(tptr);
			}
			else if (option == CHECKSUM) {
				sleep(3);
				buffer[read(rem, buffer, BUFSIZ)] = '\0';
				fprintf(stdout, "%s:%s:\t%s", sysname, *argv2, buffer);
			}
			else if (option == REMOVE) {
				read(rem, buffer, BUFSIZ);
				if (buffer[0])
					problem(0);
			}
			else {
				fprintf(stderr, "rinstall: unknown option\n");
				exit(1);
			}
		argv2++;			/* point to next file */
		}
	close(rem);			/* tell other system done */
	}
}


/*
 * problem() is called when the install demon process return a non-zero reply.
 * This usually means something recognizable went wrong and we should expect
 * a reason to follow. Read in the reason, output it on the terminal and die.
 */

problem(die)
int die;
{
	char buf[BUFSIZ];	/* place to read into */

	if (read(rem, buf, BUFSIZ) > 0)		/* if we have something */
		fprintf(stderr, "rinstall: %s", buf);
	if (die) {
		close(rem);		/* close network channel */
		exit(1);
	}
}
