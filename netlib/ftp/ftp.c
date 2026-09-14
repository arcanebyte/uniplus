#ifndef lint
static char sccsid[] = "@(#)ftp.c	4.1 (Berkeley) 1/15/83";
#endif
/*
 * Lisa (UniPlus+ with unix.net), from 2.9BSD's netser/ftp: the 4.1a
 * socket calls only (TCP4_1a); passive mode, where ftp connects to the
 * port the server gives in its PASV reply; transfer times in whole
 * seconds, as there is no gettimeofday().
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <net/in.h>

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <ctype.h>

#include "ftp.h"
#include "ftp_var.h"

struct	sockaddr_in hisctladdr;
struct	sockaddr_in data_addr;
int	data = -1;
int	connected;
struct	sockaddr_in myctladdr;

FILE	*cin, *cout;
char	reply_string[BUFSIZ];	/* last line of the last reply */
FILE	*dataconn();

struct hostent *
hookup(host, port)
	char *host;
	int port;
{
	register struct hostent *hp;
	int s;

	bzero((char *)&hisctladdr, sizeof (hisctladdr));
	hp = gethostbyname(host);
	if (hp) {
		hisctladdr.sin_family = hp->h_addrtype;
		hostname = hp->h_name;
	} else {
		static struct hostent def;
		static struct in_addr defaddr;
		static char *defptrs[2];	/* Lisa: 4.3BSD's h_addr_list */
		static char namebuf[128];

		defaddr.s_addr = inet_addr(host);
		if (defaddr.s_addr == -1) {
			fprintf(stderr, "%s: Unknown host.\n", host);
			return (0);
		}
		strcpy(namebuf, host);
		def.h_name = namebuf;
		hostname = namebuf;
		def.h_addr_list = defptrs;
		def.h_addr = (char *)&defaddr;
		def.h_length = sizeof (struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	hisctladdr.sin_family = hp -> h_addrtype;
	s = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)0, SO_KEEPALIVE);
	if (s < 0) {
		perror("ftp: socket");
		return (0);
	}
	if (bind(s, (char *)&hisctladdr, sizeof (hisctladdr), 0) < 0) {
		perror("ftp: bind");
		goto bad;
	}
	bcopy(hp->h_addr, (char *)&hisctladdr.sin_addr, hp->h_length);
	hisctladdr.sin_port = port;
	if (connect(s, (char *)&hisctladdr, sizeof (hisctladdr), 0) < 0) {
		perror("ftp: connect");
		goto bad;
	}
	if (socketaddr(s, &myctladdr) < 0) {
		perror("ftp: socketaddr");
		goto bad;
	}
	cin = fdopen(s, "r");
	cout = fdopen(s, "w");
	if (cin == NULL | cout == NULL) {
		fprintf(stderr, "ftp: fdopen failed.\n");
		if (cin)
			fclose(cin);
		if (cout)
			fclose(cout);
		goto bad;
	}
	if (verbose)
		printf("Connected to %s.\n", hp->h_name);
	(void) getreply(0); 		/* read startup message from server */
	return (hp);
bad:
	close(s);
	return ((struct hostent *)0);
}

login(hp)
	struct hostent *hp;
{
	char acct[80];
	char *user, *pass;
	int n;

	user = pass = 0;
	ruserpass(hp->h_name, &user, &pass);
	n = command("USER %s", user);
	if (n == CONTINUE)
		n = command("PASS %s", pass);
	if (n == CONTINUE) {
		printf("Account: "); (void) fflush(stdout);
		(void) fgets(acct, sizeof(acct) - 1, stdin);
		acct[strlen(acct) - 1] = '\0';
		n = command("ACCT %s", acct);
	}
	if (n != COMPLETE) {
		fprintf(stderr, "Login failed.\n");
		return (0);
	}
	return (1);
}

/*VARARGS 1*/
command(fmt, args)
	char *fmt;
{

	if (debug) {
		printf("---> ");
		_doprnt(fmt, &args, stdout);
		printf("\n");
		(void) fflush(stdout);
	}
	_doprnt(fmt, &args, cout);
	fprintf(cout, "\r\n");
	(void) fflush(cout);
	return (getreply(!strcmp(fmt, "QUIT")));
}

#include <ctype.h>

getreply(expecteof)
	int expecteof;
{
	register char c, n;
	register int code, dig;
	int originalcode = 0, continuation = 0;
	register char *cp;

	for (;;) {
		dig = n = code = 0;
		cp = reply_string;
		while ((c = getc(cin)) != '\n') {
			if (c != EOF && cp < &reply_string[sizeof (reply_string) - 1])
				*cp++ = c;
			dig++;
			if (c == EOF) {
				if (expecteof)
					return (0);
				lostpeer();
				exit(1);
			}
			if (verbose && c != '\r' ||
			    (n == '5' && dig > 4))
				putchar(c);
			if (dig < 4 && isdigit(c))
				code = code * 10 + (c - '0');
			if (dig == 4 && c == '-')
				continuation++;
			if (n == 0)
				n = c;
		}
		if (verbose || n == '5')
			putchar(c);
		*cp = '\0';
		if (continuation && code != originalcode) {
			if (originalcode == 0)
				originalcode = code;
			continue;
		}
		/*
		 * Lisa: return after each complete reply, as 4.2BSD does.
		 * 4.1c read on while more input was waiting, so a 125 reply
		 * with the 226 behind it came back as 226 and the data
		 * connection was closed unread.
		 */
		return (n - '0');
	}
}

empty(f)
	FILE *f;
{
	long mask;

	if (f->_cnt > 0)
		return (0);
	mask = (1 << fileno(f));
	(void) select(20, &mask, 0, 0L);
	return (mask == 0);
}

jmp_buf	sendabort;

abortsend()
{

	longjmp(sendabort, 1);
}

sendrequest(cmd, local, remote)
	char *cmd, *local, *remote;
{
	FILE *fin, *dout, *popen();
	int (*closefunc)(), pclose(), fclose(), (*oldintr)();
	register long bytes = 0;
	register int c;		/* Lisa: int, or a 0377 byte reads as EOF */
	struct stat st;
	struct timeval start, stop;

	closefunc = NULL;
	if (setjmp(sendabort))
		goto bad;
	oldintr = signal(SIGINT, abortsend);
	if (strcmp(local, "-") == 0)
		fin = stdin;
	else if (*local == '|') {
		fin = popen(local + 1, "r");
		if (fin == NULL) {
			perror(local + 1);
			goto bad;
		}
		closefunc = pclose;
	} else {
		fin = fopen(local, "r");
		if (fin == NULL) {
			perror(local);
			goto bad;
		}
		closefunc = fclose;
		if (fstat(fileno(fin), &st) < 0 ||
		    (st.st_mode&S_IFMT) != S_IFREG) {
			fprintf(stderr, "%s: not a plain file.", local);
			goto bad;
		}
	}
	if (initconn())
		goto bad;
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM)
			goto bad;
	} else
		if (command("%s", cmd) != PRELIM)
			goto bad;
	dout = dataconn("w");
	if (dout == NULL)
		goto bad;
	gettimeofday(&start, (struct timezone *)0);
	while ((c = getc(fin)) != EOF) {
		if (type == TYPE_A && c == '\n')
			putc('\r', dout);
		putc(c, dout);
		bytes++;
	}
#ifndef noneterror
	if (ferror (dout))
		perror ("netout");
#endif
	gettimeofday(&stop, (struct timezone *)0);
	if (closefunc != NULL)
		(*closefunc)(fin);
	(void) fclose(dout);
	if (ferror(fin)) {
		perror(local);
		goto done;
	}
	(void) getreply(0);
done:
	signal(SIGINT, oldintr);
	if (bytes > 0 && verbose)
		ptransfer("sent", bytes, &start, &stop);
	return;
bad:
	if (data >= 0)
		(void) close(data), data = -1;
	if (closefunc != NULL && fin != NULL)
		(*closefunc)(fin);
	goto done;
}

jmp_buf	recvabort;

abortrecv()
{

	longjmp(recvabort, 1);
}

recvrequest(cmd, local, remote)
	char *cmd, *local, *remote;
{
	FILE *fout, *din, *popen();
	int (*closefunc)(), pclose(), fclose(), (*oldintr)();
	register int c;		/* Lisa: int, or a 0377 byte reads as EOF */
	register long bytes = 0;
	struct timeval start, stop;

	closefunc = NULL;
	if (setjmp(recvabort))
		goto bad;
	oldintr = signal(SIGINT, abortrecv);
	if (strcmp(local, "-") && *local != '|')
		if (access(local, 2) < 0) {
			char *dir = rindex(local, '/');

			if (dir != NULL)
				*dir = 0;
			if (access(dir ? dir : ".", 2) < 0) {
				perror(local);
				goto bad;
			}
			if (dir != NULL)
				*dir = '/';
		}
	if (initconn())
		goto bad;
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM)
			goto bad;
	} else
		if (command("%s", cmd) != PRELIM)
			goto bad;
	if (strcmp(local, "-") == 0)
		fout = stdout;
	else if (*local == '|') {
		fout = popen(local + 1, "w");
		closefunc = pclose;
	} else {
		fout = fopen(local, "w");
		closefunc = fclose;
	}
	if (fout == NULL) {
		perror(local + 1);
		goto bad;
	}
	din = dataconn("r");
	if (din == NULL)
		goto bad;
	gettimeofday(&start, (struct timezone *)0);
	while ((c = getc(din)) != EOF) {
		if (c != '\r' || type != TYPE_A)
			putc(c, fout);
		bytes++;
		if (ferror(fout)) {
			perror(local);
			while (c = getc(din) != EOF)
				bytes++;
			break;
		}
	}
#ifndef noneterror
	if (ferror (din))
		perror ("netin");
#endif
	gettimeofday(&stop, (struct timezone *)0);
	(void) fclose(din);
	if (closefunc != NULL)
		(*closefunc)(fout);
	(void) getreply(0);
done:
	signal(SIGINT, oldintr);
	if (bytes > 0 && verbose)
		ptransfer("received", bytes, &start, &stop);
	return;
bad:
	if (data >= 0)
		(void) close(data), data = -1;
	if (closefunc != NULL && fout != NULL)
		(*closefunc)(fout);
	goto done;
}

/*
 * Need to start a listen on the data channel
 * before we send the command, otherwise the
 * server's connect may fail.
 */
initconn()
{
	register char *p, *a;
	int result;
	int options = SO_KEEPALIVE | SO_ACCEPTCONN;

	if (passive)
		return (pasvconn());
	data_addr = myctladdr;
	data_addr.sin_port = 0;		/* let system pick one */ 
	data = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)&data_addr, options);
	if (data < 0) {
		perror("ftp: socket");
		return (1);
	}
	if (socketaddr(data, &data_addr) < 0) {
		perror("ftp: socketaddr");
		goto bad;
	}
	a = (char *)&data_addr.sin_addr;
	p = (char *)&data_addr.sin_port;
#define	UC(b)	(((int)b)&0xff)
	result =
	    command("PORT %d,%d,%d,%d,%d,%d",
	      UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]),
	      UC(p[0]), UC(p[1]));
	return (result != COMPLETE);
bad:
	(void) close(data), data = -1;
	return (1);
}

/*
 * Passive mode: ask the server for a port with PASV and connect to it.
 */
pasvconn()
{
	register char *cp;
	int a0, a1, a2, a3, p0, p1;

	if (command("PASV") != COMPLETE) {
		printf("Passive mode refused.\n");
		return (1);
	}
	for (cp = reply_string + 3; *cp && !isdigit(*cp); cp++)
		;
	if (sscanf(cp, "%d,%d,%d,%d,%d,%d", &a0, &a1, &a2, &a3, &p0, &p1) != 6) {
		printf("Passive mode address not understood: %s\n", reply_string);
		return (1);
	}
	bzero((char *)&data_addr, sizeof (data_addr));
	data_addr.sin_family = AF_INET;
	data_addr.sin_addr.s_addr = htonl(((long)a0 << 24) | ((long)a1 << 16) |
	    ((long)a2 << 8) | (long)a3);
	data_addr.sin_port = htons((p0 << 8) | p1);
	data = socket(SOCK_STREAM, (struct sockproto *)0, (struct sockaddr *)0, SO_KEEPALIVE);
	if (data < 0) {
		perror("ftp: socket");
		return (1);
	}
	if (connect(data, (char *)&data_addr, sizeof (data_addr), 0) < 0) {
		perror("ftp: data connection");
		(void) close(data), data = -1;
		return (1);
	}
	return (0);
}

FILE *
dataconn(mode)
	char *mode;
{
	struct sockaddr_in from;
	int s, fromlen = sizeof (from);

	if (passive)
		return (fdopen(data, mode));
	s = accept(data, &from, &fromlen, 0);
	if (s < 0) {
		perror("ftp: accept");
		(void) close(data), data = -1;
		return (NULL);
	}
	(void) close(data);
	data = s;
	return (fdopen(data, mode));
}

ptransfer(direction, bytes, t0, t1)
	char *direction;
	long bytes;
	struct timeval *t0, *t1;
{
	long secs = t1->tv_sec - t0->tv_sec;

	if (secs > 0)
		printf("%ld bytes %s in %ld seconds (%ld bytes/s)\n",
			bytes, direction, secs, bytes / secs);
	else
		printf("%ld bytes %s in under a second\n", bytes, direction);
}

tvadd(tsum, t0)
	struct timeval *tsum, *t0;
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
}

tvsub(tdiff, t1, t0)
	struct timeval *tdiff, *t1, *t0;
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}
