/*	@(#)buserr.h UniPlus+ v.0.1.1 */
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
	ushort	ber_voffset:12,	/* vector offset */
		ber_format:4;	/* error type */
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
