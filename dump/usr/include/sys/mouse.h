/*
 * (C) 1983, 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Data format for the Lisa mouse.
 */

	/* Structure of mouse records read from /dev/mouse */
struct msrecord {
	ushort	mr_reset:1;	/* driver reset */
	ushort	mr_but:1;	/* button down */
	ushort	mr_ctl:1;	/* ctrl key down */
	ushort	mr_sft:1;	/* shift key down */
	ushort	mr_time:12;	/* time generated */
	char	mr_row;		/* row offset */
	char	mr_col;		/* column offset */
};

	/* Structure of IOCTL data passed and recieved from mouse driver */
struct msparms {
	ushort	mp_fdlay;	/* flush delay */
	ushort	mp_irate;	/* interrupt rate */
	ushort	mp_qlen;	/* records available */
	ushort	mp_flags;	/* control flags */
	ushort	mp_otime;	/* oldest records age */
	ushort	mp_ytime;	/* youngest records age */
};

	/* Breakdown of mp_flags above */
#define MF_VRATE 00000007	/* vertical retrace interrupt rate */
#define MF_BLK	 00000010	/* Block until data is available */
#define MF_SIG	 00000020	/* Signal (SIGMOUS) when data available */
#define MF_BUT	 00000040	/* Enable/Disable mouse button events */
#define MF_CTL	 00000100	/* Enable/Disable command key events */
#define MF_SFT	 00000200	/* Enable/Disable shift key events */
#define MF_VRT	 00000400	/* Enable/Disable verticle retrace signal */

	/* mp_fdlay above can take on value in the range of MF_MINDLAY to
	 * MF_MAXDLAY or zero which disables flushing.
	 * MF_FLUSH is the official way to flush the queue.
	 */
#define MF_MINDLAY	21
#define MF_MAXDLAY	4095
#define MF_FLUSH	1

	/* Special Constants appropriate to the mouse and display screen.
	 */
#define D_WIDTH	((ushort)90)
#define D_COLS	(D_WIDTH << 3)
#define D_ROWS	((ushort)360)
#define D_BLACK ((ulong)(-1))
#define D_WHITE ((ulong)(0))

#define	SIGMOUS	SIGUSR2	/* mouse interrupt */
