
static char *sccsid = "@(#)getty.c	4.4 (Berkeley) 8/13/81";
/*
 * getty -- adapt to terminal speed on dialup, and call login
 *
 *****************************************************************
 * HISTORY:
 *		sjh	added ioctl call to set the HUP bit. this
 *		is so that the develcon will see that the user
 *		has loged out and will make the line free again.
 *
 * 4/82		grp	Changed the ERASE, KILL, and CKILL defaults
 *		to '^H' and '^U'. I also made the '8' sequence cycle
 *		to 300 after 1200.
 *
 * 8/82		grp	Added code to timeout after 90 seconds. This
 *		will help free some develcon ports.
 *
 * 8/82		dgc	Added the U of Texas/UCLA autobaud code.
 *
 * 3/83		dgc	Cleaned up the login banner
 *			Added the Boeing SSC RTS/CTS flow control code.
 */


#define TEK_TIMEOUT 1

#include <whoami.h>
#include <sgtty.h>
#include <signal.h>
#include <ctype.h>

#define ERASE	010		/* BS, ctl-h */
#define KILL	025		/* NAK, ctl-u */

#define	CEOT	004		/* EOT, ctl-d */
#define	CKILL	025		/* NAK, ctl-u */
#define	CQUIT	034		/* FS, cntl shift L */
#define	CINTR	003		/* ctl-c */
#define	CSTOP	023		/* Stop output: ctl-s */
#define	CSTART	021		/* Start output: ctl-q */
#define	CBRK	0377


struct sgttyb tmode;

struct	tab {
	char	tname;		/* this table name */
	char	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	char	*message;	/* login message */
} itab[] = {

/* table '0'-1-2-3 300,1200,150,110 */

	'0', 1,
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+CR2,
	B300, B300,
	"\n\rlogin: ",

	1, 2,
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B1200, B1200,
	"\n\rlogin: ",

	2, 3,
	ANYP+RAW+NL1+CR1, EVENP+ECHO+FF1+CR2+TAB1+NL1,
	B150, B150,
	"\n\rlogin: ",

	3, '0',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CRMOD+XTABS+LCASE+CR1,
	B110, B110,
	"\n\rlogin: ",

/* table '-' -- Console TTY 110 */
	'-', '-',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CRMOD+XTABS+LCASE+CR1,
	B110, B110,
	"\n\rlogin: ",

/* table '1' -- 150 */
	'1', '1',
	ANYP+RAW+NL1+CR1, EVENP+ECHO+FF1+CR2+TAB1+NL1,
	B150, B150,
	"\n\r\033:\006\006\017login: ",

/* table '2' -- 9600 */
	'2', '2',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
	"\n\rlogin: ",

/* table '3'-'5' -- 1200,300 */
	'3', '5',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B1200, B1200,
	"\n\rlogin: ",

/* table '5'-'3' -- 300,1200 */
	'5', '3',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CR1,
	B300, B300,
	"\n\rlogin: ",

/* table '4' -- Console Decwriter */
	'4', '4',
	ANYP+RAW, ANYP+ECHO+CRMOD+XTABS,
	B300, B300,
	"\n\rlogin: ",

/* table '6' -- 2400  */
	'6', '6' ,
	ANYP+RAW,  ANYP+ECHO+CRMOD,
	B2400 , B2400 ,
	"\n\rlogin: ",
 
/* table '7' - - 4800 */
	'7' , '7' ,
	ANYP+RAW , ANYP+ECHO+CRMOD ,
	B4800 , B4800 ,
	"\n\rlogin: " ,

/* table '8'-'9' - 9600 */
	'8', '9',
	ANYP+RAW/*+HUPCLS*/, ANYP+XTABS+ECHO+CRMOD/*+HUPCLS*/,
	B9600, B9600,
	"\n\rlogin: ",

	/* table '9' - - 4800 */
	'9' , 'a' ,
	ANYP+RAW/*+HUPCLS*/, ANYP+XTABS+ECHO+CRMOD/*+HUPCLS*/,
	B4800 , B4800 ,
	"\n\rlogin: " ,

	/* table 'a' -- 2400  */
	'a', 'b' ,
	ANYP+RAW/*+HUPCLS*/, ANYP+XTABS+ECHO+CRMOD/*+HUPCLS*/,
	B2400 , B2400 ,
	"\n\rlogin: ",
 
	/* table 'b' - 1200 */
	'b', 'c',
	ANYP+RAW/*+HUPCLS*/, ANYP+XTABS+ECHO+CRMOD/*+HUPCLS*/,
	B1200, B1200,
	"\n\rlogin: ",

	/* table 'c' - 300 */
	'c', 'd',
	ANYP+RAW/*+HUPCLS*/, ANYP+XTABS+ECHO+CRMOD/*+HUPCLS*/,
	B300, B300,
	"\n\rlogin: ",

	/* table 'c' - 600 */
	'd', '8',
	ANYP+RAW/*+HUPCLS*/, ANYP+XTABS+ECHO+CRMOD/*+HUPCLS*/,
	B600, B600,
	"\n\rlogin: ",

/* table 'i' -- Interdata Console */
	'i', 'i',
	RAW+CRMOD, CRMOD+ECHO+LCASE,
	0, 0,
	"\n\rlogin: ",

/* table 'l' -- LSI Chess Terminal */
	'l', 'l',
	ANYP+RAW/*+HUPCL*/, ANYP+ECHO/*+HUPCL*/,
	B300, B300,
	"*",

#ifdef	TEXAS_AUTOBAUD
/* table 'B' -- Do autobauding */

#define	NTRIES	5
#define	WAITSEC	60
#define	TB_AUTOBAUD	'B'
	TB_AUTOBAUD, TB_AUTOBAUD,
	ANYP+RAW,	ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
	"\n\r;login: ",
#endif

#ifdef	SSC_RTSCTS
/* table 'S' -- Sytek port, set RTS/CTS flow control */
/* NOTE: set this on DH lines only; it will NOT work */
/* on DZ lines (and might cause system problems if   */
/* you tried it.				     */

#define	TB_RTSCTS	'S'
	TB_RTSCTS, TB_RTSCTS,
	ANYP+RAW,	ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
	"\n\r;login: ",
#endif
};

#define	NITAB	sizeof itab/sizeof itab[0]
#define	EOT	04		/* EOT char */

char	name[16];
int	crmod;
int	upper;
int	lower;

char partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,
	0202,0004,0003,0205,0005,0206,0201,0001,
	0201,0001,0001,0201,0001,0201,0201,0001,
	0001,0201,0201,0001,0201,0001,0001,0201,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0201
};

main(argc, argv)
char **argv;
{
	register struct tab *tabp;
	char tname;
	struct tchars tc;
	char hostname[32];

/*
	signal(SIGINT, 1);
	signal(SIGQUIT, 0);
*/
	gethostname(hostname,sizeof hostname);
#ifdef	SSC_RTSCTS
	ioctl(0, TIOCCSYTEK, 0);
#endif
	tname = '0';
	if (argc > 1)
		tname = argv[1][0];
	for (;;) {
		int ldisp = 0;
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
			if(tabp->tname == tname)
				break;
		if(tabp >= &itab[NITAB])
			tabp = itab;
		tmode.sg_ispeed = tabp->ispeed;
		tmode.sg_ospeed = tabp->ospeed;
		tmode.sg_flags = tabp->iflags;
		tmode.sg_ispeed = tabp->ispeed;
		tmode.sg_ospeed = tabp->ospeed;
		stty(0, &tmode);
		tc.t_intrc = CINTR;
		tc.t_quitc = CQUIT;
		tc.t_stopc = CSTOP;
		tc.t_startc = CSTART;
		tc.t_brkc = CBRK;
		tc.t_eofc = CEOT;
		ioctl(0, TIOCHPCL, &tc);    /* hang up line on last close */
		ioctl(0, TIOCSETC, &tc);
		ioctl(0, TIOCSETD, &ldisp);
#ifdef	SSC_RTSCTS
		if (tabp->tname == TB_RTSCTS)
		{	/* Set RTS/CTS flow control; DH lines only! */
			ioctl(0, TIOCSSYTEK, 0);
		}
#endif
#ifdef	TEXAS_AUTOBAUD
		if ((tabp->tname == TB_AUTOBAUD) && set_baud_rate())
		{	/* autobaud line */
			ioctl(0, TIOCGETP, &tmode); /* pickup baud rate */
			ioctl(0, TIOCSETP, &tmode); /* flush input */
		}
#endif
		if (tmode.sg_ospeed > B1200)
			puts("\n\r\n\r");
		else
			puts("\n\r\r\r\r\r\n\r\r\r\r\r");
		puts(hostname);
		puts(": ");
		puts(banner);
		puts("\n\r\r\r\r");
		puts(tabp->message);
#ifdef TEK_TIMEOUT
		if (tabp->tname != '4')
			alarm(90);
#endif
		/*
		 * Wait a while, then flush input to get rid
		 * of noise from open lines
		 */
		sleep(1);
		stty(0, &tmode);
		if(getname()) {
			if (upper == 0 && lower == 0)
				continue;
			tmode.sg_erase = ERASE;
			tmode.sg_kill = KILL;
			tmode.sg_flags = tabp->fflags;
			if(crmod)
				tmode.sg_flags |= CRMOD;
			if(upper)
				tmode.sg_flags |= LCASE;
			if(lower)
				tmode.sg_flags &= ~LCASE;
			stty(0, &tmode);
			putchr('\n');
#ifdef TEK_TIMEOUT
			alarm(0);
#endif
			execl("/bin/login", "login", name, 0);
			exit(1);
		}
		tname = tabp->nname;
	}
}

getname()
{
	register char *np;
	register c;
	char cs;

	crmod = 0;
	upper = 0;
	lower = 0;
	np = name;
	for (;;) {
		if (read(0, &cs, 1) <= 0)
			exit(0);
		if ((c = cs&0177) == 0)
			return(0);
		if (c==EOT)
			exit(1);
		if (c=='\r' || c=='\n' || np >= &name[16])
			break;
		putchr(cs);
		if (c>='a' && c <='z')
			lower++;
		else if (c>='A' && c<='Z') {
			upper++;
		} else if (c==ERASE) {
			if (np > name)
				np--;
			continue;
		} else if (c==KILL) {
			putchr('\r');
			putchr('\n');
			np = name;
			continue;
		} else if(c == ' ')
			c = '_';
		*np++ = c;
	}
	*np = 0;
	if (c == '\r')
		crmod++;
	if (upper && !lower)
		for (np = name; *np; np++)
			if (isupper(*np))
				*np = tolower(*np);
	return(1);
}

puts(as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		putchr(*s++);
}

putchr(cc)
{
	char c;
	c = cc;
	c |= partab[c&0177] & 0200;
	write(1, &c, 1);
}

#ifdef	TEXAS_AUTOBAUD

/* Autobauding tables */
struct match
{	
	char  m_char;	/* bit pattern to look for */
	char  m_mask;	/* bits to ignore in input character */
	short m_speed;	/* what speed to set if matched */
};

struct match match[] = {	/* cr,int            */
	{ 0xfc, 0x03, B9600 },	/* 111111xx (cr,int) */
	{ 0x0d, 0x80, B4800 },	/* x0001101 (cr)     */
	{ 0x03, 0x80, B4800 },	/* x0000011 (int)    */
	{ 0xe6, 0x00, B2400 },	/* 11100110 (cr)     */
	{ 0x1e, 0x00, B2400 },	/* 00011110 (int)    */
	{ 0x8c, 0x12, B1800 },	/* 100x11x0 (cr)     */
	{ 0x7c, 0x02, B1800 },	/* 011111x0 (int)    */
	{ 0x78, 0x80, B1200 },	/* x1111000 (cr,int) */
	{ 0x80, 0x00, B600  },	/* 10000000 (cr,int) */
	{ 0x00, 0x00, B300  },	/* 00000000 (cr,int) */
	{ 0,    0,    0     }
};

/* Determine baud rate of terminal line
 *     Rich Wales (UCLA), June 1982
 *
 * Determine the baud rate of a terminal from a single
 * carriage-return or ^C.  It can identify 300, 600, 1200, 1800, 2400,
 * 4800, and 9600 baud.
 *
 * If the new speed is below 2400 baud, it is advisable to do a "sleep"
 * before the "ioctl".  This allows any extra garbage from the input to
 * make it from the multiplexer (DH or DZ) to the kernel input queue,
 * where the speed change will cause it to be flushed.
 */
timeout()
{
	puts("\7Connection timed out.\n\r");
	exit(1);
};

set_baud_rate()
{   
	register struct match *m;
	register int i;
	int lmode;
	struct sgttyb sv, st;
	char c;

	/* save current TTY mode, set new mode */
	ioctl(0, TIOCGETP, &sv);
	st = sv;
	st.sg_flags  = RAW | ANYP;
	ioctl(0, TIOCSIMG, 0);
	ioctl(0, TIOCLGET, &lmode);
	st.sg_ispeed = st.sg_ospeed = B4800;
	ioctl(0, TIOCSETP, &st);
	ioctl(0, TIOCLSET, &lmode);
	alarm(0);
	for (i=0; i < NTRIES; i++) {
		/* read a character and try to match it */
		signal(14,timeout);
		alarm(WAITSEC);
		read(0,&c,1);
		alarm(0);
		c &= 0377;
		for (m = match; m->m_speed != 0; m++) {
			if ((c &~ m->m_mask) == m->m_char) {
			/* success -- restore old modes, but with new speed */
				sv.sg_ispeed = sv.sg_ospeed = m->m_speed;
				if (sv.sg_ispeed < B2400)
					sleep(1);
				ioctl(0, TIOCCIMG, 0);
				ioctl(0, TIOCSETP, &sv);
				ioctl(0, TIOCLSET, &lmode);
				return(1);
			}
		}
	}
	/* no match -- restore old modes */
	sleep(1);
	ioctl(0, TIOCCIMG, 0);
	ioctl(0, TIOCSETP, &sv);
	ioctl(0, TIOCLSET, &lmode);
	return(0);
}
#endif
