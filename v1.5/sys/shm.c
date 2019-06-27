/* @(#)shm.c	1.7 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/proc.h"
#include "sys/context.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/var.h"
#include "sys/scat.h"

extern struct shmid_ds	shmem[];	/* shared memory headers */
extern struct shmid_ds	*shm_shmem[];	/* ptrs to attached segments */
extern struct shmpt_ds	shm_pte[];	/* segment attach points */
extern struct shminfo	shminfo;	/* shared memory info structure */
int	shmtot;		/* total shared memory currently used */

extern time_t		time;			/* system idea of date */

struct ipc_perm		*ipcget();
struct shmid_ds		*shmconv();

/*
**	shmat - Shmat system call.
*/

shmat()
{
	register struct a {
		int	shmid;
		char	*addr;
		int	flag;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	register struct user *up;
	register struct proc *p;
	register int	shmn;
	register int	segbeg;
	register struct phys *ph;
	int		i, aa, ix;
	int	as,bs,ap,bp;

	up = &u;
	p = up->u_procp;
	if ((sp = shmconv(uap->shmid, SHM_DEST)) == NULL)
		return;
	if (ipcaccess(&sp->shm_perm, SHM_R))
		return;
	if ((uap->flag & SHM_RDONLY) == 0)
		if (ipcaccess(&sp->shm_perm, SHM_W))
			return;
	for(shmn = 0;shmn < shminfo.shmseg;shmn++)
		if (shm_shmem[(p - proc)*shminfo.shmseg+shmn] == NULL)
			break;
	if (shmn >= shminfo.shmseg) {
		up->u_error = EMFILE;
		return;
	}
	if (uap->flag & SHM_RND)
		uap->addr = (char *)((uint)uap->addr & ~(SHMLBA - 1));

/*
 * Check for page alignment and containment within data space
 */
	if ((int)uap->addr & (SHMLBA - 1) ||
	    sp->shm_segsz <= 0 ||
	    ((int)uap->addr & 0x80000000) ||
	    (((int)uap->addr != 0) &&
	    (((int)uap->addr + ctob(stoc(ctos(btoc(sp->shm_segsz))))) >
	    ctob(stoc(ctos(btoc(v.v_uend)-up->u_ssize)))))) {
		up->u_error = EINVAL;
		return;
	}

/*
 * An address of 0 places the shared memory into a first fit location
 */
	if (uap->addr == NULL) {
		if (shmn > 0) {	/* there was a previous attach */
			/* try the virtual address just beyond the
			   last one attached */
			segbeg = (short)(p - proc) * (short)shminfo.shmseg +
				 shmn - 1;	/* index of last shmem */
			segbeg = shm_pte[segbeg].shm_segbeg +
				 ctob(stoc(ctos(btoc(
					shm_shmem[segbeg]->shm_segsz))));
		} else {	/* this is the 1st attach */
			segbeg = v.v_ustart + 
				 ctob(stoc(ctos(up->u_tsize))) +
				 ctob(stoc(ctos(up->u_dsize)))+
				 ctob(stoc(ctos(shminfo.shmbrk)));
		}
		/* need to avoid any phys areas */
		for (ph = &u.u_phys[0]; ph < &u.u_phys[v.v_phys]; ph++) {
			if (ph->u_phsize) {
				as = segbeg;
				bs = segbeg +
				     ctob(stoc(ctos(btoc(sp->shm_segsz))));
				ap = ph->u_phladdr;
				bp = ph->u_phladdr +
				     ctob(stoc(ctos(ph->u_phsize)));
				if ((as < ap) && (bs <= ap))
					/* shmat all before phys - ok */
					continue;
				if ((as >= bp) && (bs > bp))
					/* shmat all after phys - ok */
					continue;
				/* allocation would conflict with phys */
				/* choose another location... where? */
				up->u_error = ENOMEM;
				return;
			}
		}
	} else {
/*
 * Check to make sure segment does not overlay any valid segments
 */
		segbeg = vtoseg((int)uap->addr);
		for (i = btoc(sp->shm_segsz); i > 0; i -= aa) {
			aa = min(NPAGEPERSEG, (unsigned)i);
			if ((getmmu((short *)(segbeg|ACCLIM))&PROTMASK)!=ASINVAL) {
				up->u_error = ENOMEM;
				return;
			}
			segbeg += (1<<SEGSHIFT);
		}
		segbeg = (int)uap->addr;
	}
	if (chksize(up->u_tsize, up->u_dsize, up->u_ssize)) {
		up->u_error = ENOMEM;
		return;
	}
	i = (short)(p - proc) * (short)shminfo.shmseg + shmn;
	shm_shmem[i] = sp;
	shm_pte[i].shm_segbeg = segbeg;
	shm_pte[i].shm_sflg = ((uap->flag & SHM_RDONLY) ? RO : RW);
	shm_pte[i].shm_seg = 0;
/*	cxrelse(p->p_context); */
	sureg();
/*
 * Clear segment on first attach
 */
	if (sp->shm_perm.mode & SHM_CLEAR) {
		i = btoc(sp->shm_segsz);
#ifdef NONSCATLOAD
		ix = btoc(segbeg);
		while (--i >= 0)
			clearseg(ix++);
#else
		ix = sp->shm_scat;
		while (--i >= 0) {
			clearseg(ixtoc(ix));
			ix = scatmap[ix].sc_index;
		}
#endif
		sp->shm_perm.mode &= ~SHM_CLEAR;
	}
	if (p->p_smbeg == 0 || p->p_smbeg > segbeg)
		p->p_smbeg = segbeg;
	sp->shm_nattch++;
	sp->shm_cnattch++;
	up->u_rval1 = segbeg;
	sp->shm_atime = time;
	sp->shm_lpid = p->p_pid;
}

/*
**	shmconv - Convert user supplied shmid into a ptr to the associated
**		shared memory header.
*/

struct shmid_ds *
shmconv(s, flg)
int	s;		/* shmid */
int		flg;	/* error if matching bits are set in mode */
{
	register struct shmid_ds	*sp;	/* ptr to associated header */

	sp = &shmem[(short)(s % shminfo.shmmni)];
	if (!(sp->shm_perm.mode & IPC_ALLOC) || sp->shm_perm.mode & flg ||
		s / shminfo.shmmni != sp->shm_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}

/*
**	shmctl - Shmctl system call.
*/

shmctl()
{
	register struct a {
		int		shmid,
				cmd;
		struct shmid_ds	*arg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	struct shmid_ds			ds;	/* hold area for IPC_SET */
	register struct user		*up;

	if ((sp = shmconv(uap->shmid, (uap->cmd == IPC_STAT) ? 0 : SHM_DEST)) ==
		NULL)
		return;
	up = &u;
	up->u_rval1 = 0;
	switch(uap->cmd) {

	/* Remove shared memory identifier. */
	case IPC_RMID:
		if (up->u_uid != sp->shm_perm.uid && up->u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		sp->shm_ctime = time;
		sp->shm_perm.mode |= SHM_DEST;

		/* Change key to private so old key can be reused without
			waiting for last detach.  Only allowed accesses to
			this segment now are shmdt() and shmctl(IPC_STAT).
			All others will give bad shmid. */
		sp->shm_perm.key = IPC_PRIVATE;

		/* Adjust counts to counter shmfree decrements. */
		sp->shm_nattch++;
		sp->shm_cnattch++;
		shmfree(sp);
		break;

	/* Set ownership and permissions. */
	case IPC_SET:
		if (up->u_uid != sp->shm_perm.uid && up->u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		if (copyin((caddr_t)uap->arg, (caddr_t)&ds, sizeof(ds))) {
			up->u_error = EFAULT;
			return;
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->shm_perm.mode & ~0777);
		sp->shm_ctime = time;
		break;

	/* Get shared memory data structure. */
	case IPC_STAT:
		if (ipcaccess(&sp->shm_perm, SHM_R))
			return;
		if (copyout((caddr_t)sp, (caddr_t)uap->arg, sizeof(*sp)))
			up->u_error = EFAULT;
		break;

	default:
		up->u_error = EINVAL;
		break;
	}
}

/*
**	shmdt - Shmdt system call.
*/

shmdt()
{
	struct a {
		char	*addr;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp, **spp;
	int	segbeg;
	register struct shmpt_ds	*pt;
	register i, j;
	register struct proc *p;

/*
 * Check for page alignment
 */
	if ((int)uap->addr & (ctob(1) - 1) ||
	    (segbeg = (int)uap->addr) == 0) {
		u.u_error = EINVAL;
		return;
	}
/*
 * find segment
 */
	spp = &shm_shmem[i=((p=u.u_procp)-proc)*shminfo.shmseg];
	pt = &shm_pte[i];
	for (i=0; i < shminfo.shmseg; i++, pt++, spp++) {
		if ((sp = *spp) != NULL && pt->shm_segbeg == segbeg)
			break;
		sp = NULL;
	}
	if (sp == NULL) {
		u.u_error = EINVAL;
		return;
	}
	shmfree(sp);
	sp->shm_dtime = time;
	sp->shm_lpid = p->p_pid;
	*spp = NULL;
	pt->shm_segbeg = 0;
	p->p_smbeg = 0;
	pt = &shm_pte[(p-proc)*shminfo.shmseg];
	for (j=0; j<shminfo.shmseg; j++, pt++) {
		if (i = pt->shm_segbeg) {
			if (p->p_smbeg) {
				if (p->p_smbeg > i)
					p->p_smbeg = i;
			} else {
				p->p_smbeg = i;
			}
		}
	}
/*	cxrelse(p->p_context); */
	sureg();
	u.u_rval1 = 0;
}

/*
**	shmexec - Called by setregs(os/sys1.c) to handle shared memory exec
**		processing.
*/

shmexec()
{
	register struct shmid_ds	**spp;	/* ptr to ptr to header */
	register struct shmpt_ds	*sppp;	/* ptr to pte data */
	register int			i;	/* loop control */

	if (u.u_procp->p_smbeg == 0)
		return;
	/* Detach any attached segments. */
	sppp = &shm_pte[i = (u.u_procp - proc)*shminfo.shmseg];
	u.u_procp->p_smbeg = 0;
	spp = &shm_shmem[i];
	for(i = 0; i < shminfo.shmseg; i++,spp++,sppp++) {
		if (*spp == NULL)
			continue;
		shmfree(*spp);
		*spp = NULL;
		sppp->shm_segbeg = 0;
	}
}

/*
**	shmexit - Called by exit(os/sys1.c) to clean up on process exit.
*/

shmexit()
{
	/* Same processing as for exec. */
	shmexec();
}

/*
**	shmfork - Called by newproc(os/slp.c) to handle shared memory fork
**		processing.
*/

shmfork(cp, pp)
struct proc	*cp,	/* ptr to child proc table entry */
		*pp;	/* ptr to parent proc table entry */
{
	register struct shmid_ds	**cpp,	/* ptr to child shmem ptrs */
					**ppp;	/* ptr to parent shmem ptrs */
	register struct shmpt_ds	*cppp,
					*pppp;
	register int			i;	/* loop control */

	if (pp->p_smbeg == 0)
		return;
	/* Copy ptrs and update counts on any attached segments. */
	cpp = &shm_shmem[(cp - proc)*shminfo.shmseg];
	ppp = &shm_shmem[(pp - proc)*shminfo.shmseg];
	cppp = &shm_pte[(cp - proc)*shminfo.shmseg];
	pppp = &shm_pte[(pp - proc)*shminfo.shmseg];
	cp->p_smbeg = pp->p_smbeg;
	for(i = 0;i < shminfo.shmseg; i++, cpp++, ppp++, cppp++, pppp++) {
		if (*cpp = *ppp) {
			(*cpp)->shm_nattch++;
			(*cpp)->shm_cnattch++;
			cppp->shm_segbeg = pppp->shm_segbeg;
			cppp->shm_sflg = pppp->shm_sflg;
			cppp->shm_seg = 0;
		}
	}
}

/*
**	shmfree - Decrement counts.  Free segment and page tables if
**		indicated.
*/

shmfree(sp)
register struct shmid_ds	*sp;	/* shared memory header ptr */
{
	register int size;

	if (sp == NULL)
		return;
	sp->shm_nattch--;
	if (--(sp->shm_cnattch) == 0 && sp->shm_perm.mode & SHM_DEST) {
		size = btoc(sp->shm_segsz);
#ifdef NONSCATLOAD
		mfree(coremap, size, (int)sp->shm_scat);
#else
		memfree((int)sp->shm_scat);
#endif
		/* adjust maxmem for amount freed */
		maxmem += size;
		shmtot -= size;

		sp->shm_perm.mode = 0;
		if (((int)(++(sp->shm_perm.seq)*shminfo.shmmni + (sp - shmem))) < 0)
			sp->shm_perm.seq = 0;
	}
}

/*
**	shmget - Shmget system call.
*/

shmget()
{
	register struct a {
		key_t	key;
		int	size,
			shmflg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	int				s;	/* ipcget status */
	register int			size;

	/*if (uap->size == 0)	/* Bug #675 ... Paul */
	/*	return;*/
	if ((sp = (struct shmid_ds *)ipcget(uap->key, uap->shmflg,
	    (struct ipc_perm *)shmem, shminfo.shmmni, sizeof(*sp), &s)) == NULL)
		return;
	if (s) {
		/* This is a new shared memory segment.  Allocate memory and
			finish initialization. */
		if (uap->size < shminfo.shmmin || uap->size > shminfo.shmmax) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			return;
		}
		size = btoc(uap->size);
		if (shmtot + size > shminfo.shmall) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			return;
		}
		sp->shm_segsz = uap->size;
#ifdef NONSCATLOAD
		if ((sp->shm_scat = malloc(coremap, size)) == 0) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			return;
		}
#else
		if ((sp->shm_scat = memalloc(size)) == 0) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			return;
		}
#endif
		/* adjust maxmem for the segment */
		maxmem -= size;
		shmtot += size;

		sp->shm_perm.mode |= SHM_CLEAR;
		sp->shm_nattch = 0;
		sp->shm_cnattch = 0;
		sp->shm_atime = 0;
		sp->shm_dtime = 0;
		sp->shm_ctime = time;
		sp->shm_lpid = 0;
		sp->shm_cpid = u.u_procp->p_pid;
	} else
		if (uap->size && uap->size > sp->shm_segsz) {
			u.u_error = EINVAL;
			return;
		}
	u.u_rval1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
}

/*
**	shmsys - System entry point for shmat, shmctl, shmdt, and shmget
**		system calls.
*/

shmsys()
{
	register struct a {
		uint	id;
	}	*uap = (struct a *)u.u_ap;
	int		shmat(),
			shmctl(),
			shmdt(),
			shmget();
	static int	(*calls[])() = {shmat, shmctl, shmdt, shmget};

	if (uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}

#ifdef notdef
shmreset(p, ub, p0br, p0lr)
struct proc *p;
struct user *ub;
int *p0br, p0lr;
{
	register struct shmid_ds **sp;
	register struct shmpt_ds *pt;
	register i,j;
	register int *seg, shm, *pte;

/*
 * do only if there is shared memory attached
 */
	if (p->p_smbeg == 0)
		return;
/*
 * clear unused pte's
 */
	seg = p0br + ub->u_tsize + ub->u_dsize;
	for (i = ub->u_tsize + ub->u_dsize; i < p0lr; i++)
		*seg++ = 0;
/*
 * move in the shared memory segments
 */
	sp = &shm_shmem[i = (p - proc)*shminfo.shmseg];
	pt = &shm_pte[i];
	for (i = 0; i < shminfo.shmseg; i++, sp++, pt++) {
		if (shm = pt->shm_segbeg) {
			seg = p0br + shm;
			pte = (int *)((*sp)->shm_ptbl);
			for (j = 0; j < btoc((*sp)->shm_segsz); j++)
				*seg++ = *pte++ | PG_V | pt->shm_sflg;
		}
	}
}
#endif

#ifdef notdef
dumppte(p0br,p0lr,p1lr,p1br)
int *p0br, p0lr, p1lr, *p1br;
{
	register i;

	printf("tsize %d, dsize %d\n", u.u_tsize, u.u_dsize);
	printf("p0br %x p1br %d\np1br %x p1lr %d\n\n",p0br,p0lr,p1br,p1lr);
	for (i=0; i<p0lr; i++) {
		if ((i%8) == 0)
			printf("\n");
		printf("%x  ",*p0br++);
	}
	printf("\n\n\n");
}
#endif
