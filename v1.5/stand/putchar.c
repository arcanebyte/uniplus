/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

putchar(c)
register int c;
{
	adm_putc(c);
	if (c == '\n')
		adm_putc('\r');
	else if (c == '\r')
		adm_putc('\n');
}
