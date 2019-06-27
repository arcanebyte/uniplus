/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

int cksum;
#ifdef	DEBUG
int	debug = 0;
#endif
int	errno;

main()
{
	register int ch;
	register int lastchar = 0;

	puts("\r\nSTANDALONE DOWNLOAD MONITOR\r\n");
	putremote('\n');
	for (;;) {
		if ((ch = getlocal()) > 0)	/* poll console */
			putremote(ch);
		if ((ch = getremote()) > 0) {	/* poll remote */
			putlocal(ch);
			if (ch == 'S' && (lastchar == '\n' || lastchar == '\r'))
				download();
			lastchar = ch;
		}
	}
}

/* download a file */
download()
{
	register char *addr;
	register int count;
	register int ch;
	register int i;
	register char *laddr = (char *)0;

	for (;;) {
		cksum = 0;
		switch(i = wgetremote()) {
			case '0':	/* name of input file */
				while (wgetremote() != '\n') ;
				break;
			case '1':	/* data to be loaded */
			case '2':
				i = ((char)i == '1')? 2 : 3;
				count = getnum(1) - i - 1;
				addr = (char *) getnum(i);
				if (laddr && laddr != addr) {
					error("missing record");
					return;
				}
				laddr = addr + count;
				data(addr, count);
				getbyte();
				if ((cksum & 0xFF) != 0xFF) {
					error("cksum");
					return;
				}
				putlocal('.');	/* dot for record complete */
				break;
			case '8':	/* program execution */
			case '9':
				i = (i == '8')? 3 : 2;
				count = getnum(1);
				addr = (char *) getnum(i);
				while (wgetremote() != '\n') ;
				puts("\nTYPE RETURN TO START AT ");
				puthex(addr);
				bpoint();
				while (wgetlocal() != '\r') ;
				puts("\n\r");
				((int (*)())addr)();	/* call program */
				/* should never return */
			default:
				error("rec#");
				return;
		}
		while ((ch = wgetremote()) == '\n' || ch == '\r') ;
		if (ch != 'S') {
			error("no S");
			return;
		}
	}
}

/* put out a 6-digit hex number */
puthex(v)
register int v;
{
	puth1(v>>20);
	puth1(v>>16);
	puth1(v>>12);
	puth1(v>>8);
	puth1(v>>4);
	puth1(v);
}
puth1(v)
register int v;
{
	v &= 0xF;
	if (v < 10)
		wputlocal(v+'0');
	else
		wputlocal(v-10+'A');
}

/* get a number that is nb bytes (2*nb hex digits) long */
getnum(nb)
register int nb;
{
	register int v;

	v = 0;
	do {
		v = (v << 8) | getbyte();
	} while (--nb);
	return(v);
}

/* get and load data */
data(addr, count)
register char *addr;
register int count;
{
	if (count <= 0)
		return;
	do {
		*addr++ = getbyte();
	} while (--count);
}

/* get one byte (2 hex digits) */
getbyte()
{
	register int hi, low;

	hi = wgetremote();
	if (hi >= '0' && hi <= '9')
		hi -= '0';
	else if (hi >= 'A' && hi <= 'F')
		hi -= 'A' - 10;
	else
		error("#");
	low = wgetremote();
	if (low >= '0' && low <= '9')
		low -= '0';
	else if (low >= 'A' && low <= 'F')
		low -= 'A' - 10;
	else
		error("#");
	low |= hi << 4;
	cksum += low;
	return(low);
}

/* errors */
error(s)
char *s;
{
	puts("\r\nERROR ");
	puts(s);
	puts("\r\n");
	return;
}

/* put a string on local terminal */
puts(s)
register char *s;
{
	while (*s)
		wputlocal(*s++);
}

bpoint()
{
}
