/******************************************************************************
 *                                                                            *
 * TITLE     : XXX Definitions - BLT Externals.                               *
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 * CREATED   : 19th August 1985                                               *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) bltext.h 1.1@(#)" */

#define VISIBLE 1			/* As in window.h */

#define allones    0xffff

extern struct proc *heapproc;
extern form *cportbits;
extern rectangle cd;
extern int past7;
extern int reason;
extern int mmoved;

extern word rightmasks [];

extern short sx, sy;
extern rectangle dfrect;
extern int skew;
extern short dwords, swords;
extern word  mask1, mask2, pixelmask;
extern word  blackmask, whitemask, dmask, smask;
extern int hdir, vdir;
extern short adjust, dp_div_sp, sp_div_dp;
extern short sourcedelta, sourceraster, srcplanes;
extern short destdelta, destraster, destplanes;
extern short dmul,smul;
extern addrptr  sourcebits, destbits;
extern rectangle  intclip, temprect, tempclip;
extern point dbpoint;

extern DivList  *sourceform, *destform;  /* A super set of type form */
extern addrptr orhtonebits, htonebits;
extern rectangle  *srcrect, *destrect, *cliprect;
extern byte _mode, rubbish;
extern ctrans *cindex;

extern SetMasks();
extern SelectCopyLoop();
extern CopyLoop();
extern NullFn();

extern lint tempaccum, temp1accum;

