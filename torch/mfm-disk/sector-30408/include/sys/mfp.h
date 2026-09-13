struct mfpdevice {
	char	m_gpip;		/* GENERAL PURPOSE REGISTER */
	char	m_xxx1;
	char	m_aer;		/* ACTIVE EDGE REGISTER */
	char	m_xxx3;
	char	m_ddr;		/* DATA DIRECTION REGISTER */
	char	m_xxx5;
	char	m_iera;		/* INTERRUPT ENABLE A */
	char	m_xxx7;
	char	m_ierb;		/* INTERRUPT ENABLE B */ 
	char	m_xxx9;
	char	m_ipra;		/* INTERRUPT PENDING REG A */
	char	m_xxxb;
	char	m_iprb;		/* INTERRUPT PENDING REG B */
	char	m_xxxd;
	char	m_isra;		/* INTERRUPT IN-SERVICE REG A */
	char	m_xxxf;
	char	m_isrb;		/* INTERRUPT IN-SERVICE REG B */
	char	m_xxx11;
	char	m_imra;		/* INTERRUPT MASK REG A */
	char	m_xxx13;
	char	m_imrb;		/* INTERRUPT MASK REG B */
	char	m_xxx15;
	char	m_vec;		/* VECTOR REGISTER */
	char	m_xxx17;
	char	m_tacr;		/* TIMER A CONTROL REGISTER */
	char	m_xxx19;
	char	m_tbcr;		/* TIMER B CONTROL REGISTER */
	char	m_xxx1b;
	char	m_tcdcr;	/* TIMER C & D CONTROL REGISTER */
	char	m_xxx1d;
	char	m_tadr;		/* TIMER A DATA REGISTER */
	char	m_xxx1f;
	char	m_tbdr;		/* TIMER B DATA REGISTER */
	char	m_xxx21;
	char	m_tcdr;		/* TIMER C DATA REGISTER */
	char	m_xxx23;
	char	m_tddr;		/* TIMER D DATA REGISTER */
	char	m_xxx25;
	char	m_scr;		/* SYNC CHARACTER REGISTER */ 
	char	m_xxx27;
	char	m_ucr;		/* USART CONTROL REGISTER */ 
	char	m_xxx29;
	char	m_rsr;		/* RECEIVER STATUS REGISTER */ 
	char	m_xxx2b;
	char	m_tsr;		/* TRANSMITTER STATUS REGISTER */ 
	char	m_xxx2d;
	char	m_udr;		/* USART DATA REGISTER */ 
};

#define MFPADDR  	((struct mfpdevice *)0xFFF641)
#define	SYSCONCSR	((unsigned char *)0xFFF603)	/* SYSCON control/status reg */
#define SYSCON_PAR	((char *)0xFFF609)	/* Parallel Data Port */


/* general purpose i/o registers (gpip, aer, ddr)
 * gpip sends data (1 or 0)
 * aer determines interrupt edge (1=rising or 0=falling)
 * ddr determines direction (1=output or 0=input)
 */
#define AER		0x10		/* Interrupt on rising edge */

/* interrupt control registers ( i[epsm]r[ab] )
 * ier[ab] interrupt enable
 * ipr[ab] interrupt pending
 * isr[ab] interrupt in-service
 * imr[ab] interrupt mask
 */
/* 'a' registers */
#define TAENBL		0x20		/* Enable Timer A */
#define RINTENABLE	0x10		/* receiver enable */
#define RERRENABLE	0x08
#define TINTENABLE	0x04		/* transmitter enable */
#define TERRENABLE	0x02
#define TBENABL		0x01
/* 'b' registers */
#define PTRENBL		0x40		/* Enable Printer */
#define PTRDISA		0xBF		/* Disable Printer Interrupts */
#define TCENBL		0x20
#define TDENBL		0x10

/* vector register */
#define VECTOR		0x40		/* Hardware Interrupt Vector */
#define INSERVICE	0x08

/* timer A & B control register */
#define PRESCL200	0x07		/* Prescale = 200 */

/* timer C/D control register */
#define	D4C10		0x21		/* Timer D/4, Timer C/10 */

/* timer A,B,C & D data register */
#define TCON192		0xC0		/* Time constant = 192 */

/* sync character register */

/* usart control register */ 
#define D16		0x80		/* divide by 16 */
#define BITS5		0x60		/* 5 bit data */
#define BITS6		0x40		/* 6 bit data */
#define BITS7		0x20		/* 7 bit data */
#define BITS8		0x00		/* 8 bit data */
#define STOP1		0x08		/* 1 stop bit */
#define STOP15		0x10		/* 1.5 stop bit */
#define STOP2		0x18		/* 2 stop bits */
#define PENABLE		0x04		/* parity enable */
#define PEVEN		0x02		/* even parity */

/* receiver status register */ 
#define RFULL		0x80		/* receiver full */
#define EOVERRUN	0x40		/* Overrun Error */
#define EPARITY		0x20		/* Parity Error */
#define EFRAME		0x10		/* Framing Error */
#define EBREAK		0x08		/* received a break */
#define CIP		0x04		/* character in progress */
#define RXENABL		0x01		/* Enable Receiver */

/* transmitter status register */ 
#define TXBFE		0x80		/* Transmitter buffer empty */
#define EUNDERRUN	0x40		/* Underrun Error */
#define EEND		0x10		/* End of Transmission */
#define BREAK		0x08		/* send a break */
#define HIGH		0x04		/* Marking High */
#define LOW		0x02		/* marking low */
#define TXENABL		0x01		/* Enable Transmitter */

/* usart data register */ 

