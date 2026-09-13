/*	@(#)tty.h	UniPlus 2.1.3	*/
/*	@(#)tty.h	1.4 of 10/19/86 (NFS) */

#include <sys/ttychars.h>

/*
 * A clist structure is the head of a linked list queue of characters.
 * The routines getc* and putc* manipulate these structures.
 */

struct clist {
	int	c_cc;		/* character count */
	struct cblock *c_cf;	/* pointer to first */
	struct cblock *c_cl;	/* pointer to last */
};

/* Macro to find clist structure given pointer into it	*/
#ifdef notdef
#define CMATCH(pointer)		(struct cblock *)(cfree + (pointer - cfree))
#else
/*
 * The following CMATCH macro assumes that the cblock structure size
 * is a power of two
 */
#define CMATCH(pointer)	(struct cblock *) \
	((int)cfree + (((int)pointer-(int)cfree)&(~(sizeof(struct cblock)-1))))
#endif

/* Character control block for interrupt level control	*/

struct ccblock {
	caddr_t	c_ptr;		/* buffer address	*/
	ushort	c_count;	/* character count	*/
	ushort	c_size;		/* buffer size		*/
};

/*
 * A tty structure is needed for each UNIX character device that
 * is used for normal terminal IO.
 */
#define	NCC	8
struct tty {
	struct	clist t_rawq;	/* raw input queue */
	struct	clist t_canq;	/* canonical queue */
	struct	clist t_outq;	/* output queue */
	struct	ccblock	t_tbuf;	/* tx control block */
	struct	ccblock t_rbuf;	/* rx control block */
	int	(* t_proc)();	/* routine for device functions */
	ushort	t_iflag;	/* input modes */
	ushort	t_oflag;	/* output modes */
	ushort	t_cflag;	/* control modes */
	ushort	t_lflag;	/* line discipline modes */
	int	t_state;	/* internal state */
	short	t_pgrp;		/* process group name */
	char	t_line;		/* line discipline */
	char	t_delct;	/* delimiter count */
	char	t_term;		/* terminal type */	
	char	t_tmflag;	/* terminal flags */
	char	t_col;		/* current column */
	char	t_row;		/* current row */
	char	t_vrow;		/* variable row */
	char	t_lrow;		/* last physical row */
	char	t_hqcnt;	/* no. high queue packets on t_outq */	
	char	t_dstat;	/* used by terminal handlers
						and line disciplines */
	short	t_index;	/* current tty[n] index (0 to n-1) */
	unsigned char	t_cc[NCC];	/* settable control chars */
	struct proc	*t_rsel;
	struct proc	*t_wsel;
	struct	ttychars t_chars;	/* tty */
#define	tt_erase	t_cc[VERASE]
#define	tt_kill		t_cc[VKILL]
#define	tt_intrc	t_cc[VINTR]
#define	tt_quitc	t_cc[VQUIT]
#define	tt_startc	t_chars.tc_startc
#define	tt_stopc	t_chars.tc_stopc
#define	tt_eofc		t_cc[VEOF]
#define	tt_brkc		t_cc[VEOL]
#define	tt_suspc	t_chars.tc_suspc
#define	tt_dsuspc	t_chars.tc_dsuspc
#define	tt_rprntc	t_chars.tc_rprntc
#define	tt_flushc	t_chars.tc_flushc
#define	tt_werasc	t_chars.tc_werasc
#define	tt_lnextc	t_chars.tc_lnextc
};

/*
 * The structure of a clist block
 */
#define	CLSIZE	26
struct cblock {
	struct cblock *c_next;
	char	c_first;
	char	c_last;
	char	c_data[CLSIZE];
};

/*
 * tty assist structure
 */
struct ttyptr {
	int tt_addr;		/* device address */
	struct tty *tt_tty;	/* tty structure */
};

/*
 * pseudo-tty ioctl structure
 */
struct	pt_ioctl {
	int	pt_flags;
	struct	proc *pt_selr;
	struct	proc *pt_selw;
	int	pt_send;
};

extern struct cblock *cfree;
extern struct cblock * getcb();
extern struct cblock * getcf();
extern struct clist ttnulq;
extern char partab[];

struct chead {
	struct cblock *c_next;
	int	c_size;
	int	c_flag;
};
extern struct chead cfreelist;

struct inter {
	int	cnt;
};

#define	QESC	0200	/* queue escape */
#define	HQEND	01	/* high queue end */

#define	TTIPRI	28
#define	TTOPRI	29

/* limits */
extern int ttlowat[], tthiwat[];
#define	TTYHOG	256
#define	TTXOLO	132
#define	TTXOHI	180
#define	TTECHI	80

/* Hardware bits */
#define	DONE	0200
#define	IENABLE	0100
#define	OVERRUN	040000
#define	FRERROR	020000
#define	PERROR	010000

/* Internal state */
#define	TIMEOUT		00000001	/* Delay timeout in progress */
#define	WOPEN		00000002	/* Waiting for open to complete */
#define	ISOPEN		00000004	/* Device is open */
#define	TBLOCK		00000010
#define	CARR_ON		00000020	/* Software copy of carrier-present */
#define	BUSY		00000040	/* Output in progress */
#define	OASLP		00000100	/* Wakeup when output done */
#define	IASLP		00000200	/* Wakeup when input done */
#define	TTSTOP		00000400	/* Output stopped by ctl-s */
#define	EXTPROC		00001000	/* External processing */
#define	TACT		00002000
#define	CLESC		00004000	/* Last char escape */
#define	RTO		00010000	/* Raw Timeout */
#define	TTIOW		00020000
#define	TTXON		00040000
#define	TTXOFF		00100000
#define	TS_RCOLL	00200000	/* collision in read select */
#define	TS_WCOLL	00400000	/* collision in write select */
#define	TS_NBIO		01000000	/* tty in non-blocking mode */
#define	TS_ASYNC	02000000	/* tty in async i/o mode */
#define TS_STOP		04000000	/* block background output */

#define	TSTATE_BITS	\
"\10\25TS_STOP\24ASYNC\23NBIO\22WCOLL\21RCOLL\20TTXOFF\17TTXON\16TTIOW\15RTO\
\14CLESC\13TACT\12EXTPROC\11TTSTOP\10IASLP\7OASLP\6BUSY\5CARR_ON\4TBLOCK\
\3ISOPEN\2WOPEN\1TIMEOUT"

/* l_output status */
#define	CPRES	0100000

/* device commands */
#define	T_OUTPUT	0
#define	T_TIME		1
#define	T_SUSPEND	2
#define	T_RESUME	3
#define	T_BLOCK		4
#define	T_UNBLOCK	5
#define	T_RFLUSH	6
#define	T_WFLUSH	7
#define	T_BREAK		8
#define	T_INPUT		9
#define	T_PARM		11
#define	T_SWTCH		12
/*
 * Terminal flags (set in t_tmflgs).
 */

#define SNL	1		/* non-standard new-line needed */
#define ANL	2		/* automatic new-line */
#define LCF	4		/* Special treatment of last col, row */
#define TERM_CTLECHO	010	/* Echo terminal control characters */
#define TERM_INVIS	020	/* do not send escape sequences to user */
#define QLOCKB		040	/* high queue locked for base level */
#define QLOCKI		0100	/* high queue locked for interrupts */
#define	TERM_BIT 0200		/* Bit reserved for terminal drivers. */
				/* Usually used to indicate that an esc*/
				/* character has arrived and that the  */
				/* next character is special.          */
				/* This bit is the same as the TM_SET  */
				/* bit which may never be set by a user*/
/*
 *	device report
 */
#define L_BUF		0
#define L_INTR		1
#define L_QUIT		2
#define L_BREAK		3
#define L_SWITCH	4
