/*
 *	Streams tty support
 *
 *	Copyright 1986 Unisoft Corporation of Berkeley CA
 *
 *
 *	UniPlus Source Code. This program is proprietary
 *	with Unisoft Corporation and is not to be reproduced
 *	or used in any manner except as authorized in
 *	writing by Unisoft.
 *
 */

#ifndef putnext
#define putnext(q, m) (*(q)->q_next->q_qinfo->qi_putp)((q)->q_next, m)
#endif
#define RCV_TIME RTO
#define XMT_DELAY TTIOW

struct ttx {
	queue_t	*t_q;		/* the read Q attatched to this device */
	mblk_t	*t_rm;		/* the current input buffer */
	mblk_t	*t_xm;		/* the current output buffer */
	int 	(*t_proc)();	/* our proc routine (required) */
	int 	(*t_ioctl)();	/* our ioctl routine (optional) */
	long	t_dev;		/* device id (for user) */
	caddr_t	t_addr;		/* device address (for user) */
	long	t_count;	/* input count */
	long	t_size;		/* input size */
	ushort	t_iflag;	/* input modes */
	ushort	t_oflag;	/* output modes */
	ushort	t_cflag;	/* control modes */
	ushort	t_lflag;	/* line discipline modes */
	short	t_state;	/* internal state */
};

extern int ttx_break();
extern int ttx_wputp();
extern int ttx_put();
extern int ttx_rsrvc();
extern int ttx_wsrvc();
