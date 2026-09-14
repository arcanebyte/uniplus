static char sccsid[] = "@(#)telnet.c	4.11 (Berkeley) 10/7/82";
/*
 * User telnet program.
 *
 * Lisa (UniPlus+ with unix.net), ported from 2.9BSD's netser/telnet:
 *  - select() can't wait on a terminal here, so a connection is served by
 *    two processes, as in netlib/nc.  The parent reads the network,
 *    answers option negotiation and writes the terminal; a child reads
 *    the keyboard, writes the network and runs the escape commands, and
 *    tells the parent about close, quit and options through a pipe and
 *    SIGUSR1.
 *  - Terminal modes use System V termio instead of sgtty.  There is no
 *    job control, so no "z" command, and no shutdown(), so close just
 *    closes.
 *  - Host names come from the name server or /etc/hosts through libnetdb.a.
 *  - End of line as 4.3BSD sends it: newline as CR LF, carriage return
 *    as CR NUL, and Return not mapped to newline while the remote side
 *    echoes.  4.1c sent a bare newline.
 */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <termio.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/in.h>
#include <netdb.h>
#define	connected	cnctd
#define	TELOPTS
#include "telnet.h"

#define	ctrl(x)		((x) & 037)
#define	strip(x)	((x)&0177)

char	ttyobuf[BUFSIZ], *tfrontp = ttyobuf, *tbackp = ttyobuf;
char	netobuf[BUFSIZ], *nfrontp = netobuf, *nbackp = netobuf;

char	hisopts[256];
char	myopts[256];

char	doopt[] = { IAC, DO, '%', 'c', 0 };
char	dont[] = { IAC, DONT, '%', 'c', 0 };
char	will[] = { IAC, WILL, '%', 'c', 0 };
char	wont[] = { IAC, WONT, '%', 'c', 0 };

int	connected;
int	net;
int	showoptions;
int	options;
char	*prompt;
char	escape = ctrl(']');

char	line[200];
int	margc;
char	*margv[20];

jmp_buf	toplevel;

extern	int errno;

int	tn(), quit(), bye(), help();
int	setescape(), status(), setoptions();

#define HELPINDENT (sizeof ("connect"))

struct cmd {
	char	*name;
	char	*help;
	int	(*handler)();
};

char	ohelp[] = "connect to a site";
char	chelp[] = "close current connection";
char	qhelp[] = "exit telnet";
char	ehelp[] = "set escape character";
char	shelp[] = "print status information";
char	hhelp[] = "print help information";
char	phelp[] = "toggle viewing of options processing";

struct cmd cmdtab[] = {
	{ "open",	ohelp,		tn },
	{ "close",	chelp,		bye },
	{ "quit",	qhelp,		quit },
	{ "escape",	ehelp,		setescape },
	{ "status",	shelp,		status },
	{ "options",	phelp,		setoptions },
	{ "?",		hhelp,		help },
	0
};

struct sockaddr_in sin = { AF_INET };

int	intr(), fromkbd();
char	*control();
struct	cmd *getcmd();
struct	servent *sp;

struct	termio otermio;		/* the terminal as we found it */
int	inchild;		/* this is the keyboard process */
int	kbdpid;			/* the keyboard process, in the parent */
int	cmdpipe[2];		/* keyboard process to parent: 'c', 'q', 'o' */

main(argc, argv)
	int argc;
	char *argv[];
{
	sp = getservbyname("telnet", "tcp");
	if (sp == 0) {
		fprintf(stderr, "telnet: tcp/telnet: unknown service\n");
		exit(1);
	}
	ioctl(0, TCGETA, (char *)&otermio);
	setbuf(stdin, (char *)0);
	setbuf(stdout, (char *)0);
	prompt = argv[0];
	if (argc > 1 && !strcmp(argv[1], "-d"))
		options = SO_DEBUG, argv++, argc--;
	if (argc != 1) {
		if (setjmp(toplevel) != 0)
			exit(0);
		tn(argc, argv);
	}
	setjmp(toplevel);
	for (;;)
		command(1);
}

char	*hostname;
char	hnamebuf[32];

tn(argc, argv)
	int argc;
	char *argv[];
{
	register struct hostent *host;

	if (connected) {
		printf("?Already connected to %s\n", hostname);
		return;
	}
	if (argc < 2) {
		strcpy(line, "Connect ");
		printf("(to) ");
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc > 3) {
		printf("usage: %s host-name [port]\n", argv[0]);
		return;
	}
	host = gethostbyname(argv[1]);
	if (host) {
		bcopy(host->h_addr, (char *)&sin.sin_addr, host->h_length);
		strncpy(hnamebuf, host->h_name, sizeof (hnamebuf) - 1);
	} else {
		sin.sin_addr.s_addr = inet_addr(argv[1]);
		if (sin.sin_addr.s_addr == -1) {
			printf("%s: unknown host\n", argv[1]);
			return;
		}
		strncpy(hnamebuf, argv[1], sizeof (hnamebuf) - 1);
	}
	hostname = hnamebuf;
	sin.sin_port = sp->s_port;
	if (argc == 3) {
		sin.sin_port = atoi(argv[2]);
		if (sin.sin_port <= 0) {
			printf("%s: bad port number\n", argv[2]);
			return;
		}
	}
	sin.sin_port = htons(sin.sin_port);
	if ((net = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)0, options)) < 0) {
		perror("socket");
		return;
	}
	signal(SIGINT, intr);
	printf("Trying...\n");
	if (connect(net, (struct sockaddr *)&sin) < 0) {
		perror("connect");
		signal(SIGINT, SIG_DFL);
		close(net);
		return;
	}
	connected++;
	status();
	telnet(net);
	fprintf(stderr, "Connection closed by foreign host.\n");
	exit(1);
}

/*
 * Print status about the connection.
 */
/*VARARGS*/
status()
{
	if (connected)
		printf("Connected to %s.\n", hostname);
	else
		printf("No connection.\n");
	printf("Escape character is '%s'.\n", control(escape));
}

makeargv()
{
	register char *cp;
	register char **argp = margv;

	margc = 0;
	for (cp = line; *cp;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;
}

/*
 * Tell the parent about a command run in the keyboard process.
 */
tellparent(c)
	char c;
{
	write(cmdpipe[1], &c, 1);
	kill(getppid(), SIGUSR1);
}

/*VARARGS*/
bye()
{
	if (inchild) {
		printf("Connection closed.\n");
		tellparent('c');
		exit(0);
	}
}

/*VARARGS*/
quit()
{
	if (inchild) {
		tellparent('q');
		exit(0);
	}
	exit(0);
}

/*
 * Help command.
 * Call each command handler with argc == 0 and argv[0] == name.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;

	if (argc == 1) {
		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c->name; c++)
			printf("%-*s\t%s\n", HELPINDENT, c->name, c->help);
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == (struct cmd *)0)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%s\n", c->help);
	}
}

/*
 * Terminal modes: 0 as we found it, 1 characters at a time without
 * echo (the remote side echoes), 2 characters at a time with local echo.
 * Interrupt and quit characters go to the remote side in 1 and 2.
 * In 1, Return stays a carriage return (4.2BSD turned CRMOD off here);
 * output still maps newline to CR LF.
 */
mode(f)
	register int f;
{
	static int prevmode = 0;
	struct termio t;
	int old;

	if (prevmode == f)
		return (f);
	old = prevmode;
	prevmode = f;
	t = otermio;
	if (f != 0) {
		t.c_lflag &= ~(ICANON|ISIG);
		if (f == 1) {
			t.c_lflag &= ~ECHO;
			t.c_iflag &= ~ICRNL;
		} else
			t.c_lflag |= ECHO;
		t.c_cc[VMIN] = 1;
		t.c_cc[VTIME] = 0;
	}
	ioctl(0, TCSETAW, (char *)&t);
	return (old);
}

char	sibuf[BUFSIZ], *sbp;
char	tibuf[BUFSIZ];
char	tobuf[2 * BUFSIZ];
int	scc;

/*
 * Serve the connection: this process reads the network, a child the
 * keyboard.
 */
telnet(s)
	int s;
{
	(void) mode(2);
	if (pipe(cmdpipe) < 0) {
		perror("pipe");
		return;
	}
	signal(SIGINT, SIG_IGN);	/* the terminal sends ^C to the remote side */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, fromkbd);
	if ((kbdpid = fork()) < 0) {
		perror("fork");
		return;
	}
	if (kbdpid == 0) {
		inchild = 1;
		close(cmdpipe[0]);
		signal(SIGUSR1, SIG_DFL);
		keyboard(s);
		exit(0);
	}
	close(cmdpipe[1]);
	for (;;) {
		scc = read(s, sibuf, sizeof (sibuf));
		if (scc < 0 && errno == EINTR)
			continue;
		if (scc <= 0)
			break;
		sbp = sibuf;
		telrcv();
		if (netflush(s) < 0)
			break;
		ttyflush(1);
	}
	endkbd();
	(void) mode(0);
}

/*
 * The keyboard process: copy the terminal to the network until the
 * escape character.  As in 4.3BSD, a newline goes out as CR LF and a
 * carriage return as CR NUL, the telnet end of line (RFC 854); 4.1c
 * sent a bare newline, which servers such as telehack.com ignore.
 */
keyboard(s)
	int s;
{
	register char *p, *q;
	register int n;
	int zeros = 0;

	for (;;) {
		n = read(0, tibuf, sizeof (tibuf));
		if (n < 0 && errno == EINTR)
			continue;
		/*
		 * A read that was waiting when the parent changed the terminal
		 * mode can come back empty, so only a run of them is the end.
		 */
		if (n == 0 && ++zeros < 3)
			continue;
		if (n <= 0) {
			tellparent('q');
			return;
		}
		zeros = 0;
		for (p = tibuf, q = tobuf; n > 0; n--, p++) {
			if (strip(*p) == escape) {
				if (q > tobuf && write(s, tobuf, q - tobuf) < 0)
					return;
				command(0);
				q = tobuf;
				break;
			}
			if (*p == '\n') {
				*q++ = '\r';
				*q++ = '\n';
			} else if (*p == '\r') {
				*q++ = '\r';
				*q++ = '\0';
			} else
				*q++ = *p;
		}
		if (q > tobuf && write(s, tobuf, q - tobuf) < 0)
			return;
	}
}

/*
 * SIGUSR1 in the parent: the keyboard process ran close, quit or options.
 */
fromkbd()
{
	char c;

	signal(SIGUSR1, fromkbd);
	if (read(cmdpipe[0], &c, 1) != 1)
		return;
	if (c == 'o') {
		showoptions = !showoptions;
		return;
	}
	endkbd();
	(void) mode(0);
	close(net);
	connected = 0;
	bzero(hisopts, sizeof (hisopts));
	bzero(myopts, sizeof (myopts));
	if (c == 'q')
		exit(0);
	signal(SIGINT, SIG_DFL);
	longjmp(toplevel, 1);
}

endkbd()
{
	if (kbdpid > 0) {
		kill(kbdpid, SIGKILL);
		while (wait((int *)0) != kbdpid && errno != ECHILD)
			;
		kbdpid = 0;
	}
	close(cmdpipe[0]);
}

command(top)
	int top;
{
	register struct cmd *c;
	struct termio saved;

	if (top) {
		(void) mode(0);
		signal(SIGINT, SIG_DFL);
	} else {
		ioctl(0, TCGETA, (char *)&saved);
		ioctl(0, TCSETAW, (char *)&otermio);
		putchar('\n');
	}
	for (;;) {
		printf("%s> ", prompt);
		if (gets(line) == 0)
			break;
		if (line[0] == 0)
			break;
		makeargv();
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == 0) {
			printf("?Invalid command\n");
			continue;
		}
		(*c->handler)(margc, margv);
		if (c->handler != help)
			break;
	}
	if (!top)
		ioctl(0, TCSETAW, (char *)&saved);
}

/*
 * Telnet receiver states for fsm
 */
#define	TS_DATA		0
#define	TS_IAC		1
#define	TS_WILL		2
#define	TS_WONT		3
#define	TS_DO		4
#define	TS_DONT		5

telrcv()
{
	register int c;
	static int state = TS_DATA;

	while (scc > 0) {
		c = *sbp++ & 0377, scc--;
		switch (state) {

		case TS_DATA:
			if (c == IAC)
				state = TS_IAC;
			else
				*tfrontp++ = c;
			continue;

		case TS_IAC:
			switch (c) {

			case WILL:
				state = TS_WILL;
				continue;

			case WONT:
				state = TS_WONT;
				continue;

			case DO:
				state = TS_DO;
				continue;

			case DONT:
				state = TS_DONT;
				continue;

			case DM:
				ioctl(1, TCFLSH, 1);
				break;

			case NOP:
			case GA:
				break;

			default:
				break;
			}
			state = TS_DATA;
			continue;

		case TS_WILL:
			printoption("RCVD", will, c, !hisopts[c]);
			if (!hisopts[c])
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			printoption("RCVD", wont, c, hisopts[c]);
			if (hisopts[c])
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			printoption("RCVD", doopt, c, !myopts[c]);
			if (!myopts[c])
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			printoption("RCVD", dont, c, myopts[c]);
			if (myopts[c]) {
				myopts[c] = 0;
				sprintf(nfrontp, wont, c);
				nfrontp += sizeof (wont) - 2;
				printoption("SENT", wont, c);
			}
			state = TS_DATA;
			continue;
		}
	}
}

willoption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_ECHO:
		(void) mode(1);

	case TELOPT_SGA:
		hisopts[option] = 1;
		fmt = doopt;
		break;

	case TELOPT_TM:
		fmt = dont;
		break;

	default:
		fmt = dont;
		break;
	}
	sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (dont) - 2;
	printoption("SENT", fmt, option);
}

wontoption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_ECHO:
		(void) mode(2);

	case TELOPT_SGA:
		hisopts[option] = 0;
		fmt = dont;
		break;

	default:
		fmt = dont;
	}
	sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
	printoption("SENT", fmt, option);
}

dooption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_SGA:
		fmt = will;
		break;

	/*
	 * fix from allegra!jdd for 3Com UNET uvtp servers
	 */
	case TELOPT_ECHO:
		(void) mode(2);
		fmt = will;
		hisopts[option] = 0;
		break;

	default:
		fmt = wont;
		break;
	}
	sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
	printoption("SENT", fmt, option);
}

/*
 * Set the escape character.
 */
setescape(argc, argv)
	int argc;
	char *argv[];
{
	register char *arg;
	char buf[50];

	if (argc > 2)
		arg = argv[1];
	else {
		printf("new escape character: ");
		gets(buf);
		arg = buf;
	}
	if (arg[0] != '\0')
		escape = arg[0];
	printf("Escape character is '%s'.\n", control(escape));
}

/*VARARGS*/
setoptions()
{
	showoptions = !showoptions;
	printf("%s show option processing.\n", showoptions ? "Will" : "Wont");
	if (inchild)
		tellparent('o');
}

/*
 * Construct a control character sequence
 * for a special character.
 */
char *
control(c)
	register int c;
{
	static char buf[3];

	if (c == 0177)
		return ("^?");
	if (c >= 040) {
		buf[0] = c;
		buf[1] = 0;
	} else {
		buf[0] = '^';
		buf[1] = '@'+c;
		buf[2] = 0;
	}
	return (buf);
}

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->name; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

intr()
{
	signal(SIGINT, intr);
	(void) mode(0);
	longjmp(toplevel, -1);
}

ttyflush(fd)
{
	register int n;

	while ((n = tfrontp - tbackp) > 0) {
		if ((n = write(fd, tbackp, n)) <= 0)
			break;
		tbackp += n;
	}
	tbackp = tfrontp = ttyobuf;
}

netflush(fd)
{
	register int n;

	while ((n = nfrontp - nbackp) > 0) {
		if ((n = write(fd, nbackp, n)) < 0) {
			if (errno == EINTR)
				continue;
			(void) mode(0);
			perror(hostname);
			return (-1);
		}
		nbackp += n;
	}
	nbackp = nfrontp = netobuf;
	return (0);
}

/*VARARGS*/
printoption(direction, fmt, option, what)
	char *direction, *fmt;
	int option, what;
{
	if (!showoptions)
		return;
	printf("%s ", direction);
	if (fmt == doopt)
		fmt = "do";
	else if (fmt == dont)
		fmt = "dont";
	else if (fmt == will)
		fmt = "will";
	else if (fmt == wont)
		fmt = "wont";
	else
		fmt = "???";
	if (option < TELOPT_SUPDUP)
		printf("%s %s", fmt, telopts[option]);
	else
		printf("%s %d", fmt, option);
	if (*direction == '<') {
		printf("\r\n");
		return;
	}
	printf(" (%s)\r\n", what ? "reply" : "don't reply");
}
