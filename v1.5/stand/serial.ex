/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 * Duart registers are low (odd) bytes of 16 bit registers, therefore
 * each high byte is defined as a dummy.
 */
typedef struct	 {
	unsigned char
		d_mr,	dum2,		/* Mode Register 1/2 (A) */
		d_sr,	dum3,		/* Status Register (A) */
		d_res,	dum4,		/* Reserved */
		d_rhr,	dum5,		/* RX Holding Reg (A) */
		d_ipcr,	dum6,		/* Input Port Change Reg */
		d_isr,	dum7,		/* Interrupt Status Reg */
		d_ctu,	dum8,		/* Counter/Timer Upper */
		d_ctl,	dum9,		/* Counter/Timer Lower */
		d_xx1,	dum10,		/* Mode Register 1/2 (B) */
		d_xx2,	dum11,		/* Status Register (B) */
		d_xx3,	dum12,
		d_xx4,	dum13,		/* RX Holding Reg (B) */
		d_ivr,	dum14,		/* Interrupt Vector Register */
		d_iport, dum15,		/* Input Port */
		d_ccgo,	dum16,		/* Start Counter Command */
		d_ccstp;		/* Stop Counter Command */
} duart;

/* Extra definitions for paired registers (mostly for writing) */
#define	d_csr	d_sr		/* Clock Select Register (A) */
#define	d_cr	d_res		/* Command Register A */
#define	d_thr	d_rhr		/* TX Holding Register (A) */
#define	d_acr	d_ipcr		/* Aux Control Register */
#define	d_imr	d_isr		/* Interrupt Mask Register */
#define	d_ctur	d_ctu		/* C/T Upper Register */
#define	d_ctlr	d_ctl		/* C/T Lower Register */
#define	d_opcr	d_iport		/* Output Port Conf. Reg */
#define	d_sopbc	d_ccgo		/* Set Output Port Bits Command */
#define	d_ropbc	d_ccstp		/* Reset Output Port Bits Command */

/* MR1 bits - write to d_mr after resetting using CRRMR (see below) */
#define	M1RRTSC	0x80		/* [7] Receiver Request To Send Control */
#define	M1RIS	0x40		/* [6] Receiver Interrupt Select (1=FifoFull) */
#define	M1EMS	0x20		/* [5] Error Mode Select (0=status/char mode) */
#define	M1NoPar	0x10		/* [4:3] No parity */
#define	M1Par	0x00		/* [4:3] With parity */
#define	M1EVEN	0x00		/* [2] Parity type even */
#define	M1ODD	0x04		/* [2] Parity type odd */
#define	M1BITS5	0x00		/* [1:0] 5 bits per char */
#define	M1BITS6	0x01		/* [1:0] 6 bits per char */
#define	M1BITS7	0x02		/* [1:0] 7 bits per char */
#define	M1BITS8	0x03		/* [1:0] 8 bits per char */

/* MR2 bits - subsequent writes to d_mr */
#define	M2Normal 0x00		/* [7:6] Channel mode normal */
#define	M2TRSC	0x20		/* [5] Tx Req to Send Ctl (0=no) */
#define	M2CSC	0x10		/* [4] Clr to Send Ctl (0=no) (IP0/1) */
#define	M2St1	0x07		/* [3:0] 1 stop */
#define	M2St1p5	0x08		/* [3:0] 1.5 stop bits */
#define	M2St2	0x0F		/* [3:0] 2 stop */

/* SR bits - read d_sr */
#define	SRRB	0x80		/* Receive Break */
#define	SRFE	0x40		/* Framing Error */
#define	SRPE	0x20		/* Parity Error */
#define	SROE	0x10		/* Overrun Error */
#define	SRTE	0x08		/* Transmitter Empty */
#define	SRTR	0x04		/* Transmitter Ready - use this one */
#define	SRFF	0x02		/* Receive Fifo Full */
#define	SRRR	0x01		/* Receiver Ready - use this one */

/* CR bits - write to d_cr */
#define	CRRMR	0x10		/* [6:4] Reset MR to MR1 */
#define	CRRR	0x20		/* [6:4] Reset Receiver */
#define	CRRT	0x30		/* [6:4] Reset Transmitter */
#define	CRRES	0x40		/* [6:4] Reset Error Status */
#define	CRRBCI	0x50		/* [6:4] Reset Break Change Interrupt */
#define	CRSB	0x60		/* [6:4] Start Break */
#define	CRSPB	0x70		/* [6:4] Stop Break */
#define	CRDT	0x08		/* [3] Disable Transmitter */
#define	CRET	0x04		/* [2] Enable Transmitter */
#define	CRDR	0x02		/* [1] Disable Receiver */
#define	CRER	0x01		/* [0] Enable Receiver */

/* IPCR bits - read d_ipcr ??? */
#define	IPDTR	0x04		/* DTR A, <<1 DTR B */
#define	IPdDTR	0x40		/* delta DTRA, <<1 delta DTRB */

/* ACR bits - write to d_acr */
#define	ACRBRG	0x80		/* [7] Select baud rate set - 1 == set 2 */
/*#define	ACRCT	0x30		/* [6:4] Counter mode and source select */
#define	ACRCT	0x60		/* [6:4] Counter mode and source select */
#define	ACR	0xEB		/* for standalone */
/* #define ACRRATE	3600		/* 3.6 meghz clock constant for 64 hz */
#define	ACRRATE	30720

/* ISR bits - read d_isr */
/* These bits are tested in low nibble (A) and high nibble (B) */
#define	ISRCI	0x08		/* Input Port Chng Stat / Counter int enable */
#define	ISRCB	0x04		/* Change in Break */
#define	ISRRR	0x02		/* Receiver Ready */
#define	RR(p)	((p & 0x1) ? ISRRR<<4 : ISRRR)
#define	ISRTR	0x01		/* Transmitter Ready */
#define	TR(p)	((p & 0x1) ? ISRTR<<4 : ISRTR)
#define	ISRANY	(ISRCI|ISRCB|ISRRR|ISRTR|ISRCI)

/* IMR bits - Turn bit on to enable corresponding ISR bit - write to d_imr */
#define	IMROFF	0x00		/* nothing */
#define	IMRCI	0x08		/* Counter interrupt enable / change enable */
#define	IMR	0x07		/* RxRDYA/B, TxRDYA/B, Delta Break A/B */
#define	IMRON(p)	(p & 0x2 ? (p & 0x1 ? (IMR|IMRCI)<<4 : (IMR|IMRCI)) : (p & 0x1 ? (IMR|IMRCI)<<4 : IMR))

/* Input ports - IP4 is DCD for channels 2/4, IP5 is DCD for channels 1/3 */
#define	DCDA	0x10		/* data carrier detect for port A */
#define	DCDB	0x20		/* data carrier detect for port B */
#define	DCD(p)	((p & 0x1) ? DCDB : DCDA)

/* OPCR bits for standalone */
#define	OPCR	0xF0		/* TxRDYB/A, RxRDYB/A */

/* Output ports - OP0 is RTS for channels 2/4, OP1 is RTS for channels 1/3 */
#define	RTSA	0x1		/* request to send for port A */
#define	RTSB	0x2		/* request to send for port B */
#define	RTS(p)	((p & 0x1) ? RTSB : RTSA)

/* baud rate settings for duart (set 2) - write to d_csr */
#define BAUDhup		-1
#define BAUDbad		-2
#define BAUD50		BAUDbad
#define BAUD75		0x00
#define BAUD110		0x11
#define BAUD134_5	0x22
#define BAUD150		0x33
#define BAUD200		BAUDbad
#define BAUD300		0x44
#define BAUD600		0x55
#define BAUD1200	0x66
#define BAUD1800	0xAA
#define BAUD2400	0x88
#define BAUD4800	0x99
#define BAUD9600	0xBB
#define BAUD19200	0xCC
#define BAUD38400	BAUDbad

#define	D0ADDR	0xFF0040	/* Base of duart0 regs */
#define	D1ADDR	0xFF0060	/* Base of duart1 regs */
#define	DINCR	0x10		/* Offset to B port */
#define	NDU	4		/* number of duart ports */

duart *dad[NDU] = {
	(duart *)(D0ADDR),
	(duart *)(D0ADDR + DINCR),
	(duart *)(D1ADDR),
	(duart *)(D1ADDR + DINCR)
};

#define	LOCAL	0
#define	REMOTE	1

char init;

getremote()
{
	if (init == 0)
		duinit();
	return(getcraw(REMOTE));
}

putremote(c)
char c;
{
	if (init == 0)
		duinit();
	return(putcraw(c, REMOTE));
}

getlocal()
{
	if (init == 0)
		duinit();
	return(getcraw(LOCAL));
}

putlocal(c)
char c;
{
	if (init == 0)
		duinit();
	return(putcraw(c, LOCAL));
}

putcraw(c, n)
{
	register duart *dp;

	dp = dad[n];
	if ((dp->d_sr & SRTR) == 0)
		return(0);
	dp->d_thr = c;		/* Output char */
	return(c);
}

getcraw(n)
{
	register duart *dp;

	dp = dad[n];
	if ((dp->d_sr & SRRR) == 0)
		return(0);
	return (dp->d_rhr & 0177);
}

/*
 * duinit	- initalize the duart (BOTH CHANNELS)
 */
duinit()
{
	register duart *dp;
	register i;

	for (; init < (NDU/2); init++ ) {
		dp = dad[(init*2)];
					/* Set common chip bits */
		dp->d_opcr = OPCR;		/* Set Output Port bits */
		dp->d_acr  = ACR;		/* Set Aux Control bits */
		dp->d_imr  = IMROFF;		/* Set Interrupt mask */

		/* Set up Port A */
		dp->d_cr  = CRRMR;
		dp->d_mr  = M1RRTSC|M1NoPar|M1BITS8;
		dp->d_mr  = M2Normal|M2St1;
		dp->d_csr = BAUD9600;
		dp->d_cr  = CRRR;
		dp->d_cr  = CRRT;
		dp->d_cr  = CRRES;
		dp->d_cr  = CRER|CRET;

		/* Now get Port B */
		dp = dad[(init*2)+1];
		dp->d_cr  = CRRMR;
		dp->d_mr  = M1RRTSC|M1NoPar|M1BITS8;
		dp->d_mr  = M2Normal|M2St1;
		dp->d_csr = BAUD9600;
		dp->d_cr  = CRRR;
		dp->d_cr  = CRRT;
		dp->d_cr  = CRRES;
		dp->d_cr  = CRER|CRET;
	}

	for (i=0; i<10000; i++)
		;
}
