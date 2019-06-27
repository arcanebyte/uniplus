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
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/psl.h"
#include "sys/var.h"
#include "sys/seg.h"
#include "sys/context.h"
#include "sys/map.h"
#include "sys/errno.h"
#include "sys/scat.h"

typedef int	mem_t;
short	segoff;		/* mmu segment offset */
extern struct shmid_ds	*shm_shmem[];	/* ptrs to attached segments */
extern struct shmpt_ds	shm_pte[];	/* segment attach points */
extern struct shminfo	shminfo;	/* shared memory info structure */

/* #define DUMPMM */
/* #define HOWFAR */
/* #define TRACEALL */

/*
 * Load the user hardware page map.
 */
sureg()
{
	register struct user *up;
	register struct phys *ph;
	register struct shmid_ds *sp;
	register short *addr;
	register a, i, j, page;
	struct text *tp;
	struct proc *p;

	up = &u;
	p = up->u_procp;
	tp = p->p_textp;
#ifdef HOWFAR
printf("sureg:p_addr=0x%x, tsize=%d dsize=%d ssize=%d\n",
	p->p_addr, up->u_ptsize, up->u_pdsize, up->u_pssize);
#endif
	SEG1_1 = 1;
	/* SEG2_0 = 1; */
	clearmmu();

	addr = (short *)vtoseg(v.v_ustart);
	if (tp != NULL) {
		page = tp->x_caddr + segoff;
		/* map a max of NPAGEPERSEG (256) 512-byte pages per segment */
		for (i = up->u_ptsize; i > 0; i -= a) {
			a = min(NPAGEPERSEG, (unsigned)i);
			setmmu((short *)((int)addr | ACCSEG), page);
			setmmu((short *)((int)addr | ACCLIM),
				((up->u_xrw==RO)?ASRO:ASRW) | ((256-a) & 0xFF));
			page += a;
			addr = (short *)((long)addr + (1 << SEGSHIFT));
		}
		addr=(short *)vtoseg(ctob(stoc(ctos(btoc(v.v_ustart)+up->u_ptsize))));
	} else
		if (up->u_ptsize != 0)
			addr = (short *)((long)addr +
				ctob(stoc(ctos(up->u_ptsize))));

		/* set up data segment */
	page = p->p_addr + v.v_usize + segoff;
	/* map a max of NPAGEPERSEG (256) 512-byte pages per segment */
	for (i = up->u_pdsize; i > 0; i -= a) {
		a = min(NPAGEPERSEG, (unsigned)i);
		setmmu((short *)((int)addr | ACCSEG), page);
		setmmu((short *)((int)addr | ACCLIM), ASRW | ((256-a) & 0xFF));
		page += a;
		addr = (short *)((long)addr + (1 << SEGSHIFT));
	}

		/* set up stack segment */
	addr = (short *)(vtoseg(v.v_uend));
	page += up->u_pssize;		/* stack is right after data */
	for (i = up->u_pssize; i > 0; i -= a) {
		addr = (short *)((long)addr - (1 << SEGSHIFT));
		a = min(NPAGEPERSEG, (unsigned)i);
		page -= NPAGEPERSEG;
		setmmu((short *)((int)addr | ACCSEG), page);
		setmmu((short *)((int)addr | ACCLIM), ASRWS | (a-1));
	}

		/* set up phys() */
	for (ph = &u.u_phys[0]; ph < &u.u_phys[v.v_phys]; ph++) {
	    if (ph->u_phsize) {
		page = (ph->u_phpaddr >> PAGESHIFT) + segoff;
		addr = (short *)vtoseg(ph->u_phladdr);
		for (i = ph->u_phsize; i > 0; i -= a) {
			a = min(NPAGEPERSEG, (unsigned)i);
			/* if ((getmmu(vtoseg(addr)|ACCLIM)&PROTMASK) == ASINVAL) { */
			if ((getmmu((short *)((int)addr | ACCLIM))&PROTMASK) == ASINVAL) {
				setmmu((short *)((int)addr | ACCSEG), page);
				setmmu((short *)((int)addr | ACCLIM), ASRW | ((256-a) & 0xFF));
			}
			page += a;
			addr = (short *)((long)addr + (1 << SEGSHIFT));
		    }
		/*dumpmm1(1);	/**** DEBUG ****/
	    }
	}

		/* set up shared memory */
	for (i = (p - proc) * shminfo.shmseg; /* index of first shm_shmem[] */
	     i < ((p - proc) + 1) * shminfo.shmseg; i++) {
		sp = shm_shmem[i];
		if (sp == NULL)
			/* no more shared mem segments this process */
			continue;
		/* shm_scat is starting physical click number */
		page = sp->shm_scat + segoff;
		addr = (short *)vtoseg(shm_pte[i].shm_segbeg);
		for (j = btoc(sp->shm_segsz); j > 0; j -= a) {
			a = min(NPAGEPERSEG, (unsigned)j);
			if ((getmmu((short *)((int)addr | ACCLIM))&PROTMASK) == ASINVAL) {
				setmmu((short *)((int)addr | ACCSEG), page);
				setmmu((short *)((int)addr | ACCLIM),
						ASRW | ((256-a) & 0xFF));
			}
			page += a;
			addr = (short *)((long)addr + (1 << SEGSHIFT));
		}
	}
	
}

setmmu(addr, data)
register short *addr;
register data;
{
	int s;

#ifdef TRACEALL
#ifdef HOWFAR
if (data != ASINVAL)
if (((int)addr & ACCSEG) == ACCSEG)
	printf("setmmu:addr=0x%x, data=0x%x (0x%x)\n", addr, data, data<<9);
else
	printf("setmmu:addr=0x%x prot=0x%x, length=0x%x (%d)\n",
		addr, data & PROTMASK, data, 256 - (data & 0xFF));
#endif
#endif
	s = spl7();
	SETUP_1 = 1;
	*addr = data;
	SETUP_0 = 1;
	splx(s);
}

getmmu(addr)
register short *addr;
{
	register data;
	int s;

	s = spl7();
	SETUP_1 = 1;
	data = *addr;
	SETUP_0 = 1;
	splx(s);
	data &= 0xFFF;
#ifdef TRACEALL
#ifdef HOWFAR
if (((int)addr & ACCSEG) == ACCSEG)
	printf("getmmu:addr=0x%x, data=0x%x (0x%x)\n", addr, data, data<<9);
else
	printf("getmmu:addr=0x%x prot=0x%x, length=0x%x (%d)\n",
		addr, data & PROTMASK, data, 256 - (data & 0xFF));
#endif
#endif
	return(data);
}

clearmmu()
{
	register data = 0xC00;		/* ASINVAL (d7) */
	register inc = 0x20000;		/* address increment (d6) */
	register s = 0;			/* saved priority (d5) */
	register short i = 32-1;	/* loop counter (d4) */
	register char *addr = (char *)0x8000;	/* address ACCLIM (a5) */

#ifdef lint
	*addr = (char)i;
	*addr = (char)inc;
	*addr = (char)data;
#endif
	s = spl7();
	SETUP_1 = 1;
	asm("loop:");
	asm("	movw	d7,a5@");
	asm("	addl	d6,a5");
	asm("	movw	d7,a5@");
	asm("	addl	d6,a5");
	asm("	movw	d7,a5@");
	asm("	addl	d6,a5");
	asm("	movw	d7,a5@");
	asm("	addl	d6,a5");
	asm("	dbra	d4,loop");
	SETUP_0 = 1;
	splx(s);
}

/*
 * In V7, Set up software prototype segmentation
 * registers to implement the 3 pseudo
 * text,data,stack segment sizes passed
 * as arguments.
 * The argument sep specifies if the
 * text and data+stack segments are to
 * be separated.
 * The last argument determines whether the text
 * segment is read-write or read-only.
 *
 * u.u_ptsize etc replace the proto entries on the pdp11.  They
 * are used by sureg to set up the page map.
 */
/* ARGSUSED */
estabur(nt, nd, ns, sep, xrw)
unsigned nt, nd, ns;
{
#ifdef HOWFAR
printf("estabur:nt=%d nd=%d ns=%d rw=%d\n", nt, nd, ns, xrw);
#endif
	if (verureg(nt, nd, ns, xrw))
		return(-1);
	sureg();
	return(0);
}

/*
 * verify user registers can be set up
 */
verureg(nt, nd, ns, xrw)
register unsigned nt;
unsigned nd, ns;
{
	register int s;

	/*
	 * check for sufficient number of segment registers
	 */
	if (ctos(nt) + ctos(nd) + ctos(ns) > ctos(btoc(v.v_uend-v.v_ustart)))
		goto bad;

	s = nd + ns + v.v_usize;
	if (nt == 0) {			/* non shared text */
		if (s > maxmem)
			goto bad;
	} else {			/* shared text */
		if (nt + s <= maxmem)	/* text+data can fit in largest hole */
			goto ok;
		goto bad;
	}
ok:
	u.u_ptsize = nt;	/* essentially these pass args to sureg */
	u.u_pdsize = nd;
	u.u_pssize = ns;
	u.u_xrw = xrw;
	return(0);
bad:
#ifdef HOWFAR
printf("verureg failure:nt=%d  nd=%d  ns=%d\n", nt, nd, ns);
#endif
	u.u_error = ENOMEM;
	return(-1);
}

#ifdef DUMPMM

char *mmu_codes[] = {
	"UNPREDICT-0",		/* 0 */
	"UNPREDICT-1",		/* 1 */
	"UNPREDICT-2",		/* 2 */
	"UNDEFINED",		/* 3 */
	"RO stack",		/* 4 */
	"RO",			/* 5 */
	"RW stack",		/* 6 */
	"RW",			/* 7 */
	"UNPREDICT-8",		/* 8 */
	"IO",			/* 9 */
	"UNPREDICT-A",		/* A */
	"UNPREDICT-B",		/* B */
	"INVALID",		/* C */
	"UNPREDICT-D",		/* D */
	"UNPREDICT-E",		/* E */
	"SPIO",			/* F */
};

/*
 * dumpmm(system)
 *	dump the memory management registers
 *	if system is non-zero, also dump system registers
 */
dumpmm(system)
int system;
{
	printf("p_addr=0x%x tsize=0x%x dsize=0x%x ssize=0x%x\n",
		u.u_procp->p_addr, u.u_ptsize, u.u_pdsize, u.u_pssize);
	if (system)
		dumpmm1(0);
	dumpmm1(1);
}

dumpmm1(space)
{
	register i, addr, prot, j, len;

	printf("Context %d mmu registers\n", space);
	printf("seg  logical physical  (clicks)  permission\n");
	for (i = 0; i < 128; i++) {
		if (space == 0)
			SEG1_0 = 1;
		else
			SEG1_1 = 1;
		/* SEG2_0 = 1; */
		addr = getmmu((i << SEGSHIFT) | ACCSEG);
		prot = getmmu((i << SEGSHIFT) | ACCLIM);
		if ((prot & PROTMASK) == ASINVAL)
			continue;
		addr -= segoff;
		len = prot & 0xFF;
		if (prot & 0x100) {		/* data or stack segment */
			j = i << SEGSHIFT;
			len = 256 - len;
			printf("0x%x  0x%x-0x%x 0x%x-0x%x (%d) %s\n", i, j,
				j+ctob(len), addr, addr+len, len,
				mmu_codes[(prot&PROTMASK)>>8]);
		} else {
			len++;
			j = (i+1) << SEGSHIFT;
			addr += NPAGEPERSEG;
			printf("0x%x  0x%x-0x%x 0x%x-0x%x (%d) %s\n", i,
				j-ctob(len), j, addr-len, addr, len,
				mmu_codes[(prot&PROTMASK)>>8]);
		}
	}
}
#endif DUMPMM
/*
 * check the size of a process
 */
chksize(nt, nd, ns)
register unsigned nt, nd, ns;
{
	if (nt + nd + ns + v.v_usize < maxmem )
		return(0);
	u.u_error = ENOMEM;
	return(1);
}
