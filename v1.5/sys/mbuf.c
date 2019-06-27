/*	mbuf.c	1.36	82/06/20	*/

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "net/misc.h"
#include "net/mbuf.h"
#include "sys/map.h"
#include "net/in_systm.h"		/* XXX */


/* some random constants, ints */

/*  THIS SHOULD BE AN EVEN CLICK NUMBER OF BYTES !!! */
/*
#define IOSIZE  8192            /* area for DMA buffers */
#define IOSIZE  0	        /* no space for DMA buffers for multibus */


/* unsigned int   miosize = IOSIZE; */
unsigned int   miobase;                /* loc of DMA area. */


/*
 * Initialize the buffer pool.  Called from netinit/main.
 */
mbinit()
{
	register i;
	register struct mbuf *m;
	extern struct mbuf * mballoc();

	/* allocate mbuf io area. iomalloc is machine-dependent... */
	/* miobase = mbioalloc(btoc(miosize));  */
	miobase = mbioalloc(); 
	/* link the mbufs */
	m = mballoc();
	/*
	printf("mbufs at %x\n", m);
	*/
	mbstat.m_mbufs = NMBUFS;

	for(i=0 ; i<NMBUFS ; i++) {
		m->m_off = 0;
		m->m_free = 0;
		bzero((char *)m, MSIZE);
		(void) m_free(m);
		m++;
	}
}



/* NEED SOME WAY TO RELEASE SPACE */

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
/* ARGSUSED */
struct mbuf *
m_get(canwait)
	int canwait;
{
	register struct mbuf *m;
#ifdef	PRMDEF
	struct call {
		long	local;
		long	frmptr;
		long	raddr;
	};
	int i;
	register struct call * cp = &i;
	extern dog_pr;
	register olddogpr = dog_pr;

	if (dog_pr) {printf("<mget from %x>",cp->raddr); dog_pr = 0;}
#endif

	MGET(m, canwait);
#ifdef  PRMDEF
	if (olddogpr)
		dog_pr = 1;
#endif
	return (m);
}

struct mbuf *
m_free(m)
	struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n);
	return (n);
}

m_freem(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	register int s;

	if (m == NULL)
		return;
	s = splimp();
	do {
		MFREE(m, n);
	} while (m = n);
	splx(s);
}

/*
 * Mbuffer utility routines.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy1");
	while (off > 0) {
		if (m == 0)
			panic("m_copy2");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	MAPSAVE();
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy3");
			break;
		}
		MGET(n, 1);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		{
			n->m_off = MMINOFF;
			MBCOPY(m,off,n,0,(unsigned)n->m_len);
		}
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	goto out;
nospace:
	m_freem(top);
	top = 0;
out:
	MAPREST();
	return (top);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{

	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		MBCOPY(n, 0, m, m->m_len, (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m, *n;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/* a 2 pass algorithm might be better */
		len = -len;
		while (len > 0 && m->m_len != 0) {
			while (m != NULL && m->m_len != 0) {
				n = m;
				m = m->m_next;
			}
			if (n->m_len <= len) {
				len -= n->m_len;
				n->m_len = 0;
				m = mp;
			} else {
				n->m_len -= len;
				break;
			}
		}
	}
}

struct mbuf *
m_pullup(m0, len)
	struct mbuf *m0;
	int len;
{
	register struct mbuf *m, *n;
	int count;

	n = m0;
	if (len > MLEN)
		goto bad;
	MGET(m, 0);
	if (m == 0)
		goto bad;
	m->m_off = MMINOFF;
	m->m_len = 0;
	do {
		count = MIN(MLEN - m->m_len, len);
		if (count > n->m_len)
			count = n->m_len;
		MBCOPY(n, 0, m, m->m_len, (u_int)count);
		len -= count;
		m->m_len += count;
		n->m_off += count;
		n->m_len -= count;
		if (n->m_len)
			break;
		n = m_free(n);
	} while (n);
	if (len) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

#ifdef notdef
/*
 * Allocate a contiguous buffer for DMA IO.  Called from if_ubainit().
 * TODO: fix net device drivers to handle scatter/gather to mbufs
 * on their own; thus avoiding the copy to/from this area.
 */
unsigned int
m_ioget(size)
{
	unsigned int base;

	size = ((size + 077) & ~077);   /* round up byte size */
	if (size > miosize) return(0);
	miosize -= size;
	base = miobase;
	miobase += size;
	return(base);
}
#endif

#ifdef debug
mbprint(m,s)
register struct mbuf *m;
char *s;
{
	extern enprint;
	register char *ba;
	int col,i,bc;

	if (enprint == 0) return;
	MAPSAVE();
	nprintf("MB %s\n",s);
	for (;;) {
		if (m == 0) break;
		ba = mtod(m, char *);
		col = 0;  bc = m->m_len;
		nprintf("m%o next%o off%o len%o act%o free%o\n",
			m, m->m_next, m->m_off, m->m_len, m->m_click,
			m->m_free);
		for(; bc ; bc--) {
			i = *ba++ & 0377;
			nprintf("%o ",i);
			if(++col > 31) {
				col = 0;
				nprintf("\n  ");
			}
		}
		nprintf("\n");
		m = m->m_next;
	}
	MAPREST();
}

#else
mbprint(m,s)
register struct mbuf *m;
char *s;
{
#ifdef lint
	mbprint(m, s);
#endif
}
#endif debug

#ifdef notdef
mcheck(m, msg)
struct mbuf *m;
char * msg;
{
	extern char mbufbufs[];
	extern struct mbuf * mfreep;
	register x, ret = 0;

	x = spl7();
	if ( ((m < (struct mbuf *)&mbufbufs[0]) && (m != 0)) ||
		(m > (struct mbuf *)&mbufbufs[(NMBUFS+1)*MSIZE]) || 
		((mfreep < (struct mbuf *)&mbufbufs[0]) && (mfreep != 0)) ||
		(mfreep > (struct mbuf *)&mbufbufs[(NMBUFS+1)*MSIZE]) ) {
			printf ("mcheck fail; m, mfreep = %x, %x, from %s\n",
			m,mfreep,msg);
			ret = 1;
	}
	splx(x);
	return ret;
}
#endif

#ifdef	PRMDEF
struct call {
	long	local;
	long	frmptr;
	long	raddr;
};

int dog_pr;
int dof_pr;

prmget()
{
	int i;
	register struct call * cp = &i;

	if (dog_pr) printf("<MGET from %x>",cp->raddr);
}


prmfree()
{
	int i;
	register struct call * cp = &i;

	if (dof_pr) printf("[MFREE from %x]",cp->raddr);
}

#endif
