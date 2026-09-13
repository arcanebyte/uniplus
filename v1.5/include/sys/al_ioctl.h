/*
 * RECONSTRUCTED -- not an original UniSoft file (2026).
 * Apple Lisa specific ioctl requests (console, mouse, Sony floppy).
 * See ../PROVENANCE.md for sources and confidence of each definition.
 *
 * Names and meanings: console(5L), mouse(5L), eject(1L) in the
 * UniPlus+ Lisa-specific manual (October 1984).
 * Values: decoded from the ioctl() calls in /usr/bin/setparams and
 * /usr/bin/eject on the 10 MB UniPlus disk, unless marked GUESS.
 */

#define	AL	('L'<<8)

/* set console parameters                     setparams: CONFIRMED */
#define	AL_SBVOL	(AL|1)		/* bell volume */
#define	AL_SBPITCH	(AL|2)		/* bell pitch */
#define	AL_SBTIME	(AL|3)		/* bell duration */
#define	AL_SDIMTIME	(AL|4)		/* time before dimming screen */
#define	AL_SDIMCONT	(AL|5)		/* screen contrast when dim */
#define	AL_SDIMRATE	(AL|6)		/* rate of dimming */
#define	AL_SCONTRAST	(AL|7)		/* screen contrast when bright */
#define	AL_SREPWAIT	(AL|8)		/* wait before repeating keys */
#define	AL_SREPDELAY	(AL|9)		/* delay between key repeats */
#define	AL_REVVIDEO	(AL|10)		/* >0 bright, 0 black, <0 toggle */

/* get console parameters                     setparams: CONFIRMED */
#define	AL_GBVOL	(AL|17)
#define	AL_GBPITCH	(AL|18)
#define	AL_GBTIME	(AL|19)
#define	AL_GDIMTIME	(AL|20)
#define	AL_GDIMCONT	(AL|21)
#define	AL_GDIMRATE	(AL|22)
#define	AL_GCONTRAST	(AL|23)
#define	AL_GREPWAIT	(AL|24)
#define	AL_GREPDELAY	(AL|25)

/* physical address of the bitmap display.  No surviving program uses it:
 * GUESS, the next value after the get-parameter block. */
#define	AL_GBMADDR	(AL|26)

/* mouse(5L): get/set struct msparms (sys/mouse.h).  No surviving program
 * uses them: GUESS, placed just below AL_EJECT. */
#define	AL_GMOUSE	(AL|32)
#define	AL_SMOUSE	(AL|33)

/* eject the Sony floppy                  eject, tar: CONFIRMED */
#define	AL_EJECT	(AL|34)
