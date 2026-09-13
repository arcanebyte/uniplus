/* @(#)reg.h	6.1 */
/*
 * Location of the users' stored registers relative to R0.
 * Usage is u.u_ar0[XX].
 */

#define	R0	(0)
#define	R1	(1)
#define	R2	(2)
#define	R3	(3)
#define	R4	(4)
#define	R5	(5)
#define	R6	(6)
#define	R7	(7)
#define AR0	(8)
#define	AR1	(9)
#define	AR2	(10)
#define	AR3	(11)
#define	AR4	(12)
#define	AR5	(13)
#define	AR6	(14)
#define	FP	(14)
#define	AR7	(15)
#define	SP	(15)
#define	PC	(18)
#define	RPS	(17)

#define	RPS_ALLCC	0x1F

/*
 * Label variable (label_t) index locations
 * Usage is u.u_rsav[XX].
 */
#define	LA7	(11)

/*
 * Definition of the argument stack passed
 * by pointer to an interrupt routine.
 */
struct args {
	int	a_regs[16];	/* saved registers */
	dev_t	a_dev;		/* device number */
	int	(*a_faddr)();	/* called function address */
	short	a_ps;		/* original status register */
	caddr_t	a_pc;		/* original pc */
};
