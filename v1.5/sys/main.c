/* #define HOWFAR */

/* @(#)main.c	1.7 */
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/filsys.h"
#include "sys/mount.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/tty.h"
#include "sys/var.h"
#include	<sys/file.h>

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * loop at low address in user mode -- /etc/init
 *	cannot be executed.
 */

int	maxmem;
int	minmem;
int	physmem;
int	cputype;
int	*nofault;
struct inode *rootdir;

main(cudot)
{
	register struct user *up;
	register struct proc *p;
	register struct init_tbl *initp;
	extern char oemmsg[], timestamp[];
	extern int cmask, cdlimit;

	printf("(C) Copyright 1983 - UniSoft Corporation\n    ");
	printf("%d", cputype ? cputype : 68000);
	printf(" Unix System V - August 1983\n\n");
	printf("Created %s\n\n", timestamp);

	printf("%s\n", oemmsg);

	/*
	 * set up system process
	 */

	up = &u;
	p = &proc[0];
#ifndef NONSCATLOAD
	p->p_scat = 0;
#endif
	p->p_addr = cudot;
 	p->p_size = v.v_usize;
	p->p_stat = SRUN;
	p->p_flag |= SLOAD|SSYS;
	p->p_nice = NZERO;
	up->u_procp = p;
	up->u_cmask = cmask;
	up->u_limit = cdlimit;
	up->u_stack[0] = STKMAGIC;

	startup(cudot);

	/*
	 * initialize system tables at priority 7
	 */
	for (initp = &init7_tbl[0]; initp->init_func; initp++) {
#ifdef HOWFAR
		printf("main calling %s\n", initp->init_msg);
#endif
		(*initp->init_func)();
	}

#ifdef HOWFAR
	printf("about to spl0\n");
#endif HOWFAR
	SPL0();
#ifdef HOWFAR
	printf("now at level 0\n");
#endif HOWFAR

	/*
	 * initialize system tables at priority 0
	 */
	for (initp = &init0_tbl[0]; initp->init_func; initp++) {
#ifdef HOWFAR
		printf("main calling %s\n", initp->init_msg);
#endif
		(*initp->init_func)();
	}

#ifdef HOWFAR
	printf("main calling iget\n");
#endif
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	up->u_cdir = iget(rootdev, ROOTINO);
	up->u_cdir->i_flag &= ~ILOCK;
	up->u_rdir = NULL;
	up->u_start = time;

	/*
	 * create initial processes
	 * start scheduling task
	 */

#ifdef HOWFAR
	printf("main calling newproc\n");
# endif
	if (newproc(0)) {
#ifdef HOWFAR
		printf("main calling expand\n");
# endif
		expand((int)(v.v_usize + btoc(szicode)));
#ifdef HOWFAR
		printf("main calling estabur\n");
# endif
		(void) estabur((unsigned)0, (unsigned)btoc(szicode),
			(unsigned)0, 0, RO);
#ifdef HOWFAR
		printf("main calling copyout\n");
# endif
		(void) copyout((caddr_t)icode,
			(caddr_t)(v.v_ustart+v.v_doffset), szicode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
#ifdef HOWFAR
		printf("main returning to icode\n");
# endif
		return;
	}
#ifdef HOWFAR
	printf("main calling sched\n");
# endif
	sched();
}

/*
 * iinit is called once (from main) very early in initialization.
 * It reads the root's super block and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super block.
 * Usually because of an IO error.
 */
iinit()
{
	register struct user *up;
	register struct buf *cp;
	register struct filsys *fp;
	struct inode iinode;

	up = &u;
	(*bdevsw[bmajor(rootdev)].d_open)(minor(rootdev), FREAD | FWRITE);
	(*bdevsw[bmajor(pipedev)].d_open)(minor(pipedev), FREAD | FWRITE);
	(*bdevsw[bmajor(swapdev)].d_open)(minor(swapdev), FREAD | FWRITE);
	cp = geteblk();
	fp = cp->b_un.b_filsys;
	iinode.i_mode = IFBLK;
	iinode.i_rdev = rootdev;
	up->u_offset = SUPERBOFF;
	up->u_count = sizeof(struct filsys);
	up->u_base = (caddr_t)fp;
	up->u_segflg = 1;
	readi(&iinode);
	if (up->u_error)
		panic("iinit");
	mount[0].m_bufp = cp;
	mount[0].m_flags = MINUSE;
	mount[0].m_dev = brdev(rootdev);
	if (fp->s_magic != FsMAGIC)
		fp->s_type = Fs1b;	/* assume old file system */
	if (fp->s_type == Fs2b)
		mount[0].m_dev |= Fs2BLK;
#if FsTYPE == 4
	if (fp->s_type == Fs4b)
		mount[0].m_dev |= Fs4BLK;
#endif
	rootdev = mount[0].m_dev;
	if (brdev(pipedev) == brdev(rootdev))
		pipedev = rootdev;
	fp->s_flock = 0;
	fp->s_ilock = 0;
	fp->s_ronly = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;

	clkset(fp->s_time);
}

/*
 * Initialize clist by freeing all character blocks.
 */
struct chead cfreelist;
cinit()
{
	register n;
	register struct cblock *cp;

	for(n = 0, cp = &cfree[0]; n < v.v_clist; n++, cp++) {
		cp->c_next = cfreelist.c_next;
		cfreelist.c_next = cp;
	}
	cfreelist.c_size = CLSIZE;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */
binit()
{
	register struct buf *bp;
	register struct buf *dp;
	register unsigned i;
	register long b;

	dp = &bfreelist;
	dp->b_forw = dp; dp->b_back = dp;
	dp->av_forw = dp; dp->av_back = dp;
	b = (((long)buffers + (sizeof(int) - 1)) & (-sizeof(int)));
	for (i=0, bp=buf; i<v.v_buf; i++,bp++) {
		bp->b_dev = NODEV;
		bp->b_un.b_addr = (caddr_t)b;
		b += SBUFSIZE;
		bp->b_back = dp;
		bp->b_forw = dp->b_forw;
		dp->b_forw->b_back = bp;
		dp->b_forw = bp;
		bp->b_flags = B_BUSY;
		bp->b_bcount = 0;
		brelse(bp);
	}
	pfreelist.av_forw = bp = pbuf;
	for (; bp < &pbuf[(short)(v.v_pbuf-1)]; bp++)
		bp->av_forw = bp+1;
	bp->av_forw = NULL;
	for (i=0; i < v.v_hbuf; i++)
		hbuf[(short)i].b_forw = hbuf[(short)i].b_back = (struct buf *)&hbuf[(short)i];
}
