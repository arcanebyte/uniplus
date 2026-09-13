/*
 * login [ name ]
 * login [ name.group ]
 * login -r hostname (for rlogind)
 * login -h hostname (for telnetd, etc.)
 */

#define	LOGERR			/* log all errors to the console */
#define	GROUP			/* allow login as name.group */
#define SP_SESS			/* "special session"-- root logins only */
				/* used after autoreboot failures */

#include <whoami.h>
#include <sys/types.h>
#include <sgtty.h>
#include <utmp.h>

#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <lastlog.h>
#include <ctype.h>
#ifdef	GROUP
#include <grp.h>
#endif

							/* default path */
#define	PATH	"PATH=:/usr/ucb:/bin:/usr/bin:/usr/local:/usr/hosts"
#define	SHELL	"/bin/sh"				/* default shell */
#define	JCLCSH	"/bin/csh"	/* job control shell, needs new line disc. */
#define	TIMEOUT	60			/* maximum amount of time to login */
#define	TRIES	4			/* number permitted login attempts */
	/*
	 * The umask is a local decision.  077 is very paranoid (everything is
	 * highly secret).  0 is wide open (everything readable and writable
	 * by anyone.)  022 is moderate.  027 is also a possibility.
	 */
#define UMASK	022

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define NMAX	sizeof(utmp.ut_name)
#define LMAX	sizeof(utmp.ut_line)
#define CNTL(x)	('x'&037)
#define UNDEF	'\377'

char	maildir[30] =	"/usr/spool/mail/";
char	qlog[] =	".hushlogin";
char	lastlog[] =	"/usr/adm/lastlog";
char	nolog[] =	"/etc/nologin";
struct	passwd nouser = {"", "nope"};
struct	sgttyb ttyb;
struct	utmp utmp;
char	*ttyn;
int	timeout = TIMEOUT;		/* it can be set during runtime */
char	hostname[32];
char	minusnam[16] = "-";
char	homedir[64] = "HOME=";
char	term[64] = "TERM=";
char	shell[64] = "SHELL=";
char	user[NMAX+9] = "USER=";


char	*envinit[] = {homedir, PATH, term, shell, user, 0};

#ifdef MENLO_JCL
struct	ltchars ltc =
	{ CNTL(z), CNTL(y), CNTL(r), CNTL(o), CNTL(w), CNTL(v)
};
#endif
struct	passwd *pwd;

struct	passwd *getpwnam();
char	*strcat();
int	setpwent();
char	*ttyname();
char	*crypt();
char	*getpass();
int	timedout();
#ifdef	GROUP
#define GRLEN	30		/* max length of group name */
char	group[GRLEN];
struct group *grp, *getgrnam();
#endif
#ifdef	SP_SESS
int	sp_sess = 0;
#endif
char	*rindex(), *index();
extern	char **environ;
extern	char _sobuf[];

int     rflag;
char    rusername[NMAX+1], lusername[NMAX+1];
char    rpassword[NMAX+1];
char    *rhost = "";

main(argc, argv)
char **argv;
{
	register char *namep;
	int t, f, c, wasslash, ldisc;
	int quietlog, zero = 0;
	char *cp;
	FILE *nlfd;

	setbuf(stdout, _sobuf);
	signal(SIGALRM, timedout);
	alarm(timeout);
	signal(SIGQUIT, SIG_IGN);
	nice(-100);
	nice(20);
	nice(0);

#ifdef	SP_SESS
	if (argv[0][0] == 's')
		sp_sess++;
#endif

	/*
	 * -r is used by rlogind to cause the autologin protocol;
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in lastlog
	 */
	if (argc > 1) {
		if (strcmp(argv[1], "-r") == 0) {
			rflag = remotelogin(argv[2]);
			rhost = argv[2];
			argc = 0;
		}
		if (strcmp(argv[1], "-h") == 0 && getuid() == 0) {
			rhost = argv[2];
			argc = 0;
		}
	}
	ioctl(0, TIOCLSET, &zero);
	ioctl(0, TIOCNXCL, 0);
	ioctl(0, FIONBIO, &zero);
	ioctl(0, FIOASYNC, &zero);
	ioctl(0, TIOCGETP, &ttyb);
	/*
	 * If talking to an rlogin process,
	 * propagate the terminal type and
	 * baud rate across the network.
	 */
	if (rflag)
		remoteterm(term, &ttyb);
	ioctl(0, TIOCSLTC, &ltc);
	ioctl(0, TIOCSETP, &ttyb);
	for (t=3; t<20; t++)
		close(t);
	ttyn = ttyname(0);
	if (ttyn==0)
		ttyn = "/dev/tty??";

	gethostname(hostname, sizeof (hostname));
	t = 0;
    loop:
	if (t > 0 && rflag)		/* no extra tries for remote login */
		exit(1);
	if (++t >TRIES) {
		logerr("\r\nlogin: failure on %s, %d login attempts, last username: %s %s\r\n",ttyn+5, TRIES, utmp.ut_name, rhost);
		ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
		close(0);
		sleep(2);
		exit(1);
	}
	SCPYN(utmp.ut_name, "");
#ifdef	GROUP
	SCPYN(group, "");
	if (argc>1) {
		register char *av = argv[1];
		namep = utmp.ut_name;
		while (namep < utmp.ut_name+NMAX) {
			if (*av == 0 || *av == '.')
				break;
			*namep++ = *av++;
		}
		if (*av++ == '.')
		    for (namep=group; namep<group+GRLEN; )
			if ((*namep++ = *av++) == 0)
				break;
		argc = 0;
	}
#else
	if (argc>1) {
		SCPYN(utmp.ut_name, argv[1]);
		argc = 0;
	}
#endif

/*
 * If remote login take given name,
 * otherwise prompt user for something.
 */
	if (rflag) {
		SCPYN(utmp.ut_name, lusername);
		/* autologin failed, prompt for passwd */
		if (rflag == -1)
			rflag = 0;
	} else
		getloginname(&utmp);
/*
 * If no remote login authentication and
 * a password exists for this user, prompt
 * for one and verify it.
 */

	if (!rflag && *pwd->pw_passwd != '\0') {
		namep = crypt(getpass("Password:"),pwd->pw_passwd);
		if (strcmp(namep, pwd->pw_passwd)) {
			printf("Login incorrect\n");
			goto loop;
		}
	}
#ifdef	GROUP
	if (group[0]) {
	    register i;
	    grp = getgrnam(group);
	    endgrent();
	    if(grp == 0) {
		printf("Login incorrect\n");
		goto loop;
	    }
	    for(i=0;grp->gr_mem[i];i++) 
		if(strcmp(grp->gr_mem[i], pwd->pw_name) == 0)
		    break;
	    if(grp->gr_mem[i] == 0) {
		printf("Login incorrect\n");
		goto loop;
	    }

	    if(grp->gr_passwd[0] != '\0' && pwd->pw_passwd[0] == '\0') {
		if(strcmp(grp->gr_passwd,
		    crypt(getpass("Password:"),grp->gr_passwd)) != 0) {
			printf("Login incorrect\n");
			goto loop;
		}
	    }
	    pwd->pw_gid = grp->gr_gid;
	}
#endif
	if (pwd->pw_uid != 0 && (nlfd = fopen(nolog, "r")) > 0) {
		/* logins are disabled except for root */
		while ((c = getc(nlfd)) != EOF)
			putchar(c);
		fflush(stdout);
		sleep(5);
		exit(0);
	}
#ifdef	SP_SESS
	if(sp_sess && pwd->pw_uid != 0) {
		printf("Sorry.  You cannot login at this time.\n");
		exit(0);
	}
#endif
	time(&utmp.ut_time);
	t = ttyslot();
/*
 *	Update utmp and wtmp
 */
	if (t>0 && (f = open("/etc/utmp", 1)) >= 0) {
		lseek(f, (long)(t*sizeof(utmp)), 0);
		SCPYN(utmp.ut_line, rindex(ttyn, '/')+1);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if (t>0 && (f = open("/usr/adm/wtmp", 1)) >= 0) {
		lseek(f, 0L, 2);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if (term[strlen("TERM=")] == '\0')
		getterm();
	chown(ttyn, pwd->pw_uid, pwd->pw_gid);
	chmod(ttyn, 0622);
	setgid(pwd->pw_gid);
	setuid(pwd->pw_uid);
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = SHELL;
#ifdef	MENLO_JCL
	if (!strcmp(pwd->pw_shell, JCLCSH)) {
		ldisc = NTTYDISC;
		ioctl(0, TIOCSETD, &ldisc);
	} else {
		ltc.t_suspc = ltc.t_dsuspc = UNDEF;
		ioctl(0, TIOCSLTC, &ltc);
	}
#endif
	environ = envinit;
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	strncat(user, pwd->pw_name, sizeof(user)-6);
	strncat(shell, pwd->pw_shell, sizeof(shell)-7);

	signal(SIGINT, SIG_IGN);
	alarm(0);
	quietlog = 0;
	if (access(qlog, 0) == 0)
		quietlog = 1;
/*
 * Update lastlog file
 * Tell user when he last logged in
 */
	if((f = open(lastlog, 2)) >= 0) {
		struct lastlog ll;

		lseek(f, (long) ((unsigned) (pwd->pw_uid) * sizeof (struct lastlog)), 0);
		if (read(f, (char *) &ll, sizeof ll) == sizeof ll && ll.ll_time != 0 && quietlog == 0) {
			register char *ep = (char *) ctime(&ll.ll_time);
			printf("Last login: ");
			ep[24 - 5] = 0;
			printf("%s ", ep);
			if (*ll.ll_host != '\0')
				printf("from %.*s\n", (sizeof ll.ll_host),
						      ll.ll_host);
			else
				printf("on %.*s\n", LMAX, ll.ll_line);
			fflush(stdout);	/* So user sees the message quickly! */
		}
		lseek(f, (long) ((unsigned) (pwd->pw_uid) * sizeof (struct lastlog)), 0);
		time(&ll.ll_time);
		strncpy(ll.ll_line, ttyn+5, LMAX);
		SCPYN(ll.ll_host, rhost);
		write(f, (char *) &ll, sizeof ll);
		close(f);
	}
	if (!quietlog)
		showfile("/etc/motd");
	namep = pwd->pw_dir;
	for (;;) {
		if (*namep == '\0')
			break;
		cp = namep++;
		for (; *namep != '/' && *namep != '\0'; namep++);
		wasslash = 0;
		if (*namep == '/') {
			*namep = '\0';
			wasslash++;
		}
		if (chdir(cp)<0) {
			if (chdir("/") < 0) {
			    printf("No directory!\n");
			    exit(1);
			} else {
			    printf("No directory!  Logging in with home=/\n");
			    break;
			}
		}
		showfile(".broadcast");
		if (wasslash)
			*namep++ = '/';
	}
	showfile(".reminder");

	if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;
	strcat(minusnam, namep);
	umask(UMASK);
	if (!quietlog) {
		strcat(maildir, pwd->pw_name);
		if(access(maildir,4)==0) {
			struct stat statb;
			stat(maildir, &statb);
			if (statb.st_size)
				printf("You have mail.\n");
		}
	}
	signal(SIGALRM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
#ifdef	SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif
	fflush(stdout);
	execlp(pwd->pw_shell, minusnam, 0);
	printf("No shell\n");
	sleep(3);
	exit(0);
}

int	stopmotd;
catch()
{
	signal(SIGINT, SIG_IGN);
	stopmotd++;
	printf("\n");
	fflush(stdout);		/* Immediate-looking response. */
}

showfile(name)
char *name;
{
	FILE *mf;
	register c;

	stopmotd = 0;
	signal(SIGINT, catch);
	if((mf = fopen(name,"r")) != NULL) {
		while((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
		fflush(stdout);
	}
	signal(SIGINT, SIG_IGN);
}

/*
 * make a reasonable guess as to the kind of terminal the user is on.
 * We look in /etc/ttytype for this info (format: each line has two
 * words, first word is a term type, second is a tty name), and default
 * to "unknown" if we can't find any better.  In the case of dialups we get
 * names like "dialup" which is a lousy guess but tset can
 * take it from there.
 */

getterm()
{
	register char	*sp, *tname;
	register int	i;
	register FILE	*fdes;
	char		*type, *t;
	char		ttline[64];

	if ((fdes = fopen("/etc/ttytype", "r")) == NULL) {
unknown:
		strcat(term, "unknown");
		return;
	}
	for (tname = ttyn; *tname++; )
		;
	while (*--tname != '/')
		;
	tname++;
	while (fgets(ttline, sizeof(ttline), fdes) != NULL) {
		ttline[strlen(ttline)-1] = 0;	/* zap \n on end */
		type = ttline;
		for (t=ttline; *t && *t!=' ' && *t != '\t'; t++)
			;
		*t++ = 0;
		/* Now have term and type pointing to the right guys */
		if (strcmp(t, tname) == 0) {
			strcat(term, type);
			fclose(fdes);
			return;
		}
	}
	fclose(fdes);
	goto unknown;
}

getloginname(up)
	register struct utmp *up;
{
	register char *namep;
	char c;

	while (up->ut_name[0] == '\0') {
		namep = up->ut_name;
		printf("%s login: ", hostname);
		fflush(stdout);
		while ((c = getchar()) != '\n') {
			if (c == ' ')
				c = '_';
			if (c == EOF)
				exit(0);
#ifdef	GROUP
			if (c == '.')
				break;
#endif
			if (namep < up->ut_name+NMAX)
				*namep++ = c;
		}
#ifdef	GROUP
		if (c == '.') {
			char *pgrp = group;
			while ((c = getchar()) != '\n') {
				if (c == EOF)
					exit(0);
				if (pgrp < &group[GRLEN])
					*pgrp++ = c;
			}
		}
#endif
	}
	strncpy(lusername, up->ut_name, NMAX);
	lusername[NMAX] = 0;
	setpwent();
	if ((pwd = getpwnam(lusername)) == NULL)
		pwd = &nouser;
	endpwent();
}

timedout()
{

	printf("Login timed out after %d seconds\n", timeout);
	fflush(stdout);
	exit(0);
}

remotelogin(host)
	char *host;
{
	FILE *hostf;
	int first = 1;

	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term+5, sizeof(term)-5, "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		goto bad;
	}
	setpwent();
	pwd = getpwnam(lusername);
	endpwent();
	if (pwd == NULL) {
		pwd = &nouser;
		goto bad;
	}
	hostf = pwd->pw_uid ? fopen("/etc/hosts.equiv", "r") : 0;
again:
	if (hostf) {
		char ahost[32];

		while (fgets(ahost, sizeof (ahost), hostf)) {
			char *user;

			if ((user = index(ahost, '\n')) != 0)
				*user++ = '\0';
			if ((user = index(ahost, ' ')) != 0)
				*user++ = '\0';
			if (!strcmp(host, ahost) &&
			    !strcmp(rusername, user ? user : lusername)) {
				fclose(hostf);
				return (1);
			}
		}
		fclose(hostf);
	}
	if (first == 1) {
		char *rhosts = ".rhosts";
		struct stat sbuf;

		first = 0;
		if (chdir(pwd->pw_dir) < 0)
			goto again;
		if (lstat(rhosts, &sbuf) < 0)
			goto again;
		if ((sbuf.st_mode & S_IFMT) == S_IFLNK) {
			printf("login: .rhosts is a soft link.\r\n");
			goto bad;
		}
		hostf = fopen(rhosts, "r");
		fstat(fileno(hostf), &sbuf);
		if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
			printf("login: Bad .rhosts ownership.\r\n");
			fclose(hostf);
			goto bad;
		}
		goto again;
	}
bad:
	return (-1);
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

remoteterm(term, tp)
	char *term;
	struct sgttyb *tp;
{
	char *cp = index(term, '/');
	register int i;

	if (cp) {
		*cp++ = 0;
		for (i = 0; i < NSPEEDS; i++)
			if (!strcmp(speeds[i], cp)) {
				tp->sg_ispeed = tp->sg_ospeed = i;
				break;
			}
	}
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}

logerr(fmt, a1, a2, a3, a4)
	char *fmt, *a1, *a2, *a3, *a4;
{
#ifdef LOGERR
	FILE *cons = fopen("/dev/console", "w");

	if (cons != NULL) {
		fprintf(cons, fmt, a1, a2, a3, a4);
		fprintf(cons, "\n\r");
		fflush(cons);
		fclose(cons);
	}
#endif
}
