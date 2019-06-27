/* @(#)bio.c	1.7 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/conf.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/scat.h"

/*
 * swap IO headers.
 */
struct	buf	swbuf[NSWB];

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no one else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
#define	notavail(bp)	\
{\
	register s;\
\
	s = spl6();\
	bp->av_back->av_forw = bp->av_forw;\
	bp->av_forw->av_back = bp->av_back;\
	bp->b_flags |= B_BUSY;\
	bfreelist.b_bcount--;\
	splx(s);\
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
#define	geterror(bp)	\
{\
	if (bp->b_flags&B_ERROR)\
		if ((u.u_error = bp->b_error)==0)\
			u.u_error = EIO;\
}

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;

	sysinfo.lread++;
	bp = getblk(dev, blkno);
	if (bp->b_flags&B_DONE)
		return(bp);
	bp->b_flags |= B_READ;
	bp->b_bcount = FsBSIZE(dev);
	(*bdevsw[bmajor(dev)].d_strategy)(bp);
	u.u_ior++;
	sysinfo.bread++;
	iowait(bp);
	return(bp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
struct buf *
breada(dev, blkno, rablkno)
dev_t dev;
daddr_t blkno, rablkno;
{
	register struct buf *bp, *rabp;

	bp = NULL;
	if (!incore(dev, blkno)) {
		sysinfo.lread++;
		bp = getblk(dev, blkno);
		if ((bp->b_flags&B_DONE) == 0) {
			bp->b_flags |= B_READ;
			bp->b_bcount = FsBSIZE(dev);
			(*bdevsw[bmajor(dev)].d_strategy)(bp);
			u.u_ior++;
			sysinfo.bread++;
		}
	}
	if (rablkno && bfreelist.b_bcount>1 && !incore(dev, rablkno)) {
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags |= B_READ|B_ASYNC;
			rabp->b_bcount = FsBSIZE(dev);
			(*bdevsw[bmajor(dev)].d_strategy)(rabp);
			u.u_ior++;
			sysinfo.bread++;
		}
	}
	if (bp == NULL)
		return(bread(dev, blkno));
	iowait(bp);
	return(bp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
register struct buf *bp;
{
	register flag;

	sysinfo.lwrite++;
	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	(*bdevsw[bmajor(bp->b_dev)].d_strategy)(bp);
	u.u_iow++;
	sysinfo.bwrite++;
	if ((flag&B_ASYNC) == 0) {
		iowait(bp);
		brelse(bp);
	} else if (flag & B_DELWRI)
		bp->b_flags |= B_AGE;
	else
		geterror(bp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
register struct buf *bp;
{

	sysinfo.lwrite++;
	bp->b_flags |= B_DELWRI | B_DONE;
	bp->b_resid = 0;
	brelse(bp);
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
register struct buf *bp;
{

	if (bfreelist.b_bcount>4)
		bp->b_flags |= B_ASYNC;
	bwrite(bp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
register struct buf *bp;
{
	register struct buf **backp;
	register s;

	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (bfreelist.b_flags&B_WANTED) {
		bfreelist.b_flags &= ~B_WANTED;
		wakeup((caddr_t)&bfreelist);
	}
	if (bp->b_flags&B_ERROR) {
		bp->b_flags |= B_STALE|B_AGE;
		bp->b_flags &= ~(B_ERROR|B_DELWRI);
		bp->b_error = 0;
	}

/* Put buffer on freelist, at the beginning if B_AGE, otherwise at the end. */

	s = spl6();
	if (bp->b_flags & B_AGE) {
		backp = &bfreelist.av_forw;
		(*backp)->av_back = bp;
		bp->av_forw = *backp;
		*backp = bp;
		bp->av_back = &bfreelist;
	} else {
		backp = &bfreelist.av_back;
		(*backp)->av_forw = bp;
		bp->av_back = *backp;
		*backp = bp;
		bp->av_forw = &bfreelist;
	}
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	bfreelist.b_bcount++;
	splx(s);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(dev, blkno)
register dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;

	blkno = FsLTOP(dev, blkno);
	dp = bhash(dev, blkno);
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno==blkno && bp->b_dev==dev && (bp->b_flags&B_STALE)==0)
			return(1);
	return(0);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 */
struct buf *
getblk(dev, blkno)
register dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;

	blkno = FsLTOP(dev, blkno);
    loop:
	SPL0();
	dp = bhash(dev, blkno);
	if (dp == NULL)
		panic("devtab");
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_blkno!=blkno || bp->b_dev!=dev || bp->b_flags&B_STALE)
			continue;
		SPL6();
		if (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			syswait.iowait++;
			(void) sleep((caddr_t)bp, PRIBIO+2);
			syswait.iowait--;
			goto loop;
		}
		SPL0();
		notavail(bp);
		return(bp);
	}
	SPL6();
	if (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags |= B_WANTED;
		(void) sleep((caddr_t)&bfreelist, PRIBIO+1);
		goto loop;
	}
	SPL0();
	bp = bfreelist.av_forw;
#ifdef OLD
	notavail(bp);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
#else
	if (bp->b_flags & B_DELWRI) {
		bflush(NODEV);
		goto loop;
	}
	notavail(bp);
#endif
	bp->b_flags = B_BUSY;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = dev;
	bp->b_blkno = blkno;
	bp->b_bcount = FsBSIZE(dev);
	return(bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
struct buf *
geteblk()
{
	register struct buf *bp;
	register struct buf *dp;

loop:
	SPL6();
	while (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags |= B_WANTED;
		(void) sleep((caddr_t)&bfreelist, PRIBIO+1);
	}
	SPL0();
	dp = &bfreelist;
	bp = bfreelist.av_forw;
	notavail(bp);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bp->b_flags = B_BUSY|B_AGE;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = (dev_t)NODEV;
	bp->b_bcount = SBUFSIZE;
	return(bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
register struct buf *bp;
{

	syswait.iowait++;
	SPL6();
	while ((bp->b_flags&B_DONE)==0)
		(void) sleep((caddr_t)bp, PRIBIO);
	SPL0();
	syswait.iowait--;
	geterror(bp);
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(bp)
register struct buf *bp;
{

	bp->b_flags |= B_DONE;
	if (bp->b_flags&B_ASYNC)
		brelse(bp);
	else {
		bp->b_flags &= ~B_WANTED;
		wakeup((caddr_t)bp);
	}
}

/*
 * Zero the core associated with a buffer.
 */
clrbuf(bp)
struct buf *bp;
{
	clear((caddr_t)bp->b_un.b_words, (int)bp->b_bcount);
	bp->b_resid = 0;
}

/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
daddr_t blkno;
register coreaddr, count;
{
	static struct buf *sbp;
	register struct buf *bp;
	register int c;

#ifdef SWAPTRACE
	printf("SWAP %s %d disk=0x%x core=0x%x\n",
		(rdflg==B_READ)?"IN":"OUT", count, blkno, coreaddr);
#endif SWAPTRACE
	syswait.swap++;
	if (sbp==NULL)
		sbp = &swbuf[0];
	bp = sbp++;
	if (sbp > &swbuf[NSWB-1])
		sbp = &swbuf[0];
	SPL6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		(void) sleep((caddr_t)bp, PSWP+1);
	}
	bp->b_flags = B_BUSY | B_PHYS | rdflg;
	SPL0();
	bp->b_dev = swapdev;
#ifdef NONSCATLOAD
	bp->b_un.b_addr = (caddr_t)ctob(coreaddr);
	while (count > 0) {
		if (count <= btoc(MAXCOUNT))
			c = count;
		else
			c = btoc(MAXCOUNT);
		bp->b_bcount = ctob(c);
		bp->b_blkno = swplo+blkno;
		(*bdevsw[(short)bmajor(swapdev)].d_strategy)(bp);
		u.u_iosw++;
		if (rdflg) {
			sysinfo.swapin++;
			sysinfo.bswapin += ctod(c);
		} else {
			sysinfo.swapout++;
			sysinfo.bswapout += ctod(c);
		}
		SPL6();
		while((bp->b_flags&B_DONE)==0)
			(void) sleep((caddr_t)bp, PSWP);
		SPL0();
		bp->b_un.b_addr += ctob(c);
		bp->b_flags &= ~B_DONE;
		if (bp->b_flags & B_ERROR)
			panic("IO err in swap");
		count -= c;
		blkno += ctod(c);
	}
#else
#define sindex coreaddr
	while (count > 0) {
		if (sindex == SCATEND) {
			printf("swap error:swapping beyond process\n");
			break;
		}
		c = memcontig(sindex, count);
		bp->b_un.b_addr = (caddr_t)ctob(ixtoc(sindex));
		bp->b_bcount = ctob(c);
		bp->b_blkno = swplo+blkno;
#ifdef SWAPTRACE
		printf("    SWAP %s %d disk=0x%x sindex=0x%x\n",
			(rdflg==B_READ)?"IN":"OUT", c, blkno, sindex);
#endif SWAPTRACE
		(*bdevsw[(short)bmajor(swapdev)].d_strategy)(bp);
		u.u_iosw++;
		if (rdflg) {
			sysinfo.swapin++;
			sysinfo.bswapin += ctod(c);
		} else {
			sysinfo.swapout++;
			sysinfo.bswapout += ctod(c);
		}
		SPL6();
		while((bp->b_flags&B_DONE)==0)
			(void) sleep((caddr_t)bp, PSWP);
		SPL0();
		bp->b_flags &= ~B_DONE;
		if (bp->b_flags & B_ERROR)
			panic("IO err in swap");
		count -= c;
		blkno += ctod(c);
		while (c-- > 0 && sindex != SCATEND)
			sindex = scatmap[sindex].sc_index;
	}
#endif
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
	syswait.swap--;
#ifndef NONSCATLOAD
	return(sindex);
#endif
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
bflush(dev)
dev_t dev;
{
	register struct buf *bp;

	SPL6();
	for (bp = bfreelist.av_forw; bp != &bfreelist;) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			bp->b_flags |= B_ASYNC;
			notavail(bp);
			bwrite(bp);
			SPL6();
			bp = bfreelist.av_forw;
		} else {
			if (bp->av_forw) bp = bp->av_forw;
			else panic("bflush: bad free list\n");
		}
	}
	SPL0();
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer header, sometimes of a special type owned by the
 *	device, and sometimes from the physio pool of headers.
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, bp, dev, rw)
register struct buf *bp;
int (*strat)();
{
	register struct user *up;
	register struct proc *p;
	register unsigned base;
	register unsigned limit;
	register unsigned dsstart, dsend;
	int	hpf;

	up = &u;
	p = up->u_procp;
	base = (unsigned)up->u_base;

	dsstart = v.v_ustart + ctob(stoc(ctos(up->u_tsize)));
	dsend = dsstart + ctob(up->u_dsize);
	limit = base + up->u_count - 1;

	/*
	 * Check that transfer is either entirely in the
	 * virtual data space or in the virtual stack space
	 */

	if (limit < base)	/* wraparound, base < 0, count <= 0 */
		goto bad;
	if (base >= dsstart && limit < dsend)
		goto cont;
	if (base >= v.v_uend - ctob(up->u_ssize) && limit < v.v_uend)
		goto cont;
	if (rw != B_READ && base >= v.v_ustart &&
	    limit < v.v_ustart + ctob(up->u_tsize))
		goto cont;
	if (chkphys((int)base, limit))
		goto cont;
bad:
	up->u_error = EFAULT;
	return;
cont:
	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;
	syswait.physio++;
#ifndef NONSCATLOAD
	if ((p->p_flag&SCONTIG)==0) {
		p->p_flag |= SSWAPIT;
		if (runout) {
			runout = 0;
			wakeup((caddr_t)&runout);
		}
		if (runin) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		SPL6();
		while ((p->p_flag&SCONTIG)==0)
			(void) sleep((caddr_t)scatmap, PRIBIO);
	}
#endif
	hpf = (bp == NULL);
	SPL6();
	if (hpf) {
		while ((bp = pfreelist.av_forw) == NULL) {
			pfreelist.b_flags |= B_WANTED;
			(void) asleep((caddr_t)&pfreelist, PRIBIO+1);
		}
		pfreelist.av_forw = bp->av_forw;
	} else while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		(void) asleep((caddr_t)bp, PRIBIO+1);
	}
	bp->b_error = 0;
	bp->b_un.b_addr = vtop((caddr_t)base);
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	bp->b_blkno = up->u_offset >> BSHIFT;
	bp->b_bcount = up->u_count;
	p->p_flag |= SLOCK;
	(*strat)(bp);
	SPL6();
	while ((bp->b_flags&B_DONE) == 0)
		(void) sleep((caddr_t)bp, PRIBIO);
	p->p_flag &= ~SLOCK;
	if (hpf) {
		bp->av_forw = pfreelist.av_forw;
		pfreelist.av_forw = bp;
		if (pfreelist.b_flags&B_WANTED) {
			pfreelist.b_flags &= ~B_WANTED;
			wakeup((caddr_t)&pfreelist);
		}
	} else if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (runin) {
		runin = 0;
		wakeup(&runin);
	}
	SPL0();
	bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
	up->u_count = bp->b_resid;
	geterror(bp);
	syswait.physio--;
}

physck(nblocks, rw)
daddr_t nblocks;
{
	register struct user *up;
	register unsigned over;
	off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	up = &u;
	limit = nblocks << BSHIFT;
	if (up->u_offset >= limit) {
		if (up->u_offset > limit || rw == B_WRITE)
			up->u_error = ENXIO;
		return(0);
	}
	upper = up->u_offset + up->u_count;
	if (upper > limit) {
		over = upper - limit;
		up->u_count -= over;
		uap = (struct a *)up->u_ap;
		uap->count -= over;
	}
	return(1);
}

/*
 * Invalidate blocks for a dev after last close.
 */
binval(dev)
register dev;
{
	register struct buf *dp;
	register struct buf *bp;
	register i;

	if (dev == swapdev)
		return;
	for (i=0; i<v.v_hbuf; i++) {
		dp = (struct buf *)&hbuf[(short)i];
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
			if (bp->b_dev == dev)
				bp->b_flags |= B_STALE|B_AGE;
	}
}

/*
 * Get the major device number given a strategy routine address
 */
getmajor(strat)
register int (*strat)();
{
	register struct bdevsw *bdp;
	register i;

	bdp = &bdevsw[0]; 
	for (i = 0; i < bdevcnt; i++, bdp++)
		if (bdp->d_strategy == strat)
			return(i);
	panic("findmajor");
	/* NOTREACHED */
}
