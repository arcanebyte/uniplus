/*	mbuf.h	4.13	82/06/14	*/

/*
 * billn -- For 68k, no use of "clusters" ala the vax.
 *  TODO: bigger mbufs?
 */

/*
 * Mbuf statistics.
 */
struct mbstat {
	short	m_mbufs;		/* mbufs obtained from page pool */
	short	m_mbfree;		/* mbufs on our free list */
	short	m_clusters;		/* clusters obtained from page pool */
	short	m_clfree;		/* free clusters */
	short	m_drops;		/* times failed to find space */
};

#ifdef	KERNEL
struct	mbstat mbstat;
struct	mbuf *m_get(),*m_getclr(),*m_free(),*m_more(),*m_copy(),*m_pullup();
struct	mbuf *mfreep;
#endif KERNEL


/*
 * MSIZE MUST BE A POWER OF 2 for macros, eg, dtom, to work.
 */
#define MSIZE           256                     /* size of an mbuf */
#define MMINOFF         12                       /* mbuf header length */
#define MTAIL           4
#define	MMAXOFF		(MSIZE-MTAIL)		/* offset where data ends */
#define	MLEN		(MSIZE-MMINOFF-MTAIL)	/* mbuf data length */
/*
#define NMBUFS  60              /* mbuf area = NMBUFS*MSIZE */
#define NMBUFS  100              /* mbuf area = NMBUFS*MSIZE */


/*
 * Macros for type conversion
 */

/* address in mbuf to mbuf head */
#define	dtom(x)		((struct mbuf *)((int)x & ~(MSIZE-1)))

/* mbuf head, to typed data */
#define	mtod(x,t)	((t)((int)(x) + (x)->m_off))


struct mbuf {
	struct	mbuf *m_next;		/* next buffer in chain */
	u_long	m_off;			/* offset of data */
	short	m_len;			/* amount of data in this mbuf */
	short	m_free;			/* is mbuf free? (consistency check) */
	u_char	m_dat[MLEN];		/* data storage */
	struct	mbuf *m_act;		/* link in higher-level mbuf list */
};

/* flags to m_get */
#define	M_DONTWAIT	0
#define	M_WAIT		1

/* length to m_copy to copy all */
/* ??? */
#define	M_COPYALL	1000000000
#ifdef	PRMDEF
#define	PRMFREE	prmfree()
#define	PRMGET	prmget()
#else
#define	PRMFREE
#define	PRMGET
#endif

#define	MGET(m, i) \
	{ int ms = splimp(); \
	  if ((m)=mfreep) \
		{ if ((m)->m_free == 0) panic("mget"); (m)->m_free = 0; \
		  /* printf("mget->%x, mfreep=%x   ",m,mfreep); */ \
		  PRMGET; \
		  mbstat.m_mbfree--; mfreep = (m)->m_next; (m)->m_next = 0; } \
	  else { \
		(m) = 0; \
		printf("MGET returning 0\n"); } \
	  splx(ms); }

#define	MFREE(m, n) \
	{ int ms = splimp(); \
	  /* printf("MFREE:m,m_free,m_next,m_off,mfreep=%x,%x,%x,%x,%x\n", \
	   m,m->m_free,m->m_next,m->m_off,mfreep); */ \
	  if ((m)->m_free) panic("mfreep"); (m)->m_free = 1; \
	  (n) = (m)->m_next; (m)->m_next = mfreep; \
	  (m)->m_off = 0; (m)->m_act = 0; mfreep = (m); mbstat.m_mbfree++; \
	  PRMFREE; \
	  splx(ms); }

#define MSGET(sp,s,clr) \
	{ struct mbuf *mb = m_get(M_DONTWAIT); \
	  if (mb) { \
		mb->m_off = MMINOFF;  sp = mtod(mb, s *); \
		if (clr) bzero((caddr_t)(sp), sizeof(s)); \
		PRMGET; \
	  } else sp = 0; }

#ifdef lint
#define MSFREE(sp) (void)m_free(dtom(sp))
#else
#define MSFREE(sp) {PRMFREE; (void)m_free(dtom(sp));}
#endif

#define MBCOPY(from,fromoff,to,tooff,count) \
	bcopy((caddr_t)(mtod((from),caddr_t)+fromoff),(caddr_t)(mtod((to),caddr_t)+tooff),(int)(count))

#define MAPSAVE()
#define MAPREST()
#define MAPSET(n)
