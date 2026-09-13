/*
 * Definitions for the intel 8253/8253-5 timer chip
 */

#define	CLKADDR	((struct clk *) TIMEREG)

struct clk {
	short	ctr0;
	short	ctr1;
	short	ctr2;
	short	control;
};

/* control register bits */
#define	CTR	0xc0
#define		CTR2	0x80
#define		CTR1	0x40
#define		CTR0	0x00
#define	RWMODE	0x30
#define		LOHI	0x30
#define		LSB	0x20
#define		MSB	0x10
#define		LATCH	0x00
#define	MODE	0x0e
#define		MODE5	0x0a
#define		MODE4	0x08
#define		MODE3	0x06
#define		MODE2	0x04
#define		MODE1	0x02
#define		MODE0	0x00
#define	BCD	0x01
