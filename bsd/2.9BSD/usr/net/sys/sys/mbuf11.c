/*      mbuf11.c  1.01    82/09/23        */

#include "param.h"
#ifdef	UCB_NET
#include <sys/seg.h>
#include <sys/mbuf.h>
#include "../net/in_systm.h"


#define debug   1               /* consistency checks */

#ifdef debug
#define MBTRACE(type,arg) mbtrace(type,arg)
#else
#define MBTRACE(t,a)
#endif

#define IOSIZE  8192            /* area for DMA buffers */
#define NMBUFS  60              /* mbuf area = NMBUFS*MSIZE */
#define NMBCACHE 8              /* cache of mbufs */
#define NWORDS 2000             /* init arena size, bytes=(NWORDS-2)*WORD */

#define MBXGET(c) { \
	if ((c) = mbfree) { \
		mapseg5(mbfree,MBMAPSIZE); if (MBX->m_ref) panic("MBXGET"); \
		MBX->m_ref = 1;  MBTRACE(6,mbfree);  mbfree = (u_int)MBX->m_mbuf; }}

#define MBXFREE(c) { \
	mapseg5(c,MBMAPSIZE); if (MBX->m_ref != 1) panic("MBXFREE"); \
	MBX->m_ref = 0;  MBTRACE(7,c);  MBX->m_mbuf = (struct mbuf *)mbfree; \
	mbfree = (c); }

struct mbuf *mbcache[NMBCACHE]; /* mbuf cache for fast allocation */
int     mbicache;               /* # entries in cache and index of next add */

u_int   mbbase;                 /* click address of mbuffer base, filled
				   in by mch.s */
u_int   mbsize = (NMBUFS*MSIZE) + IOSIZE;       /* used by mch.s */
u_int   mbend;
u_int   miobase;                /* click of DMA area */
u_int   miosize = IOSIZE;
u_int   mbfree;                 /* free list */

u_int   mbhalt = 1;             /* halt on any drop (for debugging) */


/*
 * Get an mbuf, consisting of an inaddress header (mbuf) and
 * an external data portion (mbufx).  Try to save some time by using
 * the cache.
 */
struct mbuf *
m_bget()
{
	register struct mbuf *m;
	int s = splimp();
	u_int click;

	if (mbicache) {
		m = mbcache[--mbicache];
		mbstat.m_mbfree--;
		splx(s);
		MBTRACE(0,m);
		m->m_next = 0;
		return(m);
	}
	/* otherwise do it the hard way */
	MAPSAVE();
	MSGET(m, struct mbuf, 0);
	if (m == 0)
		goto bad;
	MBXGET(click);
	if (!click) {
		MSFREE(m);
		mbstat.m_drops++;
		if (mbhalt) panic("mb drop");
		goto bad;
	}
	m->m_click = click;
	m->m_next = m->m_act = m->m_len = 0;
	m->m_off = MMINOFF;
	MBX->m_mbuf = m;
	mbstat.m_mbfree--;
	mbstat.m_mbufs = MIN(mbstat.m_mbfree,mbstat.m_mbufs);
	goto out;
bad:
	m = 0;
out:
	MAPREST();
	splx(s);
	MBTRACE(1,m);
	return (m);
}

/*
 * Return an mbuf.  If not the last reference, just return the header;
 * else return both mbuf/mbufx.  Use the cache if empty slot exists.
 */
struct mbuf *
m_bfree(m)
	struct mbuf *m;
{
	register struct mbuf *n;
	int s = splimp();
	u_int click;

	MAPSAVE();
	n = m->m_next;
#ifdef debug
	MBTRACE(2,m);
	if (m->m_click < mbbase || m->m_click > mbend)
		panic("m_bf");
#endif
	click = m->m_click;
	mapseg5(click,MBMAPSIZE);
	if (MBX->m_ref != 1) {
		if (MBX->m_ref < 1 || MBX->m_ref > 4) panic("m_bf2");
		MBX->m_ref--;
		MSFREE(m);
		goto out;
	}
	mbstat.m_mbfree++;
	if (mbicache < NMBCACHE) {
		int *ip = (int *) m; ip--;
		if ((*ip & 01) == 0) panic("m_bf3");
		mbcache[mbicache++] = m;
		goto out;
	}
	MSFREE(m);
	MBXFREE(click);
out:
	MAPREST();
	splx(s);
	return (n);
}

/*
 * Allocate a contiguous buffer for DMA IO.  Called from if_ubainit().
 * TODO: fix net device drivers to handle scatter/gather to mbufs
 * on their own; thus avoiding the copy to/from this area.
 */
u_int
m_ioget(size)
{
	u_int base;

	size = ((size + 077) & ~077);   /* round up byte size */
	if (size > miosize) return(0);
	miosize -= size;
	base = miobase;
	miobase += (size>>6);
	MBTRACE(3,base);
	return(base);
}

/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BLOCK, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT int
#define ALIGN int
#define NALIGN 1
#define WORD sizeof(union store)
#define BLOCK 1024	/* a multiple of WORD*/
#define BUSY 1
#define NULL 0

#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store { union store *ptr;
	      ALIGN dummy[NALIGN];
	      int calloc;	/*calloc clears an array of integers*/
};

union store allocs[NWORDS];  /*initial arena*/
union store *allocp;    /*search ptr*/
union store *alloct;    /*arena top*/

#ifdef debug
#define ASSERT(p) if(!(p))botch("p");else
botch(s)
char *s;
{
	printf("botch %s\n",s);
	panic("mbuf11");
}
#ifdef longdebug
allock()
{
	register union store *p;
	int x;
	x = 0;
	for(p= &allocs[0]; clearbusy(p->ptr) > p; p=clearbusy(p->ptr)) {
		if(p==allocp)
			x++;
	}
	ASSERT(p==alloct);
	return(x==1||p==allocp);
}
#else
#define allock() (1)
#endif longdebug
#else
#define ASSERT(p)
#endif debug

char *
m_sget(nbytes,clr)
unsigned nbytes,clr;
{
	register union store *p, *q;
	register nw;
	int temp,val;
	int s = splimp();
#define ret(v) { val = (v); goto out; }

	nw = (nbytes+WORD+WORD-1)/WORD;
	ASSERT(allocp>=allocs && allocp<=alloct);
	ASSERT(allock());
	for(p=allocp; ; ) {
		for(temp=0; ; ) {
			if(!testbusy(p->ptr)) {
				while(!testbusy((q=p->ptr)->ptr)) {
					ASSERT(q>p&&q<alloct);
					p->ptr = q->ptr;
				}
				if(q>=p+nw && p+nw>=p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if(p>q)
				ASSERT(p<=alloct);
			else if(q!=alloct || p!=allocs) {
				ASSERT(q==alloct&&p==allocs);
				ret(NULL);
			} else if(++temp>1)
				break;
		}
		/* malloc would call sbrk here and try again */
		mbstat.m_drops++;
		if (mbhalt) panic("mb drop");
		ret(NULL);
	}
found:
	allocp = p + nw;
	ASSERT(allocp<=alloct);
	if(q>allocp) {
		allocp->ptr = p->ptr;
	}
	p->ptr = setbusy(allocp);
	mbstat.m_clfree -= /* (clearbusy(p->ptr) - p) */ nw * WORD;
	mbstat.m_clusters = MIN(mbstat.m_clusters, mbstat.m_clfree);
	ret((char *)(p+1));
out:
	splx(s);
	if(val && clr)
		bzero(val,((nbytes+1)& ~1));
	MBTRACE(4,val);
	return(val);
}

/*	freeing strategy tuned for LIFO allocation
*/
m_sfree(ap)
register char *ap;
{
	register union store *p = (union store *)ap;
	int s = splimp();

	MBTRACE(5,ap);
	/* ASSERT(p>clearbusy(allocs[1].ptr)&&p<=alloct); */
	ASSERT(p>allocs&&p<=alloct);
	ASSERT(allock());
	/* allocp = --p;   leaves old blocks around longer for debugging */
		--p;
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > p && p->ptr <= alloct);
	mbstat.m_clfree += ((p->ptr) - p) * WORD;
	splx(s);
}

/*
 * Mbuf address to data address, with bounds checking.  Called from
 * "mtod" macro which does type cast.
 */
mtodf(m)
register struct mbuf *m;
{
	if (m < (struct mbuf *)&allocs[0] || m > (struct mbuf *) alloct
#ifdef debug
	    || m->m_click < mbbase || m->m_click > mbend
	    || m->m_off < MMINOFF || m->m_off > MMAXOFF
	    || m->m_len < 0 || m->m_len > MLEN
#endif
		) panic("mtodf");
	mapseg5(m->m_click,MBMAPSIZE);
	MBX->m_mbuf = m;
	return ((int)MBX + m->m_off);
}

/*
 * Initialize the buffer pool.  Called from netinit/main.
 */
mbinit()
{
	register i;
	u_int base;

	MAPSAVE();
	/* setup the arena */
	allocs[0].ptr = &allocs[NWORDS-1];
	allocs[NWORDS-1].ptr = setbusy(&allocs[0]);
	alloct = &allocs[NWORDS-1];
	allocp = &allocs[0];
	mbstat.m_clusters = mbstat.m_clfree = (NWORDS-2) * WORD;
	/* setup DMA IO area */
	miobase = mbbase;
	mbbase += (IOSIZE>>6);
	mbsize -= IOSIZE;
	/* link the mbufs */
	mbstat.m_mbufs = mbstat.m_mbfree = NMBUFS;
	base = mbbase;
	mbend = mbbase + (mbsize >> 6);

	for(i=0 ; i<NMBUFS ; i++) {
		mapseg5(base,MBMAPSIZE);
		MBX->m_ref = 1;
		MBXFREE(base);
		base += (MSIZE>>6);
	}
	MAPREST();
}


#ifdef debug

#define NMBTBUF 40

struct mbtbuf {
	u_int   mt_type;        /* type plus KISA6 */
	u_int   mt_pc;
	u_int   mt_arg;
} mbtbuf[NMBTBUF], *mbitbuf = mbtbuf;

mbtrace(type,arg)
{
	int s = spl7();
	register int *ip;
	register struct mbtbuf *mt = mbitbuf;
	extern int _ovno;

	if (++mbitbuf >= &mbtbuf[NMBTBUF])
		mbitbuf = mbtbuf;
	mt->mt_type = (type << 12) | _ovno;
	ip = &type;  ip--;  ip--;
	ip = *ip;  ip++;
	mt->mt_pc = *ip;
	mt->mt_arg = arg;
	splx(s);
}

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
		nprintf("m%o next%o off%o len%o click%o act%o back%o ref%o\n",
			m, m->m_next, m->m_off, m->m_len, m->m_click, m->m_act,
			MBX->m_mbuf, MBX->m_ref);
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

#endif debug
#endif	UCB_NET
