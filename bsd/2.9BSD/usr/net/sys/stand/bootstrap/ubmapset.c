/*
 *	SCCS id	@(#)ubmapset.c	1.2 (Berkeley)	6/2/83
 */
#define UBMAP	((physadr)0170200)
typedef	struct { int r[1]; } *	physadr;
extern char ubmap;
ubmapset()
{
	unsigned register i;

	if(ubmap)
	for(i=0; i<62; i+=2) {
		UBMAP->r[i] = i<<12;
		UBMAP->r[i+1] = i>>4;
	}
}
