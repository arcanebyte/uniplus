/*
 * ruserpass.c -- the login name and password for an ftp server.
 *
 * Lisa: a smaller stand-in for 4.1c's ruserpass(), which also decrypted
 * passwords kept in the environment.  Reads plain "machine host login
 * name password word" entries from $HOME/.netrc, then asks for whatever
 * is still missing.
 */
#include <stdio.h>
#include <ctype.h>
#include "bsd.h"

char	*getenv(), *getpass(), *getlogin(), *malloc(), *strcpy();

static char *
save(s)
	char *s;
{
	char *p = malloc((unsigned)strlen(s) + 1);

	if (p)
		strcpy(p, s);
	return (p);
}

/* next word from the .netrc file, or 0 at its end */
static char *
token(f)
	FILE *f;
{
	static char word[100];
	register int c;
	register char *cp = word;

	while ((c = getc(f)) != EOF && (isspace(c) || c == ','))
		;
	while (c != EOF && !isspace(c) && c != ',') {
		if (cp < &word[sizeof (word) - 1])
			*cp++ = c;
		c = getc(f);
	}
	*cp = '\0';
	return (cp == word ? (char *)0 : word);
}

static
netrc(host, aname, apass)
	char *host, **aname, **apass;
{
	char file[200], *home, *t;
	FILE *f;
	int mine = 0;

	if ((home = getenv("HOME")) == NULL)
		return;
	sprintf(file, "%s/.netrc", home);
	if ((f = fopen(file, "r")) == NULL)
		return;
	while ((t = token(f)) != NULL) {
		if (strcmp(t, "machine") == 0) {
			t = token(f);
			mine = t != NULL && strcmp(t, host) == 0;
		} else if (strcmp(t, "login") == 0) {
			t = token(f);
			if (mine && t && *aname == 0)
				*aname = save(t);
		} else if (strcmp(t, "password") == 0) {
			t = token(f);
			if (mine && t && *apass == 0)
				*apass = save(t);
		}
	}
	fclose(f);
}

ruserpass(host, aname, apass)
	char *host, **aname, **apass;
{
	char buf[64], *myname, *nl;

	netrc(host, aname, apass);
	if (*aname == 0) {
		if ((myname = getlogin()) == NULL)
			myname = "";
		printf("Name (%s:%s): ", host, myname);
		fflush(stdout);
		if (fgets(buf, sizeof (buf), stdin) == NULL)
			exit(1);
		if ((nl = index(buf, '\n')) != NULL)
			*nl = '\0';
		*aname = save(buf[0] ? buf : myname);
	}
	if (*apass == 0) {
		sprintf(buf, "Password (%s:%s): ", host, *aname);
		*apass = getpass(buf);
	}
}
