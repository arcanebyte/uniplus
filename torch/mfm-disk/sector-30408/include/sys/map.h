/* @(#)map.h	6.3 */
struct map
{
	unsigned int	m_size;
	unsigned int	m_addr;
};

extern struct map coremap[];
extern struct map swapmap[];
extern struct map vmap[];

#define	mapstart(X)	&X[1]
#define	mapwant(X)	X[0].m_addr
#define	mapsize(X)	X[0].m_size
#define	mapdata(X)	{(X)-2, 0} , {0, 0}
#define	mapinit(X, Y)	X[0].m_size = (Y)-2
