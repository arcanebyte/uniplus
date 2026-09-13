/*
 * RECONSTRUCTED -- not an original UniSoft file (2026).
 * Record format for /dev/speaker, as documented in speaker(5L) of the
 * UniPlus+ Lisa-specific manual (October 1984).
 * See ../PROVENANCE.md.
 */

struct speaker {
	ushort	sk_wavlen;	/* wave length, microseconds between transitions */
	ushort	sk_duration;	/* duration, clock ticks (1/60 s) */
	ushort	sk_volume;	/* volume, low 3 bits used */
};

#define	MINWLEN	8		/* speaker(5L): shorter values round up to 8 */
#define	MAXWLEN	8191		/* speaker(5L): maximum; also used as a mask in sk.c */
