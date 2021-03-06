/* Various machine specific constants */

/*
 * States needed for muli-byte COPS input sequences.
 */
#define NORMALWAIT	0
#define MOUSERD		1
#define YMOUSE		2
#define RESETCODE	3
#define SHUTDOWN	4
#define CLKREAD		5		/* must be last */

#define MAXXLOC		720
#define MAXYLOC		360

/* Raster Display information ... */
	/* Lowest possible (most dim) contrast setting */
#define TOTALDIM 0x3F
#define SCRNSIZE 0x8000				/* 32K */

	/* Contrast value set by boot rom */
#define ONCONT 0x20

#define MAXROW 40
#define MAXCOL 90

#define BPL	90
#define V_RESO	9
#define H_RESO	8

/* The Real Time Clock and Timer (wrist watch type)
 * This device hangs off the I/O board cops and is communicated with
 * through the console VIA (ie. l2copscmd).  The command byte is as follows:
 *	0x02	read clock data; the setting of the clock is returned as
 *		a seven byte reset code.
 *	0x2C	start clock and alarm set cycle.  Upto 16 nibbles of alarm
 *		and clock data (aaaaa y ddd hh mm ss t) may be sent as the
 *		low nibble of the following clock command.
 *	0x1n	write nibble `n' to clock (only valid after 0x2C cmd)
 *	0x25	terminates setup cycle and enables clock, timer is still
 *		diabled.
 * The Clock is connected to the power circuitry and can turn the system off
 * or be programed to turn it when the timer underflows.
 *	0x20	disbles the clock, timer, and shuts the system off.
 *	0x23	shuts off system leaving clock enabled and timer set to
 *		power up on underflow.
 */
#define SHUTOFF		0x21
#define READCLOCK	0x02
#define SETCLOCK	0x2C
#define CLKNIBBLE	0x10
#define STRTCLOCK	0x25

/* Other extrainous cops commands */
#define MOUSEON		0x7F		/* mouse on and int's every 28 ms */
#define MOUSEOFF	0x70		/* mouse off */
#define ENT_LOW		0x6F		/* low nibble of lights */
#define ENT_HI		0x52		/* high nibble of lights */
#define NMI_LOW		0x60		/* low nibble of NMI key */
#define NMI_HI		0x50		/* high nibble of NMI key */
#define IND_HI		0x40		/* high nibble of indicator lights */
#define IND_LOW		0x30		/* low nibble of indicator lights */
#define KBENABLE	0x00		/* enable keyboard */

struct rtime {		/* See page 35 LHRM */
	time_t	rt_tod;		/* Seconds since the Epoch */
	long	rt_alrm;	/* Seconds remaining to trigger alarm */
	short	rt_year;	/* year			(0 - 15) */
	short	rt_day;		/* julian day		(1 - 366) */
	short	rt_hour;	/* hour			(0 - 23) */
	short	rt_min;		/* minute		(0 - 59) */
	short	rt_sec;		/* second		(0 - 59) */
	short	rt_tenth;	/* tenths of a second	(0 - 9) */
};

#define MILLIRATE (1000/HZ)		/* rate of timer in milliseconds */

#define	BOOTDEV	(*(char *)(0x1B3))	/* boot device ID (used in System III) */
#define	NSLOTS	3			/* number of expansion slots */
#define	SLOTIDS	((short *)(0x298))	/* expansion slot 1 card ID */
#define	SLOTID2	((short *)(0x29A))	/* expansion slot 2 card ID */
#define	SLOTID3	((short *)(0x29C))	/* expansion slot 3 card ID */
#define	SLOTMASK	0xFFF		/* card type (ID #) of type code */
#define	ID_APLNET	1		/* card ID for applenet */
#define	ID_2PORT	2		/* card ID for 2-port card */
#define	ID_PRO		3		/* card ID for ProFile card */
#define	ID_PRIAM	5		/* card ID for Priam card */
#define	EXPIVECT	27		/* intr vector for 1st of 3 exp cards */
#define	devtoslot(d)	( ((d)==0) ? 2 : (((d)==2) ? 0 : 1) )
		/* dev 0 is ivec 29, dev 1 is ivec 28, dev 2 is ivec 27 */
