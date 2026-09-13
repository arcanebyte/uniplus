/*
 * @(#)ROOT	buserr.h	1.2	87/04/09	09:54:54
 * @(#)UniSoft	buserr.h	VVV.2.1.1
 */

/*
 * UniSoft Systems 68010 bus error structure
 */

struct buserr {
	/*
	 * start of UniSoft stack frame
	 */
	long	ber_regs[16];	/* saved registers */
	ushort	ber_dev;	/* device number */
	int	(*ber_func)();	/* called function address */
	/*
	 * start of 68010 stack frame
	 */
	ushort	ber_sr;		/* status register */
	long	ber_pc;		/* program counter */
	ushort	ber_format:4,	/* error type */
		ber_voffset:12;	/* vector offset */
	ushort	ber_sstat;	/* special status */
	long	ber_faddr;	/* fault address */
	ushort	ber_x1;		/* reserved */
	ushort	ber_dob;	/* data output buffer */
	ushort	ber_x2;		/* reserved */
	ushort	ber_dib;	/* data input buffer */
	ushort	ber_x3;		/* reserved */
	ushort	ber_iib;	/* instruction input buffer */
	ushort	ber_int[16];	/* internal information */
};

/*
 * UniSoft Systems 68020 bus error structure
 */

typedef struct sstat{
	ushort
		fc:1,	/* Fault on Stage C of Instruction Pipe */
		fb:1,	/* Fault on Stage B of Intruction Pipe */
		rc:1,	/* Rerun Flag for Stage C of Instruction Pipe */
		rb:1,	/* Rerun Flag for Stage B of Instruction Pipe */
		:3,	/* filler */
		df:1,	/* Fault/Rerun Flag for Data Cycle */
		rm:1,	/* Read-Modify-Write on Data Cycle */
		rw:1,	/* Read/Write for Data Cycle - 1=Read 0=Write */
		siz:2,	/* Size Code for Data Cycle */
		:1,	/* more filler */
		fnc:3;	/* Address Space for Data Cycle */
	} sstat_t;
typedef struct  intr1 {
	ushort
		:6,	/* filler */
		rw:1,	/* Read/Write for Pending Data Cycle */
		rmc:1,	/* Read-Modify-Write on Pending Data Cycle */
		siz:2,	/* Size Code for Data Cycle */
		fnc:3,	/* Address Space for Pending Data Cycle */
		prr:1,	/* Pending Rerun Bit */
		:2;	/* more filler */
	} intr1_t;

#define ber_sstat_68020		Ber_un.Ber_sstat
#define ber_ssw_68020		Ber_un.Ber_ssw

struct buserr_68020 {
	/*
	 * start of UniSoft stack frame
	 */
	long	ber_regs[16];	/* saved registers */
	ushort	ber_dev;	/* device number */
	int	(*ber_func)();	/* called function address */
	/*
	 * start of 68020 stack frame
	 */
	ushort	ber_sr;		/* status register */
	long	ber_pc;		/* program counter */
	ushort	ber_format:4,	/* error type */
		ber_voffset:12;	/* vector offset */
	intr1_t	ber_intr1;	/* internal register 1 */
	union {
		ushort	Ber_sstat;	 
		sstat_t	Ber_ssw;	/* bitfields */
		}Ber_un;		/* special status word */
	ushort  ber_inspc;	/* instruction pipe c */
	ushort  ber_inspb;	/* instruction pipe b */
	ulong	ber_faddr;	/* fault address */
	ulong	ber_intr2;	/* internal registers 2 */
	ulong	ber_dob;	/* data output buffer */
	long	ber_intr3;	/* internal registers 3 */
};

struct lbuserr {
	/*
	 * start of UniSoft stack frame
	 */
	long	ber_regs[16];	/* saved registers */
	ushort	ber_dev;	/* device number */
	int	(*ber_func)();	/* called function address */
	/*
	 * start of 68020 stack frame
	 */
	ushort	ber_sr;		/* status register */
	long	ber_pc;		/* program counter */
	ushort	ber_format:4,	/* error type */
		ber_voffset:12;	/* vector offset */
	intr1_t ber_intr1;	/* 0x8 internal register 1 */
	union {
		ushort	Ber_sstat;	 
		sstat_t	Ber_ssw;	/* bitfields */
		}Ber_un;		/* special status word */
	ushort  ber_inspc;	/* instruction pipe c */
	ushort  ber_inspb;	/* instruction pipe b */
	ulong	ber_faddr;	/* fault address */
	ulong	ber_intr2;	/* 0x14 internal registers, 2 words */
	ulong	ber_dob;	/* data output buffer */
	long	ber_intr3[2];	/* internal registers, 4 words */
	ulong	ber_baddr;	/* stage b address */
	long	ber_intr4;	/* internal resisters, 2 words */
	ulong	ber_dib;	/* data input buffer */
	ushort	ber_intr5[18];	/* internal registers, 18 words */
	ushort	ber_version:4,	/* CPU version number */
		ber_rest:12;	/* fill */
	ushort	ber_intr6[3];	/* internal registers, 3 words */
};

	/* ssw (special status word) codes */
#define	SSW_IMASK	0xC000		/* mask of instruction pipeline code */
#define SSW_SZMSK	0x30		/* mask size bits */
#define SSW_SZSHFT	4		/* size shift */
#define	SSW_READ	0x10		/* Special Status Word RW bit */
#define	SSW_RW		0x10		/* Special Status Word RW bit */
#define SSW_DF		0x100		/* data fault */
#define SSW_FC		0x8000		/* fault in stage C of inst. pipe */
#define SSW_FB		0x4000		/* fault in stage B of inst. pipe */
#define SSW_RC		0x2000		/* rerun stage C of inst. pipe */
#define SSW_RB		0x1000		/* rerun stage B of inst. pipe */

	/* internal register one bits */
#define INTR1_PRR	0x04		/* pending rerun bit in ber_intr1 */
#define INTR1_RW	0x200		/* read/write flag */
#define INTR1_SZMSK	0xC0		/* mask size bits */
#define INTR1_SZSHFT	6		/* size shift */

	/* 68020 exception frame formats */
#define FMT_FMASK	0xF		/* bus cycle stack frame mask */
#define FMT_SHORT	0xA		/* short bus cycle stack frame */
#define FMT_LONG	0xB		/* long bus cycle stack frame */

	/* 68020 status register codes */
#define	RPS_SUP		0x2000		/* superviser/user state */
#define	RPS_MAS		0x1000		/* master/interrupt state */
