/*
 * Copyright 1982 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
 * Header file for standalone package
 */

/*
 * Debug control for standalone package
 */
extern int debug;

#define D_INOUT	1	/* in/out routine trace */
#define D_SLOWO	2	/* slow printf output */
#define D_GETC	4	/* trace getc fetches */
#define D_RW	8	/* trace read and write blocks */

/*
 * io block: includes an
 * inode, cells for the use of seek, etc,
 * and a buffer.
 * A file name is <dv_name>(<i_unit>,<i_boff>)<file-name> .
 * A device name is <dv_name>(<i_unit>,<i_boff>) .
 */
struct  iob {
        char    i_flgs;
        struct  inode i_ino;
        int     i_Fsdev;		/* file system type */
        int     i_unit;			/* physical unit number */
	dev_t	i_device;		/* block major device number */
        daddr_t i_boff;			/* physical block offset */
        daddr_t i_cyloff;		/* used only with i_bn==0 (see below) */
        off_t   i_offset;		/* byte offset from i_boff */
        daddr_t i_bn;			/* starting physical block number */
        char    *i_ma;			/* memory address */
        int     i_cc;			/* character count */
        char    i_buf[SBUFSIZE];
};

#define	i_bblk	i_cyloff	/* flag for boot header field for block 0 */
#define	BOOTDISK	0xFFAA	/* i_bblk value for boot disk */
#define F_BOOT  020		/* flag value used only within driver */

#define F_READ  01
#define F_WRITE 02
#define F_ALLOC 04
#define F_FILE  010

#define	PBSIZE	512		/* physical block size */
#define	PBSHIFT	9

/*
 * dev switch
 */
struct devsw {
        char    *dv_name;
        int     (*dv_strategy)();
        int     (*dv_open)();
        int     (*dv_close)();
};
extern struct devsw devsw[];

/*
 * table of special boot blocks
 */
struct block0 {
        char    *dv_name;
        int	(*bblk)();
};
extern struct block0 block0[];

/*
 * table for streaming tape information
 */
struct streamer {
        char    *dv_name;
        int	(*szdisk)();		/* get size of disk */
	int	(*weof)();		/* write end-of-file mark */
	int	(*reof)();		/* read end-of-file mark */
	int	(*rewind)();		/* rewind tape */
	int	(*erase)();		/* erase tape */
	int	(*advance)();		/* advance file marks */
	int	(*reten)();		/* retension tape */
        int	(*totape)();		/* copy to tape */
        int	(*frtape)();		/* copy from tape */
};
extern struct streamer streamer[];

#define NBUFS  4
#define NFILES  4

char    b[NBUFS][SBUFSIZE];
daddr_t blknos[NBUFS];

struct  iob iob[NFILES];

#define READ    F_READ
#define WRITE   F_WRITE

#define VIDADDR	*(char *)(STDIO+0xE800)		/* Video address latch */
#define TOTMEM	*(long *)(0x2A8)		/* Total amount of RAM */
#define MEMSTART *(long *)(0x2A4)		/* Phys mem start */

#define SBOOT	101	/* starting block for boot program */
#define NBLKS	100	/* number of blocks in boot or ser'n program */
