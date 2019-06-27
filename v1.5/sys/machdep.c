/*#define HOWFAR	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/context.h"
#include "sys/seg.h"
#include "sys/map.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/utsname.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/var.h"
#include "sys/scat.h"
#include "setjmp.h"

typedef int	mem_t;

#define LOW	(USTART&0xFFFF)		/* Low user starting address */
#define HIGH	((USTART>>16)&0xFFFF)	/* High user starting address */
#define MEMFACT	8/10	/* swap space to free memory minimum ratio */

int	keroff;		/* kernel memory offset */

extern short	segoff;	/* mmu segment offset */

/*
 * Icode is the bootstrap program used to exec init.
 */
short icode[] = {
				/*	 .=USTART		*/
	0x2E7C,	HIGH, LOW+0x100,/*	 movl	#USTART+0x100,sp*/
	0x227C,	HIGH, LOW+0x26,	/*	 movl	#envp,a1	*/
	0x223C,	HIGH, LOW+0x22,	/*	 movl	#argp,d1	*/
	0x207C,	HIGH, LOW+0x2A,	/*	 movl	#name,a0	*/
	0x42A7,			/*	 clrl	sp@-		*/
	0x303C,	0x3B,		/*	 movw	#59.,d0		*/
	0x4E40,			/*	 trap	#0		*/
	0x60FE,			/*	 bra	.		*/
	HIGH,	LOW+0x2A,	/* argp: .long	name		*/
	0,	0,		/* envp: .long	0		*/
	0x2F65,	0x7463,	0x2F69,	/* name: .asciz	"/etc/init"	*/
	0x6E69,	0x7400
};
int szicode = sizeof(icode);

/*
 * startup code
 */
/* ARGSUSED */
startup(cudot)
{
	extern int nsysseg, dispatch[];
	extern caddr_t end, etext;
	extern struct var v;
	register caddr_t cp, limit;
	register long *ip;

	for (ip = &((long *) 0)[0]; ip < &((long *) 0)[NIVEC]; ip++)
		*ip = (long)dispatch + (long)ip;

	/*
	 * free swap memory
	 */
	mfree(swapmap, nswap, 1);

	keroff = btoc((unsigned)&end) + v.v_usize;
	cp = (caddr_t)(ctob(btoc((unsigned)&end)+v.v_usize));
	limit = (caddr_t)(((*((long *)MEMEND)) & 0xFFFFFE00)-ctob(1));

#ifdef HOWFAR
	printf("Segment offset = 0x%x, first click at %d(0x%x)\n\r",
		segoff, keroff, keroff);
	printf("First free memory = 0x%x\n", cp);
	printf("Last free memory = 0x%x\n", limit);
	printf("%d clicks available\n",btoc((int)limit)-keroff);
#endif
	for ( ; cp < limit; cp += ctob(1)) {
		clearseg((int)btoc((int)cp));
		maxmem++;
	}
	cp = (caddr_t)(btoc((unsigned)&end)+v.v_usize);
	mfree(coremap, maxmem, (int)cp);

	printf("Available user memory is %d bytes\n\r", ctob((long)maxmem));
	if (maxmem > dtoc(nswap*MEMFACT)) {
		maxmem = dtoc(nswap*MEMFACT);
		printf("Insufficient swap space for available memory.\n");
		printf("Largest runnable process is %d.\n", ctob(maxmem));
	}
	if (MAXMEM < maxmem) maxmem = MAXMEM;
}

/*
 * iocheck - check for an I/O device at given address
 */
iocheck(addr)
caddr_t addr;
{
	register int *saved_jb;
	register int i;
	jmp_buf jb;

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		i = *addr;
		i = 1;
	} else
		i = 0;
	nofault = saved_jb;
	return(i);
}

/*
 * usraccess - Check a virtual address for user accessibility.
 */
usraccess(virt, access)
long virt;
{
	register prot;

	SEG1_1 = 1;	/* user context */
	/* SEG2_0 = 1;	/* user context */

	prot = getmmu((short *)(vtoseg(virt) | ACCLIM)) & PROTMASK;
/* printf("usraccess at 0x%x is 0x%x\n", virt, prot); */
	if (prot == ASRW || prot == ASRWS || (prot == ASRO && access == RO))
			return(1);
	return(0);
}

/*
 * vtop - Convert virtual address to physical.
 */
caddr_t
vtop(virt)
register caddr_t virt;
{
	long segbase, phys;

	SEG1_1 = 1;	/* user context */
	/* SEG2_0 = 1;	/* user context */

	segbase = getmmu((short *)(vtoseg(virt) | ACCSEG)) & SEGBASE;
	phys = (segbase << VIRTSHIFT) + ((long)virt & OFFMASK);
	return((caddr_t)(phys - ctob(segoff)));
}

/*
 * fuword - Fetch word from user space.
 */
fuword(addr)
register caddr_t addr;
{
	int val;
	jmp_buf jb;
	int *saved_jb;

	if (addr < (caddr_t)v.v_ustart || addr+3 >= (caddr_t)v.v_uend)
		return (-1);
	if ((addr = (caddr_t)vtop(addr)) == (caddr_t)-1)
		return(-1);
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		val = *(int *)addr;
	} else
		val = -1;
	nofault = saved_jb;
	return(val);
}


/*
 * fubyte - Fetch byte from user space.
 */
fubyte(addr)
register caddr_t addr;
{
	register int val;
	jmp_buf jb;
	int *saved_jb;

	if (addr < (caddr_t)v.v_ustart || addr >= (caddr_t)v.v_uend)
		return (-1);
	if ((addr = (caddr_t)vtop(addr)) == (caddr_t)-1)
		return(-1);
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		val = *addr & 0377;
	} else
		val = -1;
	nofault = saved_jb;
	return(val);
}

/*
 * suword - Store word into user space.
 */
suword(addr, word)
register caddr_t addr;
int word;
{
	jmp_buf jb;
	int val, *saved_jb;
	
	if (addr < (caddr_t)v.v_ustart || addr+3 >= (caddr_t)v.v_uend)
		return (-1);
	if ((addr = (caddr_t)vtop(addr)) == (caddr_t)-1)
		return(-1);
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		*(int *)addr = word;
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
	return(val);
}

/*
 * subyte - Store byte into user space.
 */
subyte(addr, byte)
register caddr_t addr;
char byte;
{
	jmp_buf jb;
	int val, *saved_jb;
	
	if (addr < (caddr_t)v.v_ustart || addr >= (caddr_t)v.v_uend)
		return (-1);
	if ((addr = (char *)vtop(addr)) == (char *)-1)
		return(-1);
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		*addr = byte;
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
	return(val);
}

/*
 * copyout - Move bytes out of the system into user's address space.
 */
copyout(from, to, nbytes)
caddr_t from, to;
int nbytes;
{
	int val;
	jmp_buf jb;
	int *saved_jb;

#ifdef DGETPUT
printf("copyout:copying %d bytes to user space 0x%x (p=0x%x) from 0x%x\n",
    nbytes, to, vtop(to), from);
#endif
	if ((int)to+nbytes > v.v_uend || usraccess((long)to, RW) == 0 ||
	    (to = (char *)vtop(to)) == (char *)-1) {
		printf("copyout failed\n");
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		blt(to, from, nbytes);
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
	return(val);
}

/*
 * copyin - Move bytes into the system space out of user's address space.
 */
copyin(from, to, nbytes)
caddr_t from, to;
int nbytes;
{
	int val;
	jmp_buf jb;
	int *saved_jb;
	
#ifdef DGETPUT
printf("copyin:copying %d bytes from user space 0x%x (p=0x%x) to 0x%x\n",
    nbytes, from, vtop(from), to);
#endif
	if ((int)from+nbytes > v.v_uend || usraccess((long)from, RO)==0 ||
	    (from = (char *)vtop(from)) == (char *)-1) {
		printf("copyin failed\n");
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		blt(to, from, nbytes);
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
	return(val);
}

/*
 * copyseg - Copy one click's worth of data.
 */
copyseg(from, to)
int from, to;
{
	if (from == to)
		return;
	blt512(ctob(to), ctob(from), ctob(1)>>9);
}

/*
 * clearseg - Clear one click's worth of data.
 */
clearseg(where)
int where;
{
/*
printf("clearseg 0x%x (0x%x)\n", where, ctob(where));
*/
	clear((caddr_t)ctob(where), ctob(1));
}

/*
 * busaddr - Save the info from a bus or address error.
 */
/* VARARGS */
busaddr(frame)
struct {
	long regs[4];	/* d0,d1,a0,a1 */
	int baddr;	/* bsr return address */
	short fcode;	/* fault code */
	int aaddr;	/* fault address */
	short ireg;	/* instruction register */
} frame;
{
	u.u_fcode = frame.fcode;
	u.u_aaddr = frame.aaddr;
	u.u_ireg = frame.ireg;
}

/*
 * dophys - machine dependent set up portion of the phys system call
 */
dophys(phnum, laddr, bcount, phaddr)
unsigned phnum, laddr, phaddr;
register unsigned bcount;
{
	register struct phys *ph;
	register struct user *up;
	register unsigned addr;
	register i;

	up = &u;
	if (phnum >= v.v_phys)
		goto bad;
	ph = &up->u_phys[(short)phnum];
	ph->u_phladdr = 0;
	ph->u_phsize = 0;
	ph->u_phpaddr = 0;
	sureg();
	if (bcount == 0)
		return;
	/* valid logical address ? */
	addr = laddr;
	if (addr & (~SEGMASK))
		goto bad;
	i = ctos(btoc(bcount));
	while (i-- > 0) {
		if ((getmmu((short *)(vtoseg(addr) | ACCLIM)) & PROTMASK) !=
		     ASINVAL) {
#ifdef HOWFAR
			printf("addr=0x%x  prot=0x%x\n",
				addr, *(short *)vtoseg(addr));
#endif
			goto bad;
		}
		addr += (1<<SEGSHIFT);
	}
	if ((ctos(btoc(v.v_uend))-ctos(btoc(v.v_ustart))-ctos(up->u_ptsize)-
	    ctos(up->u_pdsize)-ctos(up->u_pssize)-ctos(btoc(bcount))) < 0)
		goto bad;
	ph->u_phladdr = laddr;
	ph->u_phsize = btoc(bcount);
	ph->u_phpaddr = phaddr;
	goto out;
bad:
	up->u_error = EINVAL;
out:
	/* cxrelse(up->u_procp->p_context); */
	sureg();
}

/*
 * Scan phys array for physical I/O (raw I/O) transfer verification
 */
chkphys(base, limit)
{
	register struct phys *ph;
	register i;

	for (i = 0, ph = &u.u_phys[0]; i < v.v_phys; i++, ph++)
		if (ph->u_phsize != 0 && base >= ph->u_phladdr &&
		    limit < ph->u_phladdr+ctob(ph->u_phsize))
			return(1);
	return(0);
}

/*
 * addupc - Take a profile sample.
 */
addupc(pc, p, incr)
unsigned pc;
register struct {
	short	*pr_base;
	unsigned pr_size;
	unsigned pr_off;
	unsigned pr_scale;
} *p;
{
	union {
		int w_form;		/* this is 32 bits on 68000 */
		short s_form[2];
	} word;
	register short *slot;

	slot = &p->pr_base[((((pc - p->pr_off) * p->pr_scale) >> 16) + 1)>>1];
	if ((caddr_t)slot >= (caddr_t)(p->pr_base) &&
	    (caddr_t)slot < (caddr_t)((unsigned)p->pr_base + p->pr_size)) {
		if ((word.w_form = fuword((caddr_t)slot)) == -1)
			u.u_prof.pr_scale = 0;	/* turn off */
		else {
			word.s_form[0] += (short)incr;
			(void) suword((caddr_t)slot, word.w_form);
		}
	}
}

/*
 * backup - Prepare for restart on a bus error.
 */
backup(ap)
register int *ap;
{
	register caddr_t pc;
	register ins;
	extern int tstb;

	pc = (caddr_t)ap[PC] - 2;
	ins = fuword((caddr_t)pc);
#ifdef HOWFAR
	printf("backup: pc, ins, tstb = %x, %x, %x.   ", pc, ins, tstb);
#endif
	if (ins & 0x8000 && (tstb & 0xFFFF0000) == (ins & 0xFFFF0000)) {
		ap[PC] = (int)pc;
		return(ins | 0xFFFF0000);
	}
	return(0);		/* signify we could not back up */
}

/*
 * sendsig - Simulate an interrupt.
 */
/* ARGSUSED */
sendsig(p, signo)
caddr_t p;
{
	register unsigned long n;
	register int *regp;
	short ps;

	regp = u.u_ar0;
	n = regp[SP] - 6;
	ps = (short)regp[RPS];
	(void) subyte((caddr_t)n, ps >> 8);	/* high order byte of ps */
	(void) subyte((caddr_t)(n+1), ps);	/* low order byte of ps */
	(void) suword((caddr_t)(n+2), regp[PC]);
	regp[SP] = n;
	regp[RPS] &= ~PS_T;
	regp[PC] = (int)p;
}

/*ARGSUSED*/
clkset(oldtime)
time_t	oldtime;
{
	/* use real time clock value instead of oldtime */
	time = rtcdoread();
}


/*
 * create a duplicate copy of a process
 */
procdup(p)
register struct proc *p;
{
	register a1, a2, n;

	n = p->p_size;
	if ((a2 = malloc(coremap, n)) == NULL)
		return(NULL);

	a1 = p->p_addr;
	p->p_addr = a2;
	while(n--)
		copyseg(a1++, a2++);
	return(1);
}
