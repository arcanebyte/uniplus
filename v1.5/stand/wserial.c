/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 *   Serial I/O routines
 */

/* wait for and return a character from the remote */
/******
wgetremote()
{
	register c;

	while ((c = getremote()) == 0)	;
	return(c);
}
******/

/* wait for remote to be ready and output a character */
/******
wputremote(c)
{
	while (putremote(c) == 0)	;
}
******/

/* wait for and return a character from the local */
wgetlocal()
{
	register c;

	while ((c = getlocal()) == 0)	;
	return(c);
}

/* wait for local to be ready and output a character */
/******
wputlocal(c)
{
	while (putlocal(c) == 0)	;
}
******/
