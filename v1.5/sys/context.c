#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/callo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/psl.h"
#include "sys/var.h"
#include "sys/context.h"
#include "sys/map.h"

/* #define HOWFAR */
extern struct shminfo	shminfo;	/* shared memory info structure */

struct context cxhdr;		/* head of context structure */

/*
 * Allocate a new context freeing one if necessary
 */
struct context *
cxalloc()
{
	register struct context *cx;

	/*
	 * search for unused context
	 */
	for (cx = cxhdr.cx_forw; cx != &cxhdr; cx = cx->cx_forw)
		if (cx->cx_proc == 0)
			return(cxunlink(cx));
	/*
	 * return the context on top of the queue
	 */
	cx = cxhdr.cx_forw;
	cxrfree(cx);
	return(cxunlink(cx));
}

/*
 * Find first used context and free it
 */
cxfree()
{
	register struct context *cx;

	for (cx = cxhdr.cx_forw; cx != &cxhdr; cx = cx->cx_forw) {
		if (cx->cx_proc) {
			cxrelse(cx);
			return(0);
		}
	}
	return(-1);
}

/*
 * Initialize the context structure linked list
 */
cxinit()
{
	register struct context *cx;
	register i;

	i = USERCX;
	cx = &cxhdr;
	cx->cx_forw = cx->cx_back = cx;
	for (cx = &context[0]; cx < &context[NUMUCONTX]; cx++) {
		cx->cx_num = cxntocx(i++);
		cxtail(cx);
	}
}

/*
 * Release the context associated with
 * a given context structure and
 * move it to the head of the queue
 */
cxrelse(cx)
register struct context *cx;
{
	register struct context **backp;

	if (cx == 0)
		return;
#ifdef HOWFAR
printf("Releasing %d segs for cx %d\n", cx->cx_dsize, cx->cx_num);
#endif
	cxrfree(cx);
	cx->cx_back->cx_forw = cx->cx_forw;
	cx->cx_forw->cx_back = cx->cx_back;
	backp = (struct context **)&cxhdr.cx_forw;
	(*backp)->cx_back = cx;
	cx->cx_forw = *backp;
	*backp = cx;
	cx->cx_back = &cxhdr;
}

/*
 * Release the context associated with
 * a given context structure
 */
cxrfree(cx)
register struct context *cx;
{
	register struct proc *p;
	register struct cxphys *cxp;
	register struct cxshm *cxs;
	int i;

	if ((p = cx->cx_proc) == NULL)
		return;
	if (cx->cx_dsize > 0) {
#ifdef HOWFAR
printf("Freeing %d data segments at %d for pid %d\n",
	cx->cx_dsize, cx->cx_daddr, p->p_pid);
#endif
		if (cx->cx_daddr == 0)
			printf("cxrfree error. cx_daddr = 0\n");
		mfree(cxmap, (short)cx->cx_dsize, (short)cx->cx_daddr);
		cx->cx_dsize = 0;
		cx->cx_daddr = 0;
	}
	cxp = &cx->cx_phys[0];
	for (i=0; i<v.v_phys; i++) {
		if (cxp->cx_phsize) {
			mfree(cxmap, (short)cxp->cx_phsize,
				(short)cxp->cx_phaddr);
			cxp->cx_phsize = 0;
			cxp->cx_phaddr = 0;
		}
		cxp++;
	}
	cxs = &cx->cx_shm[0];
	for (i=0; i < shminfo.shmseg; i++) {
		if (cxs->cx_shmsize) {
			mfree(cxmap, (short)cxs->cx_shmsize,
				(short)cxs->cx_shmaddr);
			cxs->cx_shmsize = 0;
			cxs->cx_shmaddr = 0;
		}
		cxs++;
	}
	cx->cx_proc = 0;
	p->p_context = 0;
}

/*
 * Put a context buffer onto the tail of the context queue
 */
cxtail(cx)
register struct context *cx;
{
	register struct context **backp;

	backp = (struct context **)&cxhdr.cx_back;
	(*backp)->cx_forw = cx;
	cx->cx_back = *backp;
	*backp = cx;
	cx->cx_forw = &cxhdr;
}

/*
 * Release context resources associated
 * with a text segment.
 */
cxtxfree(xp)
register struct text *xp;
{
	register struct proc *p;

	for (p = &proc[0]; p < (struct proc *)v.ve_proc; p++)
		if (p->p_textp == xp)
			cxrelse(p->p_context);
	if (xp->x_cxaddr == 0 || xp->x_size == 0)
		return;
	mfree(cxmap, (short)ctos(xp->x_size), (short)xp->x_cxaddr);
	xp->x_cxaddr = 0;
}

/*
 * unlink a context structure from the queue
 */
struct context *
cxunlink(cx)
register struct context *cx;
{
	cx->cx_back->cx_forw = cx->cx_forw;
	cx->cx_forw->cx_back = cx->cx_back;
	return(cx);
}

/*
 * Free all shared text segments
 */
txfree()
{
	register struct text *xp;
	register struct proc *p;
	int n;

	n = 0;
	for (p = &proc[0]; p < (struct proc *)v.ve_proc; p++) {
		if ((xp = p->p_textp) != NULL) {
			cxrelse(p->p_context);
			if (xp->x_cxaddr && xp->x_size) {
				mfree(cxmap, (short)ctos(xp->x_size),
					(short)xp->x_cxaddr);
				xp->x_cxaddr = 0;
				n++;
			}
		}
	}
	return(n);
}
