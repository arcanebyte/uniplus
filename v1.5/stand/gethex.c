/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

gethex(cp)
register char *cp;
{
	register c, n;

	n = 0;
	while (c = *cp++) {
		if (c >= '0' && c <= '9')
			n = n * 16 + c - '0';
		else if (c >= 'a' && c <= 'f')
			n = n * 16 + c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			n = n * 16 + c - 'A' + 10;
		else return(-1);
	}
	return(n);
}
