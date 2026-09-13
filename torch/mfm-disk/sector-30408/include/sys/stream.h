/*	@(#)stream.h	UniPlus 2.1.6	*/
/*   Copyright (c) 1984 AT&T	and UniSoft Systems */
/*     All Rights Reserved  	*/

/*   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T and UniSoft Systems */
/*   The copyright notice above does not evidence any   	*/
/*   actual or intended publication of such source code.	*/

/*
 * data queue
 */
struct	queue {
	struct	qinit	*q_qinfo;	/* procs and limits for queue */
	struct	msgb	*q_first;	/* first data block */
	struct	msgb	*q_last;	/* last data block */
	struct	queue	*q_next;	/* Q of next stream */
	struct	queue	*q_link;	/* to next Q for scheduling */
	caddr_t	q_ptr;			/* to private data structure */
	short	q_count;		/* number of blocks on Q */
	unsigned short	q_flag;		/* queue state */
	short q_minpsz;			/* min packet size accepted by this module */
	short q_maxpsz;			/* max packet size accepted by this module */
	short q_hiwat;			/* queue high water mark */
	short q_lowat;			/* queue low water mark */
	char *q_pad;			/* pad field in case FS changes */
};

typedef struct queue queue_t;

/*
 * Queue flags
 */
#define	QENAB	01			/* Queue is already enabled to run */
#define	QWANTR	02			/* Someone wants to read Q */
#define	QWANTW	04			/* Someone wants to write Q */
#define	QFULL	010			/* Q is considered full */
#define	QREADR	020			/* This is the reader (first) Q */
#define	QUSE	040			/* This queue in use (allocation) */
#define	QNOENB	0100			/* Don't enable Q via putq */



/*
 * module information structure
 */
struct module_info {
	short	mi_idnum;		/* module id number */
	char 	*mi_idname;		/* module name */
	short   mi_minpsz;		/* min packet size accepted */
	short   mi_maxpsz;		/* max packet size accepted */
	short	mi_hiwat;		/* hi-water mark */
	short 	mi_lowat;		/* lo-water mark */
	char	*mi_pad;		/* pad field in case FS changes */
};


/*
 * queue information structure
 */
struct	qinit {
	int	(*qi_putp)();		/* put procedure */
	int	(*qi_srvp)();		/* service procedure */
	int	(*qi_qopen)();		/* called on startup */
	int	(*qi_qclose)();		/* called on finish */
	int	(*qi_qadmin)();		/* for 3bnet only */
	struct module_info *qi_minfo;	/* module information structure */
	int 	*qi_dummy;		/* pad field for FS changes */
};




/*
 * Streamtab (used in cdevsw and fmodsw to point to module or driver)
 */

struct streamtab {
	struct qinit *st_rdinit;
	struct qinit *st_wrinit;
	struct qinit *st_dummy1;
	struct qinit *st_dummy2;
};



/*
 * Header for a stream: interface to rest of system
 */

struct stdata {
	struct	queue *sd_wrq;		/* write queue */
	struct	msgb *sd_iocblk;	/* return block for ioctl */
	struct	vnode *sd_vnode;	/* backptr, for hangups */
	struct 	streamtab *sd_strtab;	/* pointer to streamtab for stream */
	long	sd_flag;		/* state/flags */
	long	sd_iocid;		/* ioctl id */
	ushort	sd_iocwait;		/* count of procs waiting to do ioctl */
	short	sd_pgrp;		/* process group, for signals */
	ushort	sd_wroff;		/* write offset */
	unchar   sd_error;		/* hangup or error to set u.u_error */
	ushort	 sd_ref;		/* reference count */
	unchar   sd_wait;		/* time to timeout */
	unchar   sd_min;		/* min characters for input */
/*
 *	Stuff for Unisoft selects
 */
	struct proc *sd_rsel;		/* Read select */
	struct proc *sd_wsel;		/* Write select */
/*
 */
	struct  strevent *sd_siglist;	/* pid linked list to rcv SIGSEL sig */
	struct	strevent *sd_sellist;	/* pid linked list to wakeup select() */
	int	sd_sigflags;		/* logical OR of all siglist events */
	int	sd_selflags;		/* logical OR of all sellist events */
};


/*
 * stdata flag field defines
 */
#define	IOCWAIT		01		/* Someone wants to do ioctl */
#define RSLEEP		02		/* Someone wants to read/recv msg */
#define	WSLEEP		04		/* Someone wants to write */
#define	STRHUP	       010		/* Device has vanished */
#define	STWOPEN	       020		/* waiting for 1st open */
#define CTTYFLG	      0200		/* used of finding control tty */
#define RMSGDIS	      0400		/* read msg discard */
#define RMSGNODIS    01000		/* read msg no discard */
#define STRERR	     02000		/* fatal error from M_ERROR */
#define STRTIME      04000		/* used with timeout strtime */
#define STR2TIME    010000		/* used with timeout str2time */
#define STR3TIME    020000		/* used with timeout str3time */
#define RSEL	    040000		/* > 1 sleeper on read select */
#define WSEL	   0100000		/* > 1 sleeper on write select */
#define ESEL	   0200000		/* > 1 sleeper on exception select */
#define ESLEEP	   0400000		/* Someone wants expedited data */
/*
 *	UniSoft flags for select and timeout
 */
#define TIME_OUT  01000000		/* We are in timeout mode (pwc) */
#define TIME_WAIT 02000000		/* We are waiting (timeout mode) (pwc)*/
#define STR_RCOLL 04000000		/* Read collision */
#define STR_WCOLL 010000000		/* Write collision */
#define STR_NBIO  020000000		/* Non-blocking IO */
#define STR_ASYNC 040000000		/* Async IO notification */

#define	STSOPEN	  0100000000 		/* Someone else is waiting for open */
#define	STR_CLOSING 0200000000 		/* We are in the process of closing */
#define TIMED_OUT   0400000000		/* a timeout occured */
#define STR_TOSTOP 01000000000		/* BSD stop on background output */

/*
 *  Data block descriptor
 */
struct datab {
	struct datab	*db_freep;
	char		*db_base;
	char		*db_lim;
	unsigned char	db_ref;
	unsigned char	db_type;
	unsigned char	db_class;
};


/*
 * Message block descriptor
 */
struct	msgb {
	struct	msgb	*b_next;
	struct	msgb	*b_prev;
	struct	msgb	*b_cont;
	char		*b_rptr;
	char		*b_wptr;
	struct datab 	*b_datap;
#ifdef HOWFAR
#ifdef STRDEBUG
	short		b_debug;	/* NOTE: to use this the whole kernel
						 must be recompiled */
#endif
#endif
};

typedef struct msgb mblk_t;
typedef struct datab dblk_t;

#ifdef HOWFAR
#ifdef STRDEBUG
#define	MM_Q	0x01		/* the message is on a queue */
#define	MM_FREE	0x02		/* the message is free */
#endif
#endif



/********************************************************************************/
/* 			Streams message types					*/
/********************************************************************************/


/*
 * Data and protocol messages (regular priority)
 */
#define	M_DATA		00		/* regular data */
#define M_PROTO		01		/* protocol control */
#define M_SPROTO	02		/* strippable protocol control */

/*
 * Control messages (regular priority)
 */
#define	M_BREAK		010		/* line break */
#define	M_SIG		013		/* generate process signal */
#define	M_DELAY		014		/* real-time xmit delay (1 param) */
#define M_CTL		015		/* device-specific control message */
#define	M_IOCTL		016		/* ioctl; set/get params */
#define M_SETOPTS	020		/* set various stream head options */
#define M_ADMIN		030		/* administration (out of stream) */

/*
 * Expedited messages (placed between regular priority and high
 * priority messages on a queue)
 */
#define M_EXPROTO	0100		/* expedited protocol control */
#define M_EXDATA	0101		/* expedited data */
#define M_EXSPROTO	0102		/* strippable expedited protocol control */
#define	M_EXSIG		0103		/* generate process signal */

/*
 * Control messages (high priority; go to head of queue)
 */
#define	M_IOCACK	0201		/* acknowledge ioctl */
#define	M_IOCNAK	0202		/* negative ioctl acknowledge */
#define	M_PCSIG		0203		/* generate process signal */
#define	M_FLUSH		0206		/* flush your queues */
#define	M_STOP		0207		/* stop transmission immediately */
#define	M_START		0210		/* restart transmission after stop */
#define	M_HANGUP	0211		/* line disconnect */
#define M_ERROR		0212		/* fatal error used to set u.u_error */


/*
 * Misc message type defines
 */
#define QNORM    0			/* normal messages */
#define QEXP  0100			/* expedited messages */
#define QPCTL 0200			/* priority cntrl messages */



/*
 *  IOCTL structure - this structure is the format of the M_IOCTL message type.
 */
struct iocblk {
	int 	ioc_cmd;		/* ioctl command type */
	ushort	ioc_uid;		/* effective uid of user */
	ushort	ioc_gid;		/* effective gid of user */
	int	ioc_id;			/* ioctl id */
	int	ioc_count;		/* count of bytes in data field */
	int	ioc_error;		/* error code */
	int	ioc_rval;		/* return value  */
};


/*
 * Options structure for M_SETOPTS message.  This is sent upstream
 * by driver to set stream head options.
 */
struct stroptions {
	short so_flags;		/* options to set */
	short so_readopt;	/* read option */
	short so_wroff;		/* write offset */
	short so_minpsz;	/* minimum read packet size */
	short so_maxpsz;	/* maximum read packet size */
	short so_hiwat;		/* read queue high water mark */
	short so_lowat;		/* read queue low water mark */
};

/* flags for stream options set message */

#define SO_ALL		077	/* set all options */
#define SO_READOPT	 01	/* set read opttion */
#define SO_WROFF	 02	/* set write offset */
#define SO_MINPSZ	 04	/* set min packet size */
#define SO_MAXPSZ	010	/* set max packet size */
#define SO_HIWAT	020	/* set high water mark */
#define SO_LOWAT	040	/* set low water mark */




/********************************************************************************/
/*		Miscellaneous parameters and flags				*/
/********************************************************************************/

/*
 * Default timeout in seconds for ioctls and close
 */
#define STRTIMOUT 15

/*
 * Stream head default high/low water marks 
 */
#define STRHIGH 512
#define STRLOW	256

/*
 * sleep priorities for input, output
 */
#define	STIPRI	PZERO+3
#define	STOPRI	PZERO+4

/*
 * The number of buffer classes streams supports
 */
#define NCLASS 9

/*
 * minimum size for which allocation retries are made (strgetblk)
 */
#define QBSIZE	64

/*
 * Value for infinite packet size specification
 */
#define INFPSZ	-1


/*
 * maximum ioctl data block size
 */
#define MAXIOCBSZ	1024


/*
 * Copy modes for tty and I_STR ioctls
 */
#define	U_TO_K 	01			/* User to Kernel */
#define	K_TO_K  02			/* Kernel to Kernel */


/*
 * Values for dev in open to indicate module open, clone open;
 * return value for failure.
 */
#define DEVOPEN 	0		/* open as a device */
#define MODOPEN 	1		/* open as a module */
#define CLONEOPEN	2		/* open for clone, pick own minor device */
#define FWDPUSH		3		/* forwarder push */
#define FWDPOP		4		/* forwarder pop */

#define OPENFAIL	-1		/* returned for open failure */
#define OPENSLEEP	-2		/* returned for open sleep (remote
					   forwarder modules only) */

/*
 *	Forwarder module ID range (do not use for non-forwarder modules)
 */

#define FORWARDERMIN	5000
#define FORWARDERMAX	5299

/*
 * Block allocation priorities
 */ 
#define BPRI_LO		1
#define BPRI_MED	2
#define BPRI_HI		3


/********************************************************************************/
/*	Defintions of Streams macros and function interfaces.			*/
/********************************************************************************/

/*
 * BLKALLOC defines (used in qinit)
 */
#ifdef u3b
#define BLKALLOC(SIZE)	kseg(SEG_RW,btop((int)SIZE))
#endif
#ifdef vax
#define	BLKALLOC(SIZE)	sptalloc(btoc(SIZE),PG_V|PG_KW,0)
#endif
#ifdef u3b2
#define BLKALLOC(SIZE)  kseg(btoc((int)SIZE))
#endif
#ifdef m68k
#ifdef PAGING
#define	BLKALLOC(SIZE)	sptalloc(btop(SIZE),PG_V|PG_RW,0)
#else
#define BLKALLOC(SIZE)  kseg(btoc((int)SIZE))
#endif PAGING
#endif

/*
 *  setqsched() defines (generate programmed interrupt to service queues)
 */
#define setqsched()     qrunflag = 1
#define qready()	qrunflag


/*
 * Definition of spl function needed to provide critical region protection
 * for streams drivers and modules.
 */
#ifdef u3b2
#define splstr() spl6()
#else
#ifdef m68k
#define splstr() spl6()
#else
#define splstr() spl5()
#endif
#endif


/*
 * Finding related queues
 */
#define	OTHERQ(q)	((q)->q_flag&QREADR? (q)+1: (q)-1)
#define	WR(q)		(q+1)
#define	RD(q)		(q-1)


/*
 * extract queue priority class of message
 */
#define queclass(bp) (bp->b_datap->db_type & (QPCTL | QEXP))


/*
 * free a queue pair
 */
#define freeq(q)	((q)->q_flag = WR(q)->q_flag = 0)


/*
 * declarations of common routines
 */
extern mblk_t *rmvb();
extern mblk_t *dupmsg();
extern mblk_t *copymsg();
extern mblk_t *allocb();
extern mblk_t *unlinkb();
extern mblk_t *dupb();
extern mblk_t *copyb();
extern mblk_t *getq();
extern int     putq();
extern queue_t *backq();
extern queue_t *allocq();
extern mblk_t *unlinkb();
extern mblk_t *unlinkmsg();

/*
 * shared or externally configured data structures
 */
extern struct stdata *streams;		/* table of streams */
extern queue_t	*queue;			/* table of queues */
extern mblk_t 	*mblock; 		/* table of msg blk desc */
extern dblk_t 	*dblock;	 	/* table of data blk desc */
extern unsigned short rbsize[];		/* map of class to real block size */
extern struct strstat strst;		/* Streams statistics structure */
extern queue_t	*qhead;			/* head of runnable queue list */
extern queue_t	*qtail;			/* tail of runnable queue list */
extern int strmsgsz;			/* maximum stream message size */
extern int nmblock;			/* number of msg blk desc */
extern char qrunflag;			/* for new function call interface */

extern int (*stream_open)();		/* stropen() hook */
extern int (*stream_close)();		/* strclose() hook */
extern int (*stream_read)();		/* strread() hook */
extern int (*stream_write)();		/* strwrite() hook */
extern int (*stream_ioctl)();		/* strioctl() hook */
extern int (*stream_run)();		/* stream scheduler */

#define ALLOC_LOCK(x)	
#define EXTERN_LOCK(x)	
#define INITLOCK(x,y)	
#define SPSEMA(x)	
#define SVSEMA(x)	
#define PSEMA(x,y)	
#define VSEMA(x,y)	

#define putnext(q, m) (*(q)->q_next->q_qinfo->qi_putp)((q)->q_next, m)
