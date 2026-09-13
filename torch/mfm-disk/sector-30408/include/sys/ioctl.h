/* NFSSRC @(#)ioctl.h	2.1 86/04/11 */
/*	@(#)ioctl.h	1.6 of 12/16/86 */
/*	ioctl.h	6.1	83/07/29	*/
/*
 * Ioctl definitions
 */
#ifndef	_IOCTL_
#define	_IOCTL_

#ifndef _IO
/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 127 bytes.
 */
#define	IOCPARM_MASK	0x7f		/* parameters must be < 128 bytes */
#define	IOCTYPE_MASK	0xff00		/* i/o control type mask */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		0x80000000	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
/* the 0x20000000 is so we can distinguish new ioctl's from old */
#define	_IO(x,y)	(IOC_VOID|('x'<<8)|y)
#define	_IOR(x,y,t)	(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_IOW(x,y,t)	(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
/* this should be _IORW, but stdio got there first */
#define	_IOWR(x,y,t)	(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_IOCTYPE(x)	((x) & IOCTYPE_MASK)
#endif

#define	TIOCGCOMPAT	_IOR(t, 121, int)	/* get bsd compatibility flag */
#define	TIOCSCOMPAT	_IOW(t, 120, int)	/* set bsd compatibility flag */
#define	TIOCGPGRP	_IOR(t, 119, int)	/* get pgrp of tty */
#define	TIOCSPGRP	_IOW(t, 118, int)	/* set pgrp of tty */
#define	TIOCSLTC	_IOW(t,117,struct ltchars)/* set local special chars */
#define	TIOCGLTC	_IOR(t,116,struct ltchars)/* get local special chars */
#define	TIOCNOTTY	_IO(t, 113)		/* void tty association */
#define	TIOCPKT		_IOW(t, 112, int)	/* pty: set/clear packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */

#define	FIOCLEX		_IO(f, 1)		/* set exclusive use on fd */
#define	FIONCLEX	_IO(f, 2)		/* remove exclusive use */
/* another local */
#define	FIONREAD	_IOR(f, 127, int)	/* get # bytes to read */
#define	FIONBIO		_IOW(f, 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW(f, 125, int)	/* set/clear async i/o */
#define	FIOSETOWN	_IOW(f, 124, int)	/* set owner */
#define	FIOGETOWN	_IOR(f, 123, int)	/* get owner */

/* V.2 i/o controls */
#define	LDOPEN		_IO(D,   0)			/* line disc. open */
#define	LDCLOSE		_IO(D,   1)			/* line disc. open */
#define	LDCHG		_IO(D,   2)			/* line disc. change */
#define	LDGETT		_IOR(D,  8, struct termcb)	/* get termcb struct */
#define	LDSETT		_IOW(D,  9, struct termcb)	/* set termcb struct */

#define	SXTIOCLINK	_IO(b,   0)
#define	SXTIOCTRACE	_IO(b,   1)
#define	SXTIOCNOTRACE	_IO(b,   2)
#define	SXTIOCSWTCH	_IO(b,   3)
#define	SXTIOCWF	_IO(b,   4)
#define	SXTIOCBLK	_IO(b,   5)
#define	SXTIOCUBLK	_IO(b,   6)
#define	SXTIOCSTAT	_IOR(b,  7, struct sxtblock)

#define	TCGETA		_IOR(T,  1, struct termio)	/* get termio struct */
#define	TCSETA		_IOW(T,  2, struct termio)	/* get termio struct */
#define	TCSETAW		_IOW(T,  3, struct termio)	/* get termio struct */
#define	TCSETAF		_IOW(T,  4, struct termio)	/* get termio struct */
#define	TCSBRK		_IO(T,   5)			/* get/set  break */
#define	TCXONC		_IO(T,   6)			/* suspend/resume i/o */
#define	TCFLSH		_IO(T,   7)			/* flush input/output */

/* UniSoft i/o controls */
#define	UIOCFORMAT	_IOW(U,  0, struct diskformat)	/* format disk */
#define	UIOCEXTE	_IO(U,   1)			/* enable extended error reporting */
#define	UIOCNEXTE	_IO(U,   2)			/* disable extended error reporting */
#define	UIOCWCHK	_IO(U,   3)			/* enable write check */
#define	UIOCNWCHK	_IO(U,   4)			/* disable write check */
#define	UIOCGETDT	_IOR(U,  8, struct disktune)	/* get disktune struct */
#define	UIOCSETDT	_IOW(U,  9, struct disktune)	/* set disktune struct */
#define	UIOCBDBK	_IO(U,   10)			/* resync bad block table */


/*
 *	The following ioctls are for modem control - not all devices support
 *		all or any them. If any are supported then UIOCTTSTAT is
 *		supported.
 *	The default is UIOCMODEM/UIOCNOFLOW. All these are "remembered" when a
 *		device is closed and reopened again.
 *
 *	The four following are mutually exclusive (on some systems DTR/DCD
 *		are named oppositely: here DCD is the input, DTR the output)
 *
 *	UIOCMODEM	- modem control (DTR/DCD) is enabled (DCD is required
 *			  before a device can be opened, if it is removed the
 *			  device is "hung up", on opening DTR is asserted)
 *	UIOCNOMODEM	- no modem control, DTR is still asserted but DCD is
 *			  ignored and opens always complete without waiting
 *	UIOCDTRFLOW	- the DCD (on some printers this is the DTR line)
 *			  is used for flow control. It must be asserted before
 *			  characters can be transmitted.
 *	UIOCEMODEM	- 'european style' modem control (DTR/DCD/RI) is enabled
 *			  (DCD is required before a device can be opened, if it
 *			  is removed the device is "hung up", on opening DTR is
 *			  not asserted until a RI input is detected)
 *			
 *
 *	The next two are also mutually exclusive. Again CTS/RTS are sometimes
 *		named oppositely. Here RTS is the output, CTS the input
 *
 *	UIOCNOFLOW	- hardware flow control is disabled. RTS is asserted
 *			  before transmitting (or asserted all the time). CTS
 *			  is ignored
 *	UIOCFLOW	- hardware flow control is enabled. RTS is asserted
 *			  before transmitting. CTS must be asserted by the other
 *			  end before transmission can start. (this is required
 *			  for every character).
 *
 *
 *	UIOCTTSTAT	- this returns 3 bytes. The first is 1 if UIOCMODEM
 *			  is enabled, 2 if UIOCEMODEM is enabled. The second is
 *			  1 if UIOCDTRFLOW is enabled and the third is 1 if
 *			  UIOCFLOW is enabled.
 *
 */

#define	UIOCMODEM	_IO(U,   128)
#define	UIOCNOMODEM	_IO(U,   129)
#define	UIOCEMODEM	_IO(U,   134)
#define	UIOCDTRFLOW	_IO(U,   130)
#define	UIOCFLOW	_IO(U,   131)
#define	UIOCNOFLOW	_IO(U,   132)
#define	UIOCTTSTAT	_IOR(U,   133, char[3])

/* socket i/o controls */
#define	SIOCSHIWAT	_IOW(s,  0, int)		/* set high watermark */
#define	SIOCGHIWAT	_IOR(s,  1, int)		/* get high watermark */
#define	SIOCSLOWAT	_IOW(s,  2, int)		/* set low watermark */
#define	SIOCGLOWAT	_IOR(s,  3, int)		/* get low watermark */
#define	SIOCATMARK	_IOR(s,  7, int)		/* at oob mark? */
#define	SIOCSPGRP	_IOW(s,  8, int)		/* set process group */
#define	SIOCGPGRP	_IOR(s,  9, int)		/* get process group */

#define	SIOCADDRT	_IOW(r, 10, struct rtentry)	/* add route */
#define	SIOCDELRT	_IOW(r, 11, struct rtentry)	/* delete route */

#define	SIOCSIFADDR	_IOW(i, 12, struct ifreq)	/* set ifnet address */
#define	SIOCGIFADDR	_IOWR(i,13, struct ifreq)	/* get ifnet address */
#define	SIOCSIFDSTADDR	_IOW(i, 14, struct ifreq)	/* set p-p address */
#define	SIOCGIFDSTADDR	_IOWR(i,15, struct ifreq)	/* get p-p address */
#define	SIOCSIFFLAGS	_IOW(i, 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR(i,17, struct ifreq)	/* get ifnet flags */
#define	SIOCSIFMEM	_IOW(i, 18, struct ifreq)	/* set interface mem */
#define	SIOCGIFMEM	_IOWR(i,19, struct ifreq)	/* get interface mem */
#define	SIOCGIFCONF	_IOWR(i,20, struct ifconf)	/* get ifnet list */
#define	SIOCSIFMTU	_IOW(i, 21, struct ifreq)	/* set if_mtu */
#define	SIOCGIFMTU	_IOWR(i,22, struct ifreq)	/* get if_mtu */

#define	SIOCSARP	_IOW(i, 30, struct arpreq)	/* set arp entry */
#define	SIOCGARP	_IOWR(i, 31, struct arpreq)	/* get arp entry */
#define	SIOCDARP	_IOW(i, 32, struct arpreq)	/* delete arp entry */
#define SIOCUPPER       _IOW(i, 40, struct ifreq)       /* attach upper layer */
#define SIOCLOWER       _IOW(i, 41, struct ifreq)       /* attach lower layer */

/* protocol i/o controls */
#define SIOCSNIT        _IOW(p,  0, struct nit_ioc)     /* set nit modes */
#define SIOCGNIT        _IOWR(p, 1, struct nit_ioc)     /* get nit modes */

struct ltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};
struct ttysize {
	int	ts_lines;	/* number of lines on terminal */
	int	ts_cols;	/* number of columns on terminal */
};

#endif
