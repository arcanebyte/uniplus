/*	@(#)values.h	6.2		*/
/*	@(#)values.h	6.1		*/
/*	@(#)values.h	1.3	UNIX System V/68	*/
/*	@(#)values.h	2.4 83/07/08	*/
#ifndef BITSPERBYTE
/* These values work with any binary representation of integers
 * where the high-order bit contains the sign. */

/* a number used normally for size of a shift */
#if gcos
#define BITSPERBYTE	9
#else
#define BITSPERBYTE	8
#endif
#define BITS(type)	(BITSPERBYTE * (int)sizeof(type))

/* short, regular and long ints with only the high-order bit turned on */
#define HIBITS	((short)(1 << BITS(short) - 1))
#define HIBITI	(1 << BITS(int) - 1)
#define HIBITL	(1L << BITS(long) - 1)

/* largest short, regular and long int */
#define MAXSHORT	((short)~HIBITS)
#define MAXINT	(~HIBITI)
#define MAXLONG	(~HIBITL)

/* various values that describe the binary floating-point representation
 * _EXPBASE	- the exponent base
 * DMAXEXP 	- the maximum exponent of a double (as returned by frexp())
 * FMAXEXP 	- the maximum exponent of a float  (as returned by frexp())
 * DMINEXP 	- the minimum exponent of a double (as returned by frexp())
 * FMINEXP 	- the minimum exponent of a float  (as returned by frexp())
 * MAXDOUBLE	- the largest double
			((_EXPBASE ** DMAXEXP) * (1 - (_EXPBASE ** -DSIGNIF)))
 * MAXFLOAT	- the largest float
			((_EXPBASE ** FMAXEXP) * (1 - (_EXPBASE ** -FSIGNIF)))
 * MINDOUBLE	- the smallest double (_EXPBASE ** (DMINEXP - 1))
 * MINFLOAT	- the smallest float (_EXPBASE ** (FMINEXP - 1))
 * DSIGNIF	- the number of significant bits in a double
 * FSIGNIF	- the number of significant bits in a float
 * DMAXPOWTWO	- the largest power of two exactly representable as a double
 * FMAXPOWTWO	- the largest power of two exactly representable as a float
 * _IEEE	- 1 if IEEE standard representation is used
 * _DEXPLEN	- the number of bits for the exponent of a double
 * _FEXPLEN	- the number of bits for the exponent of a float
 * _HIDDENBIT	- 1 if high-significance bit of mantissa is implicit
 * LN_MAXDOUBLE	- the natural log of the largest double  -- log(MAXDOUBLE)
 * LN_MINDOUBLE	- the natural log of the smallest double -- log(MINDOUBLE)
 */
#if u3b || u3b5
#define MAXDOUBLE	1.79769313486231470e+308
#define MAXFLOAT	((float)3.40282346638528860e+38)
#define MINDOUBLE	4.94065645841246544e-324
#define MINFLOAT	((float)1.40129846432481707e-45)
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
#define FMINEXP	(-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))
#endif
#if m68k
/* The following declarations are a kludge to get around the problems
** of cross-compiling float/double constants.  The constant in question
** is the maximum float/double value.  Since the VAX has a smaller range
** than the M68000, it cannot convert MAXFLOAT using the native atof().
** What we do here is to announce external variables whose values
** are set explicitly in an assembly language file.
*/
    extern double _MaXdOuB, _MiNdOuB;
    extern float _MaXfLoA, _MiNfLoA;

#define	MAXFLOAT	_MaXfLoA
#define	MAXDOUBLE	_MaXdOuB
#define	MINFLOAT	_MiNfLoA
#define	MINDOUBLE	_MiNdOuB
/* #define MAXFLOAT	((float)3.40282346638528860e+38) */
/* #define MAXDOUBLE	((double)1.79769313486231470e+308) */
/* MINFLOAT/MINDOUBLE are defined to give the smallest non-denormalized
** value.
*/
/* #define MINDOUBLE	((double)4.4501477170144023e-308) */
/* #define MINFLOAT	((float)MINDOUBLE) */
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP) + 1)
#define FMINEXP	(-(FMAXEXP) + 1)
#define CUBRTHUGE	6981463519622.333
#define INV_CUBRTHUGE	1.4323644278729908e-13

/* an IEEE std. infinity occurs when exp=max=0x7ff and fractional part == 0 */
#define INF(X) \
(((union { double d; struct { unsigned :1, e:11, f1:20, f2:32; } s; } *)&X)->s.e == 0x7ff) \
&&  \
(((union { double d; struct { unsigned :1, e:11, f1:20, f2:32; } s; } *)&X)->s.f1 == 0) \
&& \
(((union { double d; struct { unsigned :1, e:11, f1:20, f2:32; } s; } *)&X)->s.f2 == 0)
/* The following detects an infinity and changes it to a large number
   with the correct sign.  The large number is "MAXFLOAT."
   This is consistent with the way "libm" handles overflow, 
   but one wonders why it can't be set to MAXDOUBLE instead. */

#define CHNGinf(X)	if (INF(X)) \
				X = (X>0.0) ? MAXFLOAT : -MAXFLOAT;
#endif
#if pdp11 || vax
#define MAXDOUBLE	1.701411834604692293e+38
#define MAXFLOAT	((float)1.701411733192644299e+38)
/* The following is kludged because the PDP-11 compilers botch the simple form.
   The kludge causes the constant to be computed at run-time on the PDP-11,
   even though it is still "folded" at compile-time on the VAX. */
#define MINDOUBLE	(0.01 * 2.938735877055718770e-37)
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	1
#define DMINEXP	(-DMAXEXP)
#define FMINEXP	(-FMAXEXP)
#endif
#if gcos
#define MAXDOUBLE	1.7014118346046923171e+38
#define MAXFLOAT	((float)1.7014118219281863150e+38)
#define MINDOUBLE	2.9387358770557187699e-39
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	0
#define DMINEXP	(-(DMAXEXP + 1))
#define FMINEXP	(-(FMAXEXP + 1))
#endif
#if u370
#define _LENBASE	4
#else
#define _LENBASE	1
#endif
#define _EXPBASE	(1 << _LENBASE)
#define _FEXPLEN	8
#define DSIGNIF	(BITS(double) - _DEXPLEN + _HIDDENBIT - 1)
#define FSIGNIF	(BITS(float)  - _FEXPLEN + _HIDDENBIT - 1)
#if m68k		/* different because BITS(long) > DSIGNIF */
/***TEMP #define	DMAXPOWTWO	((double)(1L << DSIGNIF - 1)) */
#define DMAXPOWTWO	((double)(1L << BITS(long) - 2) * \
				(1L << DSIGNIF - BITS(long) + 1))
#else
#define DMAXPOWTWO	((double)(1L << BITS(long) - 2) * \
				(1L << DSIGNIF - BITS(long) + 1))
#endif
#define FMAXPOWTWO	((float)(1L << FSIGNIF - 1))
#define DMAXEXP	((1 << _DEXPLEN - 1) - 1 + _IEEE)
#define FMAXEXP	((1 << _FEXPLEN - 1) - 1 + _IEEE)
#define LN_MAXDOUBLE	(M_LN2 * DMAXEXP)
#define LN_MINDOUBLE	(M_LN2 * (DMINEXP - 1))
#define H_PREC	(DSIGNIF % 2 ? (1L << DSIGNIF/2) * M_SQRT2 : 1L << DSIGNIF/2)
#define X_EPS	(1.0/H_PREC)
#define X_PLOSS	((double)(long)(M_PI * H_PREC))
#define X_TLOSS	(M_PI * DMAXPOWTWO)
#define M_LN2	0.69314718055994530942
#define M_PI	3.14159265358979323846
#define M_SQRT2	1.41421356237309504880
#define MAXBEXP	DMAXEXP /* for backward compatibility */
#define MINBEXP	DMINEXP /* for backward compatibility */
#define MAXPOWTWO	DMAXPOWTWO /* for backward compatibility */
#endif
