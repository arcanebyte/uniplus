/* @(#)text.c	1.3 */
#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/context.h"
#include "sys/text.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/sysinfo.h"
#include "sys/scat.h"

typedef int	mem_t;

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * On a partial swap ff is the negative number of blocks to be swapped.
 * Os is the old size  of the process,
 * and is supplied during core expansion swaps.
 */
xswap(p, ff, os)
register struct proc *p;
{
	register a, i, s, tos;
	int addr, sz;

	if (os == 0)
		os = p->p_size;
	p->p_flag |= (SLOCK|SNOMMU);
	xccdec(p->p_textp);
	cxrelse(p->p_context);
	a = malloc(swapmap, ctod(p->p_size));
	if (a == NULL) {
		/*
		 * s = decreasing click size of total disk space needed
		 * tos = decreasing click size of process being swapped
		 */
		tos = os;
		s = p->p_size;
#ifdef NONSCATLOAD
		addr = p->p_addr;
#else
		addr = p->p_scat;
#endif
		for (i=0; i < NSCATSWAP; i++) {
			if ((a = dtoc(mallocl(swapmap))) == NULL)
				break;
			if (a > s)
				a = s;
			p->p_xaddr[i] = malloc(swapmap, (mem_t)ctod(a));
			p->p_xsize[i] = a;
			sz = MIN(a, tos);
			if (sz) {
#ifdef NONSCATLOAD
				swap(p->p_xaddr[i], addr, (mem_t)sz, B_WRITE);
#else
				addr = swap(p->p_xaddr[i], addr, (mem_t)sz,
					B_WRITE);
#endif
			}
			s -= a;
			if (s == 0)
				break;
			tos -= a;
			if (tos < 0)
				tos = 0;
#ifdef NONSCATLOAD
			addr += a;
#endif
		}
		if (s != 0)
			panic("out of swap space");
		a = p->p_xaddr[0];	/* for /bin/ps */
	} else {
#ifdef NONSCATLOAD
		swap((daddr_t)a, (int)p->p_addr, (mem_t)os, B_WRITE);
#else
		(void) swap((daddr_t)a, (int)p->p_scat, (mem_t)os, B_WRITE);
#endif
	}
 	p->p_flag &= ~SNOMMU;
	if (ff) {
#ifdef NONSCATLOAD
		mfree(coremap, (mem_t)os, (mem_t)p->p_addr);
#else
		memfree(p->p_scat);
		p->p_scat = 0;
#endif
	}
	cxrelse(p->p_context);
	p->p_dkaddr = a;
	p->p_flag &= ~(SLOAD|SLOCK);
#ifndef NONSCATLOAD
	if (p->p_flag & SSWAPIT) {
		p->p_flag &= ~SSWAPIT;
		p->p_flag |= SCONTIG;
		wakeup((caddr_t)scatmap);
	}
#endif
	p->p_time = 0;
	if (runout) {
		runout = 0;
		wakeup((caddr_t)&runout);
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct inode *ip;
	register struct proc *p = u.u_procp;

	if ((xp = p->p_textp) == NULL)
		return;
	xlock(xp);
	xp->x_flag &= ~XLOCK;
	p->p_textp = NULL;
	u.u_ptsize = 0;
	ip = xp->x_iptr;
	if (--xp->x_count==0 && ((ip->i_mode&ISVTX)==0 || xp->x_flag&XERROR)) {
		xmsave(xp);
		xp->x_iptr = NULL;
		if (xp->x_daddr)
			mfree(swapmap, ctod(xp->x_size), (int)xp->x_daddr);
		cxtxfree(xp);
		ip->i_flag &= ~ITEXT;
		if (ip->i_flag&ILOCK)
			ip->i_count--;
		else
			iput(ip);
	} else
		xccdec(xp);
	cxrelse(u.u_procp->p_context);
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */
xalloc(ip)
register struct inode *ip;
{
	register struct text *xp;
	register ts;
	register struct text *xp1;
	register struct user *up;

	up = &u;
	if (up->u_exdata.ux_tsize == 0)
		return;
	xp1 = NULL;
loop:
	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++) {
		if (xp->x_iptr == NULL) {
			if (xp1 == NULL)
				xp1 = xp;
			continue;
		}
		if (xp->x_iptr == ip) {
			xlock(xp);
			xp->x_count++;
			up->u_procp->p_textp = xp;
			if (xp->x_ccount == 0)
				(void) xexpand(xp);
			else
				xp->x_ccount++;
			xunlock(xp);
			return;
		}
	}
	if ((xp=xp1) == NULL) {
		printf("out of text\n");
		syserr.textovf++;
		if (xumount(NODEV))
			goto loop;
		psignal(up->u_procp, SIGKILL);
		return;
	}
	xp->x_flag = XLOAD|XLOCK;
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = ip;
	ip->i_flag |= ITEXT;
	ip->i_count++;
	ts = btoc(up->u_exdata.ux_tsize);
	xp->x_size = ts;
	/* if ((xp->x_daddr = malloc(swapmap, ctod(ts))) == NULL) */
		/* panic("out of swap space"); */
	xp->x_daddr = 0;	/* defer swap alloc til later */
	up->u_procp->p_textp = xp;
	if (xexpand(xp)) {
		(void) estabur((unsigned)ts, (unsigned)0, (unsigned)0, 0, RW);
		xp->x_flag = XWRIT;
		return;
	}
	(void) estabur((unsigned)ts, (unsigned)0, (unsigned)0, 0, RW);
	up->u_count = up->u_exdata.ux_tsize;
	up->u_offset = sizeof(up->u_exdata);
	/* up->u_offset = up->u_exdata.ux_tstart; */
	up->u_base = (caddr_t)v.v_ustart;
	/* up->u_base = 0; */
	up->u_segflg = 2;
	up->u_procp->p_flag |= SLOCK;
	readi(ip);
	up->u_procp->p_flag &= ~SLOCK;
	up->u_segflg = 0;
	if (up->u_error || up->u_count!=0)
		xp->x_flag = XERROR;
	else
		xp->x_flag = XWRIT;
}

/*
 * Assure core for text segment
 * Text must be locked to keep someone else from
 * freeing it in the meantime.
 * x_ccount must be 0.
 */
xexpand(xp)
register struct text *xp;
{
	if (xmlink(xp)) {
		xp->x_ccount++;
		xunlock(xp);
		return(1);
	}
#ifdef NONSCATLOAD
	if ((xp->x_caddr = malloc(coremap, xp->x_size)) != NULL) {
		if ((xp->x_flag&XLOAD)==0)
			swap(xp->x_daddr, (int)xp->x_caddr, xp->x_size, B_READ);
		xp->x_ccount++;
		xunlock(xp);
		return(0);
	}
#else
	if ((xp->x_scat = memalloc(xp->x_size)) != NULL) {
		if ((xp->x_flag&XLOAD)==0)
			(void) swap(xp->x_daddr, (int)xp->x_scat,
				xp->x_size, B_READ);
		xp->x_ccount++;
		xunlock(xp);
		return(0);
	}
#endif
	if (save(u.u_ssav)) {
		cxtxfree(xp);
		sureg();
		return(0);
	}
	xswap(u.u_procp, 1, 0);
	xunlock(xp);
	u.u_procp->p_flag |= SSWAP;
	qswtch();
#ifdef lint
	return(0);
#endif
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(xp)
register struct text *xp;
{

	while(xp->x_flag&XLOCK) {
		xp->x_flag |= XWANT;
		(void) sleep((caddr_t)xp, PSWP);
	}
	xp->x_flag |= XLOCK;
}

xunlock(xp)
register struct text *xp;
{

	if (xp->x_flag&XWANT)
		wakeup((caddr_t)xp);
	xp->x_flag &= ~(XLOCK|XWANT);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp)
register struct text *xp;
{
	int prevlock;

	if (xp==NULL || xp->x_ccount==0)
		return;
	xlock(xp);
	if (!(prevlock = (u.u_procp->p_flag & SLOCK)))
		u.u_procp->p_flag |= SLOCK;
	if (--xp->x_ccount==0) {
		if (xp->x_flag&XWRIT) {
			xp->x_flag &= ~XWRIT;
			if (xp->x_daddr == 0)
				xp->x_daddr = swapalloc(ctod(xp->x_size), 1);
#ifdef NONSCATLOAD
			swap(xp->x_daddr,(int)xp->x_caddr, xp->x_size, B_WRITE);
#else
			(void) swap(xp->x_daddr,
				(int)xp->x_scat, xp->x_size, B_WRITE);
#endif
		}
		xmsave(xp);
		cxtxfree(xp);
	}
	if (!prevlock)
		u.u_procp->p_flag &= ~SLOCK;
	xunlock(xp);
}

/*
 * free the swap image of all unused saved-text text segments
 * which are from device dev (used by umount system call).
 */
xumount(dev)
register dev_t dev;
{
	register struct inode *ip;
	register struct text *xp;
	register count = 0;

	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++) {
		if ((ip = xp->x_iptr) == NULL)
			continue;
		if (dev != NODEV && dev != ip->i_dev)
			continue;
		if (xuntext(xp))
			count++;
	}
	return(count);
}

/*
 * remove a shared text segment from the text table, if possible.
 */
xrele(ip)
register struct inode *ip;
{
	register struct text *xp;

	if ((ip->i_flag&ITEXT) == 0)
		return;
	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++)
		if (ip==xp->x_iptr)
			(void) xuntext(xp);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
register struct text *xp;
{
	register struct inode *ip;

	xlock(xp);
	if (xp->x_count) {
		xunlock(xp);
		return(0);
	}
	ip = xp->x_iptr;
	xmfree(ip);
	xp->x_flag &= ~XLOCK;
	xp->x_iptr = NULL;
	cxtxfree(xp);
	if (xp->x_daddr)
		mfree(swapmap, ctod(xp->x_size), (int)xp->x_daddr);
	ip->i_flag &= ~ITEXT;
	if (ip->i_flag&ILOCK)
		ip->i_count--;
	else
		iput(ip);
	return(1);
}

/*
 * allocate swap blocks, freeing and sleeping as necessary
 */
swapalloc(size, sflg)
{
	register addr;

	for (;;) {
		if (addr = malloc(swapmap, size))
			return(addr);
		if (swapclu()) {
			printf("\nWARNING: swap space running out\n");
			printf("  needed %d blocks\n", size);
			continue;
		}
		printf("\nDANGER: out of swap space\n");
		printf("  needed %d blocks\n", size);
		if (sflg) {
			mapwant(swapmap)++;
			(void) sleep((caddr_t)swapmap, PSWP);
		} else
			return(0);
	}
}

/*
 * clean up swap used by text
 */
swapclu()
{
	register struct text *xp;
	register ans = 0;

	for (xp = text; xp < (struct text *)v.ve_text; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		if (xp->x_flag&XLOCK)
			continue;
		if (xp->x_daddr == 0)
			continue;
		if (xp->x_count) {
			if (xp->x_ccount) {
				mfree(swapmap, ctod(xp->x_size),
					(int)xp->x_daddr);
				xp->x_flag |= XWRIT;
				xp->x_daddr = 0;
				ans++;
			}
		} else {
			(void) xuntext(xp);
			ans++;
		}
	}
	return(ans);
}

/*
 * free the saved text area associated with an inode
 */
xmfree(ip)
register struct inode *ip;
{
	register struct svtext *svx;

	for (svx = &svtext[0]; svx < (struct svtext *)v.ve_svtext; svx++) {
		if (svx->x_svflag&XSVBUSY && ip->i_number==svx->x_svnumber &&
		    ip->i_dev==svx->x_svdev) {
			svx->x_svflag &= ~XSVBUSY;
#ifdef NONSCATLOAD
			mfree(coremap, svx->x_svsize, (mem_t)svx->x_svcaddr);
#else
			memfree((mem_t)svx->x_svscat);
#endif
			break;
		}
	}
}

/*
 * link up to a text region already in memory
 */
xmlink(xp)
register struct text *xp;
{
	register struct svtext *svx;
	register struct inode *ip;

	ip = xp->x_iptr;
	for (svx = &svtext[0]; svx < (struct svtext *)v.ve_svtext; svx++) {
		if (svx->x_svflag&XSVBUSY && ip->i_number==svx->x_svnumber &&
		    ip->i_dev==svx->x_svdev) {
			svx->x_svflag &= ~XSVBUSY;
#ifdef NONSCATLOAD
			xp->x_caddr = svx->x_svcaddr;
#else
			xp->x_scat = svx->x_svscat;
#endif
#ifdef TEXTTRACE
			printf("linking to text caddr 0x%x\n", svx->x_svcaddr);
#endif
			return(1);
		}
	}
	return(0);
}

/*
 * Release a shared text segment in the text area space.
 */
xmrelse()
{
	register struct svtext *svx, *tsvx;
	register n;

	n = ((unsigned) -1) >> 1;
	tsvx = NULL;
	for (svx = &svtext[0]; svx < (struct svtext *)v.ve_svtext; svx++) {
		if (svx->x_svflag&XSVBUSY && svx->x_svsize<n) {
			n = svx->x_svsize;
			tsvx = svx;
			continue;
		}
	}
	if (tsvx == NULL)
		return(0);
#ifdef TEXTTRACE
	printf("freeing %d segments at text caddr 0x%x\n",
		tsvx->x_svsize, tsvx->x_svcaddr);
#endif
#ifdef NONSCATLOAD
	mfree(coremap, tsvx->x_svsize, (mem_t)tsvx->x_svcaddr);
#else
	memfree((mem_t)tsvx->x_svscat);
#endif
	tsvx->x_svflag &= ~XSVBUSY;
	return(1);
}

/*
 * Save the memory of a text region of a shared process
 */
xmsave(xp)
register struct text *xp;
{
	register struct svtext *svx, *tsvx;
	register struct inode *ip;

	tsvx = NULL;
	ip = xp->x_iptr;
	for (svx = &svtext[0]; svx < (struct svtext *)v.ve_svtext; svx++) {
		if ((svx->x_svflag&XSVBUSY) == 0) {
			if (tsvx == NULL)
				tsvx = svx;
			continue;
		}
		if (ip->i_number==svx->x_svnumber && ip->i_dev==svx->x_svdev) {
			printf("xmrelse:memory saved more than once\n");
			tsvx = NULL;
			break;
		}
	}
	/*
	 * No space left in table
	 */
	if (xp->x_flag&XERROR || tsvx == NULL) {
#ifdef NONSCATLOAD
		mfree(coremap, xp->x_size, (mem_t)xp->x_caddr);
#else
		memfree((mem_t)xp->x_scat);
#endif
	} else {
		tsvx->x_svflag |= XSVBUSY;
		tsvx->x_svsize = xp->x_size;
#ifdef NONSCATLOAD
		tsvx->x_svcaddr = xp->x_caddr;
#else
		tsvx->x_svscat = xp->x_scat;
#endif
		tsvx->x_svdev = ip->i_dev;
		tsvx->x_svnumber = ip->i_number;
#ifdef TEXTTRACE
		printf("saving %d segments at text caddr 0x%x\n",
			tsvx->x_svsize, tsvx->x_svcaddr);
#endif
	}
}
