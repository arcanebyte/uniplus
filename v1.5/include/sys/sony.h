/*
 * Sony definitions
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

struct s_iob {
	char dum00;	unsigned char gobyte;
	char dum01;	unsigned char cmd;
#define mask cmd
	char dum02;	unsigned char drive;
	char dum03;	unsigned char side;
	char dum04;	unsigned char sector;
	char dum05;	unsigned char track;
	char dum06;	unsigned char unused;
	char dum07;	unsigned char confirm;
	char dum08;	unsigned char status;
	char dum09;	unsigned char format;
	char dum11;	unsigned char type;
	char dum12;	unsigned char error;
	char dum13;	unsigned char seek;
	char dum14[23];	unsigned char rom_id;		/* FCC031 */
	char dum15[15];	unsigned char diskin;		/* FCC041 */
	char dum16[7];	unsigned char drv_connect;	/* FCC049 */
	char dum17[21];	unsigned char intr_status;	/* FCC05F */
	char dum18[49];	unsigned char d_strt_bitslip;
	char dum19;	unsigned char d_end_bitslip;
	char dum20;	unsigned char d_checksum;
	char dum21;	unsigned char a_strt_bitslip;
	char dum22;	unsigned char a_end_bitslip;
	char dum23;	unsigned char a_wrng_sector;
	char dum24;	unsigned char a_wrng_track;
	char dum25;	unsigned char a_checksum;
	char dum26[88];	unsigned char cmd_index;
};

struct sn_hdr {
	unsigned short version;		/* unused */
	unsigned short volume;		/* unused */
	unsigned short fileid;		/* must be 0xAAAA for the boot block */
	unsigned short relpg;		/* unused */
	unsigned short dum1;		/* unused */
	unsigned short dum2;		/* unused */
};
#define	FILEID	0xAAAA			/* file id for boot block */

#define SNIOB ((struct s_iob *)0xFCC000)
#define SN_HDRBUF 0xFCC3E8		/* 12-byte header of 524-byte buffer */
#define SN_DATABUF 0xFCC400		/* real 512-byte buffer */
#define SN_CMD_BASE 0xFCC103		/* where the old commands are */

#define NSN 		1
#define SN_MAXBN	800

/* SONY COMMANDS */
#define SN_READ		0
#define SN_WRITE	1
#define SN_EJECT	2
#define SN_FORMAT	3
#define SN_VERIFY	4
#define SN_FMTTRK	5
#define SN_VFYTRK	6
#define SN_RDBRUT	7
#define SN_WRTBRUT	8
#define SN_CLAMP	9

#define SN_SHAKE	0x80
#define SN_CMD		0x81
#define SN_SEEK		0x83
#define SN_CALL		0x84
#define SN_CLRST	0x85
#define SN_STMASK	0x86
#define SN_CLRINT	0x87
#define SN_WAITRM	0x88
#define SN_GOAWAY	0x89
#define SN_CLEARMSK 	0x77

/* ROM revision number indicates twiggy vs. sony and fast vs. slow timer. */
#define	ROMMASK		0xE0		/* look at these bits only */
#define	ROMTW		0x80		/* 0=twiggies, 1=sony */
#define	ROMSLOW		0x20		/* for Lisa 2s, 0=fast, 1=slow */
