/*
 * Priam datatower definitions
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

struct pm_base {
	/* read registers */
	uchar_t	status;	char xx0;	/* status register */
	union {
		ushort	WData;		/* r/w disk data register */
		uchar_t	PData[2];	/* write only high byte for packet */
	} Data;
#define data	Data.WData		/* normal word access */
#define pdata	Data.PData[0]		/* byte access for packet (format) */
	uchar_t	r0;	char xx1;	/* result 0 */
	uchar_t	r1;	char xx2;	/* result 1 */
	uchar_t	r2;	char xx3;	/* result 2 */
	uchar_t	r3;	char xx4;	/* result 3 */
	uchar_t	r4;	char xx5;	/* result 4 */
	uchar_t	r5;	char xx6;	/* result 5 */
};

	/* write registers */
#define	cmdreg	status			/* command register */
#define	p0	r0			/* parameter 0 */
#define	p1	r1			/* parameter 1 */
#define	p2	r2			/* parameter 2 */
#define	p3	r3			/* parameter 3 */
#define	p4	r4			/* parameter 4 */
#define	p5	r5			/* parameter 5 */

	/* channel definitions */
#define DISK	0
#define NOTUSED	1
#define TAPE	2
#define HOST	3
#define SPECIAL	4

	/* flag definitions */
#define	IDLING		0
#define	INITING		1
#define	DREADING	2
#define	DWRITING	3
#define	TREADING	4
#define	TWRITING	5
#define TCONT		6

	/* read parity register (pmpaddr, lo select space) */
#define	parity	status
#define	PMPERROR	0x80		/* extract parity FF on reads */

/* status register bits */
#define	CMD_REJECT	0x80		/* command reject */
#define	CMD_DONE	0x40		/* command completion request */
#define	CMD_SDONE	0x20		/* special command completion */
#define	BTR_INT		0x10		/* block transfer - interrupt */
#define	ISR_BUSY	0x08		/* interface busy */
#define	DTREQ		0x04		/* data transfer request */
#define	RW_REQ		0x02		/* r/w request */
#define	DBUS_ENA	0x01		/* data bus enabled */

/* commands */
#define	PMRESET		0x7		/* software reset */
#define	PMSMODE		0x8		/* set mode */
#define	PMRMODE		0x9		/* read mode */
#define	PMSETP		0xC		/* specify parameters */
#define	PMSPINUPW	0x82		/* sequence up disk and wait */
#define	PMRDEVPMS	0x85		/* read device parms */
#define	PMSPINDN	0x81		/* sequence down disk */
#define	PMNWRITE	0x42		/* write data, retries disabled */
#define	PMNREAD		0x43		/* read data, retries disabled */
#define	PMWRITE		0x52		/* write data, retries enabled */
#define	PMREAD		0x53		/* read data, retries enabled */
#define PMWMRK		0x62		/* write tape file mark */
#define PMRMRK		0x63		/* read tape file mark */
#define PMVERIFY	0x64		/* verify tape mark */
#define PMREWIND	0x6A		/* rewind tape */
#define PMERASE		0x6F		/* erase tape */
#define	PMCBTI		0x01		/* clear "block transfer intr" */
#define	PMPKTXFER	0xB0		/* transfer packet (used for format) */
#define	PMPKTRST	0xB8		/* read packet status */
#define PMPKTRES	0xB1		/* resume packet operation */
#define PMPKTABRT	0xBF		/* abort packet */
#define PMADVANCE	0xC0		/* advance file marks */
#define PMRETEN		0xC1		/* retension tape */

/* results */
#define	NPMRES		6		/* number of result registers */
#define	PMCTYPE		0x30		/* error completion type mask (r0) */
#define	PMICOMP		0x10		/* init complete (r0) */
#define	PMECCERR	0x11		/* ECC error (r0) */
#define	PMDTIMOUT	0x33		/* timeout error (r0) */
#define	PMNH(r)		(int)((r) >> 4)	/* no. heads (r1 after PMSPINUP) */
#define	PMNC(r1,r2)	(int)((((r1)&0xF)<<8)|(r2))/* no. cyls (r1,r2 after PMSPINUP) */
#define	PMNS(r)		(int)(r)	/* no. heads (r3 after PMSPINUP) */

/* parameters */
#define	PMLOGSECT	0x40		/* log. sector mode (p1) */
#define	PMPSEL1		0x01		/* parm select 1 (p1) */
#define	PM1PARMS	0x07		/* parm select 1 parameters (p2)
					   disable high performance /
					   block transfer intr enabled /
					   block transfer timer disable */
#define	PMPSEL0		0x00		/* parm select 0 (p1) */
#define	PM0PARMS	0x03		/* parm select 0 parameters (p2)
					   auto. defect management enabled /
					   init. complete intr disabled /
					   command complete intr enabled /
					   enable parity */

#define PMCARD		0x4000		/* offset to next card */
#define PMHISEL		0x2000		/* offset to hi select space */
	/* lo sel space (boot ROM and parity) */
#define pmlosel(u)	(0xFC0000 + ((u) * PMCARD))
#define pmpaddr(u)	((struct pm_base *)pmlosel(u))
	/* hi sel space (data handling) */
#define pmaddr(u)	((struct pm_base *)(pmlosel(u) + PMHISEL))

/* interface control bits (added to pmaddr) */
#define	PM_N		0x200		/* no device cycle */
#define	PM_I		0x100		/* interrupt enable */
#define	PM_B		0x080		/* byte mode */
#define	PM_W		0x040		/* waiting (while U wait) */
#define	PM_P		0x020		/* parity checking enabled */

#define	pmNaddr(p)	((struct pm_base *)(((long)p) + PM_N))
#define	pmIaddr(p)	((struct pm_base *)(((long)p) + PM_I))
#define	pmBaddr(p)	((struct pm_base *)(((long)p) + PM_B))
#define	pmWaddr(p)	((struct pm_base *)(((long)p) + PM_W))
#define	pmPaddr(p)	((struct pm_base *)(((long)p) + PM_P))
#define	pmBWaddr(p)	((struct pm_base *)(((long)p) + PM_B + PM_W))
#define	pmBIaddr(p)	((struct pm_base *)(((long)p) + PM_B + PM_I))
#define	pmBWIaddr(p)	((struct pm_base *)(((long)p) + PM_B + PM_W + PM_I))

/*
 * Format disc packet parameters structure
 */
struct pmfmtparms {
	unsigned char	pm_opcode;	/* operation code = PMFMT */
#define	PMFMT		0x02		/* opcode for format */
	unsigned char	pm_devsel;	/* device select = 0 (no daisy chaining) */
	unsigned char	pm_scntl;	/* sector control: FBD, media type */
#define	PMFBD		0x80		/* fill byte disable */
	unsigned char	pm_fill;	/* sector control: fill byte */
	unsigned short	pm_ssize;	/* sector control: sector size */
	unsigned char	pm_dcntl;	/* defect control: DMD, # spare sectors */
#define	PMDMD		0x80		/* defect mapping disable */
	unsigned char	pm_ncyl;	/* defect control: # alternate cylinders */
	unsigned char	pm_cif;		/* interleave control: cyl interleave factor */
	unsigned char	pm_hif;		/* interleave control: head interleave factor */
	unsigned char	pm_sif;		/* interleave control: sect interleave factor */
	unsigned char	pm_sitl;	/* interleave control:
						   sector interleave table length */
};

/*
 * Packet status report structure (used by packet-based format command)
 */
struct pmfmtstat {
	unsigned char	pm_pid;		/* packet ID */
	unsigned char	pm_pstate;	/* packet state */
#define	PMPKTCOMP	0x0D		/* packet completed, not resumeable */
	unsigned char	pm_tdf;		/* termination device flag */
	unsigned char	pm_pristat;	/* primary termination status */
	unsigned char	pm_xx[32];	/* more status - unused */
};

/*
 * Header structure
 *	Used by the lisa office system.  It's 4 bytes longer than the
 *	header for the profile, to include the size of the disk, but
 *	otherwise the same (see profile.h).
 */
struct pmheader {
	unsigned short	pm_version;	/* unused */
	unsigned short	pm_volume;	/* unused */
	unsigned short	pm_fileid;	/* must be 0xAAAA for the boot block */
	unsigned short	pm_cs1;		/* unused */
	unsigned int	pm_cs2;		/* unused */
	unsigned short	pm_relpage;	/* unused */
	unsigned int	pm_nblk;	/* unused */
	unsigned short	pm_pblk;	/* unused */
	unsigned int	pm_size;	/* unused - size of disk */
};
#define HDRSIZE (sizeof (struct pmheader))
#define	FILEID	0xAAAA			/* file id for boot block */

#define	TIMELIMIT	500000		/* timeout of about 5 secs. */

#define NPM 		3
