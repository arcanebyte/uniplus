#ifndef lint
static char sccsid[] = "@(#)raddr.c	4.3 82/08/24";
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

char	*any(), *rany(), *malloc();
u_long value();

char *
raddr(desaddr)
	int desaddr;
{
	FILE *hf = fopen("/usr/lib/hosts", "r");
	char hbuf[BUFSIZ], *host;
	register char *cp;
	int first = 1;

	if (hf == NULL) {
		perror("/usr/lib/hosts");
		exit(1);
	}
top:
	while (fgets(hbuf, sizeof (hbuf), hf)) {
		int addr;

		if (*hbuf == '#')
			continue;
		cp = rany(hbuf, "#\n");
		if (cp == NULL)
			continue;
		*cp = '\0';
		addr = (int)value(hbuf);
		if (addr != desaddr || addr == -1)
			continue;
		host = any(hbuf, " \t");
		if (host == NULL)
			continue;
		while (*host == ' ' || *host == '\t')
			host++;
		cp = any(host, " \t");
		if (cp)
			*cp = '\0';
		cp = malloc(strlen(host)+1);
		strcpy(cp, host);
		fclose(hf);
		return (cp);
	}
	if (first == 1) {
		first = 0;
		fclose(hf);
		if (hf = fopen("/etc/hosts.local", "r"))
			goto top;
		return (0);
	}
	fclose(hf);
	return (0);
}

static u_long
value(cp)
	register char *cp;
{
	u_long val, base, n;
	register char c;
	u_long parts[4], *pp = parts;

again:
	val = 0; base = 10;
	if (*cp == '0')
		base = 8, cp++;
	if (*cp == 'x' || *cp == 'X')
		base = 16, cp++;
	while (c = *cp) {
		if (isdigit(c)) {
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		/*
		 * Internet format:
		 *	a.b.c.d
		 *	a.b.c	(with c treated as 16-bits)
		 *	a.b	(with b treated as 24 bits)
		 */
		if (pp >= parts + 4)
			return (-1);
		*pp++ = val, cp++;
		goto again;
	}
	if (*cp && !isspace(*cp))
		return (-1);
	n = pp - parts;
	if (n > 0) {
		if (n > 4)
			return (-1);
		*pp++ = val; n++;
		val = parts[0];
		if (n > 1)
			val <<= 24;
		if (n > 2)
			val |= (parts[1] & 0xff) << 16;
		if (n > 3)
			val |= (parts[2] & 0xff) << 8;
		if (n > 1)
			val |= parts[n - 1];
#if vax || pdp11
		val = htonl(val);
#endif
	}
	return (val);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

static char *
rany(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;
	char *last = NULL;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				last = cp;
		cp++;
	
	}
	return (last);
}
