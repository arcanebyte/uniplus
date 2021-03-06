/*
 * Copyright 1982 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
 * Parallel Port interface definitions
 */

struct device_d {
	char d_f0[1];	char	d_irb;		/* I/O register B */
	char d_f1[7];	char	d_ira;		/* I/O register A */
	char d_f2[7];	char	d_ddrb;		/* Data Dir reg B */
	char d_f3[7];	char	d_ddra;		/* Data Dir reg A */
	char d_f4[7];	char	d_t1cl;		/* T1 low Latches Counter */
	char d_f5[7];	char	d_t1ch;		/* T1 hi Latches Counter */
	char d_f6[7];	char	d_t1ll;		/* T1 low Latches */
	char d_f7[7];	char	d_t1lh;		/* T1 hi Latches */
	char d_f8[7];	char	d_t2cl;		/* T2 low Latches Counter */
	char d_f9[7];	char	d_t2ch;		/* T2 hi Latches Counter */
	char d_fa[7];	char	d_sr;		/* Shift Register */
	char d_fb[7];	char	d_acr;		/* Aux Ctrl Reg */
	char d_fc[7];	char	d_pcr;		/* Perif ctrl Reg */
	char d_fd[7];	char	d_ifr;		/* Int Flag Reg */
	char d_fe[7];	char	d_ier;		/* Int Ena Reg */
	char d_ff[7];	char	d_aira;		/* Alt d_ira (no handshake)*/
};

#define PPADDR (struct device_d *)(STDIO + 0xD900)

	/* Breakdown of d_irb byte (above) [pg. 45 LHRM APR-82]*/
#define OCD	0x01	/* Open cable detect (paper empty)*/
#define BSY	0x02	/* Busy bit (handshake line) */
#define DEN	0x04	/* Disk Enable (buffers which drive interface lines)*/
#define DRW	0x08	/* Direction (Disk Read/Write) off = write to disk */
#define CMD	0x10	/* CMD bit (printer ON/OFF line)*/
#define	PCHK	0x20	/* enable parity check(input for printer/out for disk)*/
#define DSKDIAG	0x40
#define	WCNT	0x80

#define NPPDEVS 10		/* number of parallel port devices */
#define	PPOK(x)	(((x)>0) && ((x)<9) && ((x)!=3) && ((x)!=6))	/* 1,2,4,5,7,8 */
#define	PPSLOT(x)	((x)/3)	/* expansion slot number from physical unit */
