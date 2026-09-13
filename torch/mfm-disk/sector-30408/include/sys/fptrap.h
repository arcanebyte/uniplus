/*	@(#)fptrap.h	UniPlus VVV.2.1.1	*/
/*
 * MC68881 Floating-Point Coprocessor Trap Numbers
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#define	FPBSUN	48	/* Branch or Set on Unordered Condition */
#define	FPINEX	49	/* Inexact Result */
#define	FPDZ	50	/* Floating Point Divide by Zero */
#define	FPUNFL	51	/* Underflow */
#define	FPOPERR	52	/* Operand Error */
#define	FPOVFL	53	/* Overflow */
#define	FPSNAN	54	/* Signalling NAN (Not-A-Number) */
