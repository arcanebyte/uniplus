/* Profile Controller Definitions
 * Unless otherwise noted this information comes from an undated
 * document titled 'PROFILE COMMUNICATIONS PROTOCOL' and is apparently
 * either version 3.96 or 3.98
 */
	/* command codes */
#define PROREAD		0	/* read command */
#define PROWRITE	1	/* write command */
#define PROWRITEV	2	/* write/verify command */

	/* States (responses read from profile) */
	/* Kept in pd_state and pd_nxtst to determine what to do on intr */

#define SERR		0	/* pseudo state for err and initialization */
#define SCMD		1	/* waiting for a command */
#define SRDBLK		2	/* ready to read a block */
#define SWRTD		3	/* ready to receive for write data */
#define SVERD		4	/* ready to receive for verify data */
#define SFINI		5	/* pseudo state for write status pickup */
#define SPERFORM	6	/* waiting to do actual write or verify */
#define SSTOP		7	/* pseudo state for idle */

	/* Responses sent in reply to controller (messages) */
#define PGO		0x55	/* Proceed to next state (exec cmd) */
#define PIDL		0x00	/* Quit and return to idle loop */

	/* Polling Delays */
#define RSPTIME		0xC000		/* response timeout (~1 ms) */

	/* Operation status word breakdown.  Returned for each command
	 * and kept in pd_sbuf.
	 */
		/* Breakdown of status bits for Status Byte 1 */
#define FAIL	0x01		/* Operation unsuccessful */
#define TIMEOUT 0x04		/* Couldn't read header after 9 revs */
#define CRCERR	0x08		/* CRC error while trying to read/write */
#define SEEKERR	0x10		/* Unable in 3 tries to read 3 consec sec*/
#define NOTABLE	0x20		/* Data table not in RAM (spare updated)*/
#define ABORTED	0x40		/* >532 bytes sent or no spare table read*/
#define	GOAHEAD	0x80		/* If the profile received 0x55 last time*/

		/* Breakdown of status bits for Status Byte 2 */
#define BADSEEK 0x02		/* Seek to wrong track occured */
#define SPARED	0x04		/* set if sparing occured */
#define RDSTATS	0x08		/* Ctrl unable to read status sector */
#define BLKTABO	0x10		/* Bad block table overflow: > 100 bads */
#define SECTABO	0x40		/* Spared Sector table overflow >32 secs*/
#define SEEKER2 0x80		/* unable in 1 try to read 3 consec secs*/

		/* Breakdown of status bits for Status Byte 3 */
#define PARITYE 0x01		/* Parity error */
#define BADRESP	0x02		/* If Ctrl gave a bad response */
#define WASREST	0x04		/* If Ctrl was reset */
#define BLKIDMM	0x20		/* Block id at end of sector mismatch */
#define BLKNINV	0x40		/* Block number invalid */
#define RESETP	0x80		/* Ctrl has been reset */
		/* Status byte 4 is the # of errs rereading after read err*/

		/* Mask to remove redundant bits from status word */
#define STATMSK	0x8000

		/* Controller Command Format */
struct cmd {
	char	p_cmd;		/* command register */
	char	p_high;		/* high block byte */
	char	p_mid;		/* mid block byte */
	char	p_low;		/* low block byte */
	char	p_retry;	/* retry count */
	char	p_thold;	/* threshold count */
};

#define	MAXBLOCK 19455	/* Last sector on disk */
#define SECSIZE	512	/* Number of bytes per sector */
#define LOG_SS	9	/* Log (base 2) (SECSIZE) */
#define NSEC	16	/* Number of sectors/track */
#define NRETRY	10	/* Number of chksum error retries */

extern struct device_d *pro_da[];

#define logical(x)	(minor(x) & 7)		/* eight logicals per phys */
#define interleave(x)	(minor(x) & 0x8)	/* interleave bit for swaping */
#define physical(x)	((minor(x) & 0xF0) >> 4)/* 10 physical devs */

#define ul_forw av_back

/* Since there my be up to ten devices active the driver locals have been
 * collected into this structure to allow easy swithing between them.
 */
struct prodata {
	struct device_d	*pd_da;		/* physical dev pointer */
	struct cmd	pd_cmdb;	/* command buffer */
	struct buf	*pd_actv;	/* ptr to active buf */
	daddr_t		pd_blkno;	/* present block being transferred */
	daddr_t		pd_limit;	/* max blk number for this log dev */
	caddr_t		pd_addr;	/* present core address */
	long		pd_bcount;	/* present count */
	long		pd_sbuf;	/* Status */
	long		pd_flags;	/* mode of operation flags */
	short		pd_state;	/* state of controller */
	short		pd_nxtst;	/* next state scheduled for disk */
	short		pd_unit;	/* phys unit number of this dev */
	short		pd_offline;	/* wait for disk to come online again */
	char		*pd_err;	/* err mesg for last error */
} prodata[NPPDEVS];

/* Breakdown of bits in pd_flags above: */
#define NOCHKSUM	0x01	/* diasble checksums failure on reads */
#define NOPARITY	0x02	/* disable fail on parity errors */

extern char pro_secmap[];

#ifndef NODEBUG
#define DEBUG(x) printf x
#else
#define DEBUG(x)
#endif

