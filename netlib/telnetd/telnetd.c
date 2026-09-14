#ifndef lint
static char sccsid[] = "@(#)telnetd.c	4.9 82/10/10";
#endif

/*
 * Stripped-down telnet server.
 *
 * Lisa (UniPlus+ with unix.net), ported from 2.9BSD's netser/telnet:
 *  - The pseudo-terminals are the kernel's pty.c: /dev/ptyp0-f (major 20)
 *    and /dev/ttyp0-f (major 21); mkptys makes the device files.
 *  - System V's login only runs for a process that already has a utmp
 *    entry, as getty leaves, so the child writes a LOGIN_PROCESS entry
 *    for its pseudo-terminal before exec'ing /bin/login.  The child calls
 *    setpgrp() first, so the pseudo-terminal becomes its controlling
 *    terminal (System V has no TIOCNOTTY).
 *  - Terminal modes use termio on the pty (TCSETA, which unlike TCSETAW
 *    doesn't make pty.c flush output).
 *  - The end of a session is SIGCLD from the login process: a pty's
 *    controlling side doesn't see end of file here.  At the end the
 *    utmp entry is marked DEAD_PROCESS and a wtmp record is added.
 *  - 4.1a sockets: accept() connects the listening socket itself, so a
 *    new listening socket is made for each connection.
 */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/in.h>
#include <netdb.h>
#include <utmp.h>
#include "telnet.h"

#define	INFINITY	((long)10000000)
#define	BELL		'\07'

/* sgtty-style mode bits for mode(), mapped onto termio there */
#define	M_ECHO		01
#define	M_CRMOD		02
#define	M_RAW		04

char	hisopts[256];
char	myopts[256];

char	doopt[] = { IAC, DO, '%', 'c', 0 };
char	dont[] = { IAC, DONT, '%', 'c', 0 };
char	will[] = { IAC, WILL, '%', 'c', 0 };
char	wont[] = { IAC, WONT, '%', 'c', 0 };

/*
 * I/O data buffers, pointers, and counters.
 */
char	ptyibuf[BUFSIZ], *ptyip = ptyibuf;
char	ptyobuf[BUFSIZ], *pfrontp = ptyobuf, *pbackp = ptyobuf;
char	netibuf[BUFSIZ], *netip = netibuf;
char	netobuf[BUFSIZ], *nfrontp = netobuf, *nbackp = netobuf;
int	pcc, ncc;

int	pty, net;
int	inter;
int	loginpid;		/* the login process, in the connection's server */
extern	int errno;
char	line[] = "/dev/ptyp0";

struct	sockaddr_in sin = { AF_INET };
int	options = SO_ACCEPTCONN|SO_KEEPALIVE;

main(argc, argv)
	char *argv[];
{
	int s, pid;
	struct servent *sp;
	struct sockaddr_in from;

	sp = getservbyname("telnet", "tcp");
	if (sp == 0) {
		fprintf(stderr, "telnetd: tcp/telnet: unknown service\n");
		exit(1);
	}
	sin.sin_port = sp->s_port;
	argc--, argv++;
	if (argc > 0 && !strcmp(argv[0], "-d"))
		options |= SO_DEBUG, argc--, argv++;
	if (argc > 0) {
		sin.sin_port = atoi(*argv);
		if (sin.sin_port <= 0) {
			fprintf(stderr, "telnetd: %s: bad port #\n", *argv);
			exit(1);
		}
	}
	sin.sin_port = htons(sin.sin_port);
#ifndef DEBUG
	if (fork())
		exit(0);
	for (s = 0; s < 10; s++)
		(void) close(s);
	(void) open("/", 0);
	(void) dup(0);
	(void) dup(0);
	setpgrp();		/* no controlling terminal */
#endif
	signal(SIGCLD, SIG_IGN);	/* System V: children leave no zombies */
	signal(SIGHUP, SIG_IGN);
	for (;;) {
		errno = 0;
		if ((s = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)&sin, options)) < 0) {
			perror("socket");
			sleep(5);
			continue;
		}
		if (accept(s, (struct sockaddr *)&from) < 0) {
			perror("accept");
			close(s);
			sleep(1);
			continue;
		}
		if ((pid = fork()) < 0)
			printf("Out of processes\n");
		else if (pid == 0)
			doit(s);
		close(s);
	}
	/*NOTREACHED*/
}

int	cleanup();

/*
 * Get a pty, scan input lines.
 */
doit(f)
{
	char *cp = line;
	int i, p, t;

	for (i = 0; i < 16; i++) {
		cp[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
		p = open(cp, 2);
		if (p > 0)
			goto gotpty;
	}
	write(f, "All network ports in use.\r\n", 27);
	exit(1);
gotpty:
	cp[strlen("/dev/")] = 't';
	signal(SIGCLD, cleanup);
	if ((i = fork()) < 0) {
		write(f, "telnetd: out of processes\r\n", 27);
		exit(1);
	}
	if (i) {
		loginpid = i;
		telnet(f, p);
	}
	close(f);
	close(p);
	setpgrp();		/* the pty becomes the controlling terminal */
	for (t = 0; t < 10; t++)
		(void) close(t);
	t = open(cp, 2);
	if (t != 0)
		exit(1);
	(void) dup(0);
	(void) dup(0);
	ttymodes(0);
	loginutmp(cp + strlen("/dev/"));
	signal(SIGCLD, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	execl("/bin/login", "login", (char *)0);
	perror("/bin/login");
	exit(1);
}

/*
 * The pseudo-terminal's modes for login: canonical input with echo (the
 * server echoes), CR to NL on input and NL to CR NL on output.
 */
ttymodes(fd)
{
	struct termio t;

	ioctl(fd, TCGETA, (char *)&t);
	t.c_iflag = ICRNL|IXON;
	t.c_oflag = OPOST|ONLCR|TAB3;
	t.c_cflag = B9600|CS8|CREAD|HUPCL;
	t.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
	t.c_cc[VEOF] = CEOF;
	t.c_cc[VEOL] = 0;
	ioctl(fd, TCSETA, (char *)&t);
}

/*
 * System V login wants an entry for its process, as getty would leave.
 */
loginutmp(tty)
	char *tty;
{
	struct utmp u;

	bzero((char *)&u, sizeof (u));
	strncpy(u.ut_user, "LOGIN", sizeof (u.ut_user));
	u.ut_id[0] = 'p';
	u.ut_id[1] = tty[strlen(tty) - 1];
	strncpy(u.ut_line, tty, sizeof (u.ut_line));
	u.ut_pid = getpid();
	u.ut_type = LOGIN_PROCESS;
	time(&u.ut_time);
	setutent();
	pututline(&u);
	endutent();
}

/*
 * Main loop.  Select from pty and network, and
 * hand data to telnet receiver finite state machine.
 */
telnet(f, p)
{
	int on = 1;

	net = f, pty = p;
	ioctl(f, FIONBIO, (char *)&on);
	ioctl(p, FIONBIO, (char *)&on);

	/*
	 * Request to do remote echo.
	 */
	dooption(TELOPT_ECHO);
	myopts[TELOPT_ECHO] = 1;
	for (;;) {
		long ibits = 0, obits = 0;
		register int c;

		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if (nfrontp - nbackp)
			obits |= (1 << f);
		else
			ibits |= (1 << p);
		if (pfrontp - pbackp)
			obits |= (1 << p);
		else
			ibits |= (1 << f);
		if (ncc < 0 && pcc < 0)
			break;
		if (select(32, &ibits, &obits, INFINITY) < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (ibits == 0 && obits == 0) {
			sleep(5);
			continue;
		}

		/*
		 * Something to read from the network...
		 */
		if (ibits & (1 << f)) {
			ncc = read(f, netibuf, BUFSIZ);
			if (ncc < 0 && (errno == EWOULDBLOCK || errno == EINTR))
				ncc = 0;
			else {
				if (ncc <= 0)
					break;
				netip = netibuf;
			}
		}

		/*
		 * Something to read from the pty...
		 */
		if (ibits & (1 << p)) {
			pcc = read(p, ptyibuf, BUFSIZ);
			if (pcc < 0 && (errno == EWOULDBLOCK || errno == EINTR))
				pcc = 0;
			else {
				if (pcc <= 0)
					break;
				ptyip = ptyibuf;
			}
		}

		while (pcc > 0) {
			if ((&netobuf[BUFSIZ] - nfrontp) < 2)
				break;
			c = *ptyip++ & 0377, pcc--;
			if (c == IAC)
				*nfrontp++ = c;
			*nfrontp++ = c;
		}
		if ((obits & (1 << f)) && (nfrontp - nbackp) > 0)
			netflush();
		if (ncc > 0)
			telrcv();
		if ((obits & (1 << p)) && (pfrontp - pbackp) > 0)
			ptyflush();
	}
	cleanup();
}

/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_BEGINNEG	3	/* throw away begin's... */
#define	TS_ENDNEG	4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

telrcv()
{
	register int c;
	static int state = TS_DATA;
	struct termio b;

	while (ncc > 0) {
		if ((&ptyobuf[BUFSIZ] - pfrontp) < 2)
			return;
		c = *netip++ & 0377, ncc--;
		switch (state) {

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			if (inter > 0)
				break;
			*pfrontp++ = c;
			if (!myopts[TELOPT_BINARY] && c == '\r')
				state = TS_CR;
			break;

		case TS_CR:
			if (c && c != '\n')
				*pfrontp++ = c;
			state = TS_DATA;
			break;

		case TS_IAC:
			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case BREAK:
			case IP:
				interrupt();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
				*pfrontp++ = BELL;
				break;

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL:
				ptyflush();	/* half-hearted */
				ioctl(pty, TCGETA, (char *)&b);
				*pfrontp++ = (c == EC) ?
					b.c_cc[VERASE] : b.c_cc[VKILL];
				break;

			/*
			 * Check for urgent data...
			 */
			case DM:
				break;

			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_BEGINNEG;
				continue;

			case WILL:
			case WONT:
			case DO:
			case DONT:
				state = TS_WILL + (c - WILL);
				continue;

			case IAC:
				*pfrontp++ = c;
				break;
			}
			state = TS_DATA;
			break;

		case TS_BEGINNEG:
			if (c == IAC)
				state = TS_ENDNEG;
			break;

		case TS_ENDNEG:
			state = c == SE ? TS_DATA : TS_BEGINNEG;
			break;

		case TS_WILL:
			if (!hisopts[c])
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			if (hisopts[c])
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			if (!myopts[c])
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			if (myopts[c]) {
				myopts[c] = 0;
				sprintf(nfrontp, wont, c);
				nfrontp += sizeof (wont) - 2;
			}
			state = TS_DATA;
			continue;

		default:
			printf("netser: panic state=%d\n", state);
			exit(1);
		}
	}
}

willoption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_BINARY:
		mode(M_RAW, 0);
		goto common;

	case TELOPT_ECHO:
		mode(0, M_ECHO|M_CRMOD);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
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
}

wontoption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_ECHO:
		mode(M_ECHO|M_CRMOD, 0);
		goto common;

	case TELOPT_BINARY:
		mode(0, M_RAW);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		hisopts[option] = 0;
		fmt = dont;
		break;

	default:
		fmt = dont;
	}
	sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
}

dooption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_ECHO:
		mode(M_ECHO|M_CRMOD, 0);
		goto common;

	case TELOPT_BINARY:
		mode(M_RAW, 0);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		fmt = will;
		break;

	default:
		fmt = wont;
		break;
	}
	sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
}

/*
 * Turn sgtty-style ECHO, CRMOD and RAW on and off in the pty's termio
 * modes.  RAW keeps the canonical-mode EOF and EOL characters to put
 * back, as the same slots hold MIN and TIME without ICANON.
 */
mode(on, off)
	int on, off;
{
	struct termio b;
	static char eof = CEOF, eol = 0;

	ptyflush();
	ioctl(pty, TCGETA, (char *)&b);
	if (on & M_ECHO)
		b.c_lflag |= ECHO;
	if (off & M_ECHO)
		b.c_lflag &= ~ECHO;
	if (on & M_CRMOD) {
		b.c_iflag |= ICRNL;
		b.c_oflag |= ONLCR;
	}
	if (off & M_CRMOD) {
		b.c_iflag &= ~ICRNL;
		b.c_oflag &= ~ONLCR;
	}
	if ((on & M_RAW) && (b.c_lflag & ICANON)) {
		eof = b.c_cc[VEOF];
		eol = b.c_cc[VEOL];
		b.c_lflag &= ~(ICANON|ISIG);
		b.c_iflag &= ~(ICRNL|IXON|ISTRIP);
		b.c_oflag &= ~OPOST;
		b.c_cc[VMIN] = 1;
		b.c_cc[VTIME] = 0;
	}
	if ((off & M_RAW) && !(b.c_lflag & ICANON)) {
		b.c_lflag |= ICANON|ISIG;
		b.c_iflag |= IXON;
		b.c_oflag |= OPOST;
		b.c_cc[VEOF] = eof;
		b.c_cc[VEOL] = eol;
	}
	ioctl(pty, TCSETA, (char *)&b);
}

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
interrupt()
{
	struct termio b;

	ptyflush();	/* half-hearted */
	if (ioctl(pty, TCGETA, (char *)&b) < 0) {
		*pfrontp++ = '\177';
		return;
	}
	if ((b.c_lflag & ISIG) == 0) {
		*pfrontp++ = '\0';
		return;
	}
	*pfrontp++ = b.c_cc[VINTR];
}

ptyflush()
{
	int n;

	if ((n = pfrontp - pbackp) > 0)
		n = write(pty, pbackp, n);
	if (n < 0)
		return;
	pbackp += n;
	if (pbackp == pfrontp)
		pbackp = pfrontp = ptyobuf;
}

netflush()
{
	int n;

	if ((n = nfrontp - nbackp) > 0)
		n = write(net, nbackp, n);
	if (n < 0) {
		if (errno == EWOULDBLOCK)
			return;
		/* should blow this guy away... */
		return;
	}
	nbackp += n;
	if (nbackp == nfrontp)
		nbackp = nfrontp = netobuf;
}

cleanup()
{
	signal(SIGCLD, SIG_IGN);
	rmut();
	if (loginpid > 0)
		kill(-loginpid, SIGHUP);	/* its process group */
	close(net);
	exit(1);
}

/*
 * Mark the session's utmp entry dead and log the logout in wtmp.
 */
rmut()
{
	struct utmp *u, key;
	register f;

	bzero((char *)&key, sizeof (key));
	strncpy(key.ut_line, line + strlen("/dev/"), sizeof (key.ut_line));
	setutent();
	if ((u = getutline(&key)) != NULL) {
		u->ut_type = DEAD_PROCESS;
		u->ut_exit.e_termination = 0;
		u->ut_exit.e_exit = 0;
		time(&u->ut_time);
		pututline(u);
		f = open(WTMP_FILE, 1);
		if (f >= 0) {
			lseek(f, (long)0, 2);
			write(f, (char *)u, sizeof (*u));
			close(f);
		}
	}
	endutent();
	chmod(line, 0666);
	chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	chmod(line, 0666);
	chown(line, 0, 0);
}
