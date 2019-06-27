/* @(#)clist.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/tty.h"

getc(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int c;

#ifdef mc68000
	/*
	 * Note use of d6
	 */
#ifndef lint
	register int s = 0;
#endif

	asm("	movw	sr,d6");
	asm("	movw	#0x2600,sr");
#else
	register int s;
	s = spl6();
#endif

	if (p->c_cc > 0) {
		p->c_cc--;
		bp = p->c_cf;
		c = bp->c_data[bp->c_first++]&0377;
		if (bp->c_first == bp->c_last) {
			if ((p->c_cf = bp->c_next) == NULL)
				p->c_cl = NULL;
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
			if (cfreelist.c_flag) {
				cfreelist.c_flag = 0;
				wakeup((caddr_t)&cfreelist);
			}
		}
	} else
		c = -1;
#ifdef mc68000
	asm("	movw	d6,sr");
#else
	splx(s);
#endif
	return(c);
}

putc(c, p)
register struct clist *p;
{
	register struct cblock *bp, *obp;

#ifdef mc68000
	/*
	 * Note use of d7
	 */
#ifndef lint
	register int s = 0;
#endif

	asm("	movw	sr,d7");
	asm("	movw	#0x2600,sr");
#else
	register int s;
	s = spl6();
#endif

	if ((bp = p->c_cl) == NULL || bp->c_last == (char)cfreelist.c_size) {
		obp = bp;
		if ((bp = cfreelist.c_next) == NULL) {
#ifdef mc68000
			asm("	movw	d7,sr");
#else
			splx(s);
#endif
			return(-1);
		}
		cfreelist.c_next = bp->c_next;
		bp->c_next = NULL;
		bp->c_first = 0; bp->c_last = 0;
		if (obp == NULL)
			p->c_cf = bp;
		else
			obp->c_next = bp;
		p->c_cl = bp;
	}
	bp->c_data[bp->c_last++] = c;
	p->c_cc++;
#ifdef mc68000
	asm("	movw	d7,sr");
#else
	splx(s);
#endif
	return(0);
}

struct cblock *
getcf()
{
	register struct cblock *bp;
	register struct chead *cf;

#ifdef mc68000
	/*
	 * Note use of d7
	 */
#ifndef lint
	register int s = 0;
#endif

	asm("	movw	sr,d7");
	asm("	movw	#0x2600,sr");
#else
	register int s;
	s = spl6();
#endif

	cf = &cfreelist;
	if ((bp = cf->c_next) != NULL) {
		cf->c_next = bp->c_next;
		bp->c_next = NULL;
		bp->c_first = 0;
		bp->c_last = cf->c_size;
	}
#ifdef mc68000
	asm("	movw	d7,sr");
#else
	splx(s);
#endif
	return(bp);
}

putcf(bp)
register struct cblock *bp;
{
	register struct chead *cf;

#ifdef mc68000
	/*
	 * Note use of d7
	 */
#ifndef lint
	register int s = 0;
#endif

	asm("	movw	sr,d7");
	asm("	movw	#0x2600,sr");
#else
	register int s;
	s = spl6();
#endif

	cf = &cfreelist;
	bp->c_next = cf->c_next;
	cf->c_next = bp;
	if (cf->c_flag) {
		cf->c_flag = 0;
		wakeup((caddr_t)cf);
	}
#ifdef mc68000
	asm("	movw	d7,sr");
#else
	splx(s);
#endif
}

struct cblock *
getcb(p)
register struct clist *p;
{
	register struct cblock *bp;

#ifdef mc68000
	/*
	 * Note use of d7
	 */
#ifndef lint
	register int s = 0;
#endif

	asm("	movw	sr,d7");
	asm("	movw	#0x2600,sr");
#else
	register int s;
	s = spl6();
#endif

	if ((bp = p->c_cf) != NULL) {
		p->c_cc -= bp->c_last - bp->c_first;
		if ((p->c_cf = bp->c_next) == NULL)
			p->c_cl = NULL;
	}
#ifdef mc68000
	asm("	movw	d7,sr");
#else
	splx(s);
#endif
	return(bp);
}

putcb(bp, p)
register struct cblock *bp;
register struct clist *p;
{
#ifdef mc68000
	/*
	 * Note use of d7
	 */
#ifndef lint
	register int s = 0;
#endif

	asm("	movw	sr,d7");
	asm("	movw	#0x2600,sr");
#else
	register int s;
	s = spl6();
#endif

	if (p->c_cl == NULL)
		p->c_cf = bp;
	else
		p->c_cl->c_next = bp;
	p->c_cl = bp;
	bp->c_next = NULL;
	p->c_cc += bp->c_last - bp->c_first;
#ifdef mc68000
	asm("	movw	d7,sr");
#else
	splx(s);
#endif
}

#ifdef notdef
getcbp(p, cp, n)
struct clist *p;
register char *cp;
register n;
{
	register struct cblock *bp;
	register char *op;
	register on;
	register char *acp = cp;

	while (n) {
		if ((bp = p->c_cf) == NULL)
			break;
		op = &bp->c_data[bp->c_first];
		on = bp->c_last - bp->c_first;
		if (n >= on) {
			bcopy(op, cp, on);
			cp += on;
			n -= on;
			if ((p->c_cf = bp->c_next) == NULL)
				p->c_cl = NULL;
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
		} else {
			bcopy(op, cp, n);
			bp->c_first += n;
			cp += n;
			n = 0;
			break;
		}
	}
	n = cp - acp;
	p->c_cc -= n;
	return(n);
}
#endif

#ifdef notdef
putcbp(p, cp, n)
struct clist *p;
register char *cp;
register n;
{
	register struct cblock *bp, *obp;
	register char *op;
	register on;
	register char *acp = cp;

	while (n) {
		if ((bp = p->c_cl) == NULL || bp->c_last == cfreelist.c_size) {
			obp = bp;
			if ((bp = cfreelist.c_next) == NULL)
				break;
			cfreelist.c_next = bp->c_next;
			bp->c_next = NULL;
			bp->c_first = 0; bp->c_last = 0;
			if (obp == NULL)
				p->c_cf = bp;
			else
				obp->c_next = bp;
			p->c_cl = bp;
		}
		op = &bp->c_data[bp->c_last];
		on = cfreelist.c_size - bp->c_last;
		if (n >= on) {
			bcopy(cp, op, on);
			cp += on;
			bp->c_last += on;
			n -= on;
		} else {
			bcopy(cp, op, n);
			cp += n;
			bp->c_last += n;
			n = 0;
			break;
		}
	}
	n = cp - acp;
	p->c_cc += n;
	return(n);
}
#endif
