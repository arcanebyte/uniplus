/*
 * httpd.c -- a very small web server for UniPlus+ running unix.net
 *
 * usage: httpd [-p port] [docroot]	(defaults: port 80, /usr/www)
 *
 * Serves .html and .txt files from docroot with HTTP/1.0: GET and HEAD,
 * one connection at a time, closed after each reply.  A path ending in /
 * serves index.html.  Paths containing ".." are refused.  Each request is
 * logged on standard output.  Ports below 1024 need root.
 *
 * In the 4.1a socket API accept() connects the listening socket itself,
 * so every connection gets a fresh listening socket, as in tcpecho.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "net/socket.h"
#include "net/in.h"

#define REQLEN	1024		/* request line and headers kept */
#define PATHLEN	256
#define TIMEOUT	30		/* seconds a client has to send its request */

extern int errno;

int timedout;

ontimer()
{
	timedout = 1;
	signal(SIGALRM, ontimer);
}

/* write all of buf; returns -1 if the client has gone */
putall(s, buf, n)
int s, n;
char *buf;
{
	int len;

	while (n > 0) {
		if ((len = write(s, buf, n)) <= 0)
			return (-1);
		buf += len;
		n -= len;
	}
	return (0);
}

putstr(s, str)
int s;
char *str;
{
	return (putall(s, str, strlen(str)));
}

/* status line and headers */
header(s, code, reason, type, len)
int s, code;
char *reason, *type;
long len;
{
	char buf[256];

	sprintf(buf, "HTTP/1.0 %d %s\r\nServer: UniPlus+ httpd (Apple Lisa)\r\n",
	    code, reason);
	putstr(s, buf);
	sprintf(buf, "Content-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n",
	    type, len);
	putstr(s, buf);
}

/* an error reply with a short HTML body */
error(s, code, reason, head)
int s, code, head;
char *reason;
{
	char body[256];

	sprintf(body, "<html><head><title>%d %s</title></head>\n<body><h1>%d %s</h1></body></html>\n",
	    code, reason, code, reason);
	header(s, code, reason, "text/html", (long)strlen(body));
	if (!head)
		putstr(s, body);
	return (code);
}

/* does str contain sub?  (the Lisa's libc has no strstr) */
contains(str, sub)
register char *str;
char *sub;
{
	register int n = strlen(sub);

	for (; *str; str++)
		if (strncmp(str, sub, n) == 0)
			return (1);
	return (0);
}

/* ends in suffix? */
endswith(str, suffix)
char *str, *suffix;
{
	int n = strlen(str), m = strlen(suffix);

	return (n >= m && strcmp(str + n - m, suffix) == 0);
}

/* read the request line and headers, up to the blank line; returns length */
getreq(s, req)
int s;
char *req;
{
	int n = 0, len;

	timedout = 0;
	alarm(TIMEOUT);
	while (n < REQLEN - 1) {
		len = read(s, req + n, REQLEN - 1 - n);
		if (len <= 0)
			break;
		n += len;
		req[n] = '\0';
		if (contains(req, "\r\n\r\n") || contains(req, "\n\n"))
			break;
	}
	alarm(0);
	req[n] = '\0';
	return (timedout ? 0 : n);
}

/* answer one request; returns the status code, 0 if nothing was sent */
serve(s, req, logline)
int s;
char *req, *logline;
{
	char method[16], path[PATHLEN], file[PATHLEN + 16], buf[512];
	register char *p;
	char *type;
	struct stat st;
	int fd, n, head;

	if (sscanf(req, "%15s %255s", method, path) != 2) {
		strcpy(logline, "- -");
		return (error(s, 400, "Bad Request", 0));
	}
	sprintf(logline, "%s %s", method, path);
	head = strcmp(method, "HEAD") == 0;
	if (!head && strcmp(method, "GET") != 0)
		return (error(s, 501, "Not Implemented", 0));

	if ((p = strchr(path, '?')) != NULL)
		*p = '\0';
	if (path[0] != '/' || contains(path, ".."))
		return (error(s, 400, "Bad Request", head));
	for (p = path; *p; p++)
		if (*p < ' ' || *p > '~')
			return (error(s, 400, "Bad Request", head));

	/* "/" and "/dir/" serve index.html; the path is relative to docroot */
	sprintf(file, "%s%s", path + 1, endswith(path, "/") ? "index.html" : "");
	if (file[0] == '\0')
		strcpy(file, "index.html");

	if (endswith(file, ".html") || endswith(file, ".htm"))
		type = "text/html";
	else if (endswith(file, ".txt"))
		type = "text/plain";
	else
		return (error(s, 404, "Not Found", head));

	if ((fd = open(file, 0)) < 0)
		return (error(s, 404, "Not Found", head));
	if (fstat(fd, &st) < 0 || (st.st_mode & S_IFMT) != S_IFREG) {
		close(fd);
		return (error(s, 404, "Not Found", head));
	}
	header(s, 200, "OK", type, (long)st.st_size);
	if (!head)
		while ((n = read(fd, buf, sizeof (buf))) > 0)
			if (putall(s, buf, n) < 0)
				break;
	close(fd);
	return (200);
}

setaddr(sin, port)
struct sockaddr_in *sin;
int port;
{
	register char *p;
	register int i;

	p = (char *)sin;
	for (i = 0; i < sizeof (*sin); i++)
		*p++ = 0;
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
}

main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in sin, from;
	char req[REQLEN], logline[PATHLEN + 32];
	char *docroot = "/usr/www";
	unsigned long a;
	int s, port = 80, code, tries;

	while (argc > 1 && argv[1][0] == '-') {
		if (strcmp(argv[1], "-p") == 0 && argc > 2 &&
		    (port = atoi(argv[2])) > 0 && port <= 65535) {
			argv += 2;
			argc -= 2;
		} else {
			fprintf(stderr, "usage: httpd [-p port] [docroot]\n");
			exit(2);
		}
	}
	if (argc > 1)
		docroot = argv[1];
	if (chdir(docroot) < 0) {
		perror(docroot);
		exit(1);
	}
	signal(SIGPIPE, SIG_IGN);	/* a client that goes away mid-reply */
	signal(SIGALRM, ontimer);
	printf("httpd: serving %s on port %d\n", docroot, port);
	fflush(stdout);

	for (;;) {
		/* the port can stay busy briefly after a connection closes */
		for (tries = 0; ; tries++) {
			setaddr(&sin, port);
			s = socket(SOCK_STREAM, (struct sockproto *)0,
			    (struct sockaddr *)&sin, SO_ACCEPTCONN);
			if (s >= 0)
				break;
			if (errno != EADDRINUSE || tries == 30) {
				perror("httpd: socket");
				exit(1);
			}
			sleep(1);
		}
		setaddr(&from, 0);
		if (accept(s, (struct sockaddr *)&from) < 0) {
			perror("httpd: accept");
			close(s);
			continue;
		}
		strcpy(logline, "- -");
		if (getreq(s, req) > 0)
			code = serve(s, req, logline);
		else
			code = 0;
		a = ntohl(from.sin_addr.s_addr);
		printf("%d.%d.%d.%d %s %d\n", (int)((a >> 24) & 0xff),
		    (int)((a >> 16) & 0xff), (int)((a >> 8) & 0xff),
		    (int)(a & 0xff), logline, code);
		fflush(stdout);
		close(s);
	}
}
