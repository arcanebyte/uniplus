/* @(#)malloc.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/var.h"
#include "sys/scat.h"

/*
 * Allocate 'size' units from the given map.
 * Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 * The swap map unit is 512 bytes.
 * Algorithm is first-fit.
 */
malloc(mp, size)
register struct map *mp;
{
	int n;

	if (mp != coremap)
		return(domall(mp, size));
	do
		if ((n = domall(mp, size)) != 0)
			return(n);
	while (xmrelse() != 0);
	return(0);
}
domall(mp, size)
struct map *mp;
register size;
{
	register struct map *bp;
	register a;

	for (bp = mapstart(mp); bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
				mapsize(mp)++;
			}
			return(a);
		}
	}
	return(0);
}

/*
 * Free the previously allocated space aa
 * of size units into the specified map.
 * Sort aa into map and combine on
 * one or both ends if possible.
 */
mfree(mp, size, a)
struct map *mp;
register a;
{
	register struct map *bp;
	register t;

	bp = mapstart(mp);
	for (; bp->m_addr<=a && bp->m_size!=0; bp++);
	if (bp>mapstart(mp) && (bp-1)->m_addr+(bp-1)->m_size == a) {
		(bp-1)->m_size += size;
		if (a+size == bp->m_addr) {
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
			mapsize(mp)++;
		}
	} else {
		if (a+size == bp->m_addr && bp->m_size) {
			bp->m_addr -= size;
			bp->m_size += size;
		} else if (size) {
			if (mapsize(mp) == 0) {
				printf("\nDANGER: mfree map overflow at map 0x%x\n", mp);
				printf("  lost %d items starting at 0x%x\n", size, a);
				return;
			}
			do {
				t = bp->m_addr;
				bp->m_addr = a;
				a = t;
				t = bp->m_size;
				bp->m_size = size;
				bp++;
			} while (size = t);
			mapsize(mp)--;
		}
	}
#ifdef NONSCATLOAD
	/*
	 * Wake scheduler when freeing core
	 */
	if (mp==coremap) {
		if(runin) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
	} else
#else
	if (mp != coremap)
#endif
	/*
	 * wakeup anyone waiting for a map
	 */
	if (mapwant(mp)) {
		mapwant(mp) = 0;
		wakeup((caddr_t)mp);
	}
}

/*
 * return the largest size in the given map structure
 */
mallocl(mp)
struct map *mp;
{
	register struct map *bp;
	register a;

	a = 0;
	for (bp=mapstart(mp); bp->m_size; bp++)
		if (bp->m_size > a)
			a = bp->m_size;
	return(a);
}

#ifdef NONSCATLOAD
/*
 * malloc size clicks starting at address 'start'
 */
mallocat(mp, size, start)
struct map *mp;
register start;
{
	register struct map *bp;
	register a;

	for (bp=mapstart(mp); bp->m_size; bp++) {
		if (bp->m_addr == start && bp->m_size >= size) {
			a = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
				mapsize(mp)++;
			}
			return(a);
		}
	}
	return(0);
}
#else

extern int nscatfree;

/*
 * allocate size units from the memory map
 */
memalloc(size)
{
	int n;

	while ((n = domemalloc(size)) == NULL)
		if (xmrelse() == 0)
			break;
	return(n);
}

domemalloc(size)
register size;
{
	register struct scatter *s;
	register short i;
	register a1, a2;

	if (size <= 0) {
		printf("memalloc error: tried to allocate %d units\n", size);
		return(NULL);
	}
	if (size > nscatfree) {
		/* printf("memalloc: less than %d free pages at present\n",
			size); */
		return(NULL);
	}
	s = scatmap;
	a1 = a2 = scatfreelist.sc_index;
	if (size > 1) {
		i = size - 2;
		do {
			a1 = s[a1].sc_index;
		} while (--i != -1);
	}
	scatfreelist.sc_index = s[a1].sc_index;
	s[a1].sc_index = SCATEND;
	nscatfree -= size;
	/* printf("memalloc: found %d free pages starting at index %d\n",
		size, a2); */
	return(a2);
}

/*
 * allocate size contiguous units from the memory map
 */
cmemalloc(size)
{
	int n;

	do
		scatsort();
	while ((n = docmemalloc(size))==NULL && xmrelse());
/* printf("cmemalloc:size=%d starting address=0x%x\n", size, n); */
	return(n);
}
docmemalloc(size)
register size;
{
	register struct scatter *s, *ss;
	register short i;
	register a1, a2, n;

	if (size <= 0) {
		printf("cmemalloc error: tried to allocate %d units\n", size);
		return(NULL);
	}
	if (size > nscatfree)
		return(NULL);
	s = scatmap;
	ss = &scatfreelist;
	a1 = ss->sc_index;
	for (;;) {
		n = memcontig(a1, size);
		if (n >= size)
			break;
		if (n > 1) {
			i = n - 2;
			do
				a1 = s[a1].sc_index;
			while (--i != -1);
		}
		ss = &s[a1];
		if (ss->sc_index == SCATEND)
			return(NULL);
		a1 = ss->sc_index;
	}
	a2 = a1;
	if (size > 1) {
		i = size - 2;
		do
			a1 = s[a1].sc_index;
		while (--i != -1);
	}
	ss->sc_index = s[a1].sc_index;
	s[a1].sc_index = SCATEND;
	nscatfree -= size;
	/* printf("cmemalloc: found %d free pages starting at index %d\n",
		size, a2); */
	if (countscat(a2) != size)
		printf("cmemalloc:improper allocation countscat=%d size=%d\n",
			countscat(a2), size);
	return(a2);
}

/*
 * free memory map chain
 */
memfree(a)
register a;
{
	register struct scatter *s;
	register i, a1, a2;

	if (a <= 0 || a >= v.v_nscatload) {
		printf("memfree:illegal index %d (0x%x)\n", a, a);
		return;
	}
	i = 1;
	s = scatmap;
	a1 = a;
	while ((a2 = s[a1].sc_index) != SCATEND) {
		a1 = a2;
		i++;
	}
	/* printf("memfree:%d units freed starting at %d\n", i, a); */
	s[a1].sc_index = scatfreelist.sc_index;
	scatfreelist.sc_index = a;
	nscatfree += i;
	/*
	 * Wake scheduler when freeing memory
	 */
	if (runin) {
		runin = 0;
		wakeup((caddr_t)&runin);
	}
}

/*
 * find number of contiguous memory pages
 */
memcontig(sindex, ct)
register sindex, ct;
{
	register struct scatter *s;
	register a1, n;

	if (sindex == SCATEND || ct <= 0) {
		printf("memcontig:sindex=0x%x ct=%d\n", sindex, ct);
		return(0);
	}
	n = 1;
	s = scatmap;
	a1 = ixtoc(sindex);
	while (--ct > 0) {
		if ((sindex = s[sindex].sc_index) == SCATEND)
			break;
		if (++a1 != ixtoc(sindex))
			break;
		n++;
	}
	return(n);
}

/*
 * sort the scatter load map
 */
scatsort()
{
	register struct scatter *s, *sf;
	register int j, k, n, *ip, *jp;
	register short i;

	/*
	 * clear scatter sort array
	 */
	ip = scsortmap;
	i = ((v.v_nscatload+31) >> 5) - 1;
	do
		*ip++ = 0;
	while (--i != -1);
	/*
	 * build bit map of free pages
	 */
	s = scatmap;
	sf = &scatfreelist;
	ip = scsortmap;
	for (j = sf->sc_index; j != SCATEND; j = s[j].sc_index)
		ip[j>>5] |= 1 << (j&31);
	/*
	 * rebuild freelist
	 */
	n = 0;
	sf->sc_index = SCATEND;
	j = ((v.v_nscatload+31) >> 5) - 1;
	jp = &scsortmap[j];
	for ( ; j >= 0; j--, jp--) {
		if (*jp == 0)
			continue;
		for (k=31; k>=0; k--) {
			if (*jp&(1<<k)) {
				n++;
				i = sf->sc_index;
				sf->sc_index = (j << 5) + k;
				s[(j << 5) + k].sc_index = i;
			}
		}
	}
	nscatfree = n;
}
#endif
