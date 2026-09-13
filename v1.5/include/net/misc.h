/*	@(#)misc.h UniPlus+ v.0.1.1 */
/* misc defines, etc, with nowhere else to go for now */

typedef	long		ubadr_t;	/* physical unibus address */
typedef unsigned short  u_short;
typedef unsigned int	u_int;
typedef long            u_long;         /* watch out! */
typedef char            u_char;         /* watch out! */
typedef short           void;           /* watch out! */
typedef struct  fd_set { long fds_bits[1]; } fd_set;

/* queue format expected by VAX queue instr's */
struct vaxque {
	struct vaxque *vq_next;
	struct vaxque *vq_prev;
};
	/* selectors and constructor for device code */

/* misc net related data */

int     selwait;        /* wchan for select waits */
u_long  ntohl(),htonl();
u_short ntohs(),htons();
#define	insque(q,p)	_insque((struct vaxque *)(q),(struct vaxque *)(p))
#define	remque(q)	_remque((struct vaxque *)(q))
#define DELAY(n) delay((long)n)
#define useracc(a,c,m) (1)
#define UBAPURGE(a,b)

/* conversion from VAX long identifiers to PDP (unique within 7 chars) */

#define INET 1
/*
#define getnetbyaddr gtntbya
#define gethostbyaddr gthsbya
*/
#define tcpstates tcpstas
#define sockaddr_in sock_in
#define sockaddr_pup sock_pup
#define spup_zero1 spup_z1
#define spup_zero2 spup_z2
#define if_ifwithaddr if_iwaddr
#define if_ifwithnet if_iwnet
#define if_ifwithaf if_iwaf
#define hz HZ
/* ??? */
#define NBBY 8
#define NBPG 512
/* ??? */
#define in_pcbdetach in_pcbetach
#define	SECDAY		((unsigned)(24*60*60))		/* seconds per day */
#define tcps_badsum tcps_bsum
#define tcps_badoff tcps_boff
#define tcps_badsegs tcps_bsegs
#define tcp_output tcp_oput
#define tcp_usrclosed tcp_uclosed
#define udps_badsum udps_bsum
#define udps_badlen udps_blen
#define ssocketaddr ssockad
#define soisconnecting soiscing
#define soisconnected soisced
#define soisdisconnecting soisding
#define soisdisconnected soisded
#define sbappendaddr sbappadd
#define protoswLAST protoLAST
#define rawintr rawint
#define tcp_initopt tcp_iopt
#define hostnamelen hostnlen
#define tcp_debx tcp_dbx
#define ips_toosmall ips_tsm
#define ips_tooshort ips_tsh

/*
 * Return values from tsleep().
 */
#define	TS_OK	0	/* normal wakeup */
#define	TS_TIME	1	/* timed-out wakeup */
#define	TS_SIG	2	/* asynchronous signal wakeup */

/*  a signal... */
#define SIGURG  16      /* urgent condition on IO channel */

/* misc ioctl values which should go in ioctl.h */
#define	FIONBIO		(('f'<<8)|126)
#define	FIOASYNC	(('f'<<8)|125)
#define	TIOCPKT		(('t'<<8)|112)	/* on pty: set/clear packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */

#define	SIOCDONE	(('s'<<8)|0)	/* shutdown read/write on socket */
#define	SIOCSKEEP	(('s'<<8)|1)	/* set keep alive */
#define	SIOCGKEEP	(('s'<<8)|2)	/* inspect keep alive */
#define	SIOCSLINGER	(('s'<<8)|3)	/* set linger time */
#define	SIOCGLINGER	(('s'<<8)|4)	/* get linger time */
#define	SIOCSENDOOB	(('s'<<8)|5)	/* send out of band */
#define	SIOCRCVOOB	(('s'<<8)|6)	/* get out of band */
#define	SIOCATMARK	(('s'<<8)|7)	/* at out of band mark? */
#define	SIOCSPGRP	(('s'<<8)|8)	/* set process group */
#define	SIOCGPGRP	(('s'<<8)|9)	/* get process group */
#define	SIOCADDRT	(('s'<<8)|10)	/* add a routing table entry */
#define	SIOCDELRT	(('s'<<8)|11)	/* delete a routing table entry */
#define	SIOCCHGRT	(('s'<<8)|12)	/* change a routing table entry */
#define	SIOCCIADDR	(('s'<<8)|13)	/* change internet address */
#define	SIOCGIADDR	(('s'<<8)|14)	/* get internet address */

/* low int of a long */
#define	loint(l)	((int) l & 0177777)

/* high int of a long */
#define	hiint(l)	(((int) (l >> 16))&0xffff)

/*  eg, htons = host-to-network-short */
#define htons(x)	(x)
#define ntohs(x)	(x)
#define htonl(x)	(x)
#define ntohl(x)	(x)


/* for address kludges in the driver */
struct klhshtab {
	long klval;		/* internet value */
	char klenaddr[6];	/* real ethernet addr */
};

/* system 3 ... */
#define index strchr
#define rindex strrchr

/* some flags for procs.  Should be in proc.h */
#define SSEL    010000          /* selecting, cleared if rescan needed */
#define STIMO   020000          /* timing out during sleep */

/* some defines for files.  Should be in file.h */
#define f_socket f_offset
#define FSOCKET 040     /* descriptor of a socket */
#define FISUSER 0100    /* fclose flag: !FISUSER -> fclose from exec or exit */

/* to make new code work with old code */
#define inetsw protosw
