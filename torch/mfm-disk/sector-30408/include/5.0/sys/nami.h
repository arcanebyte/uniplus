/*	@(#)nami.h UniPlus+ v.0.1.1 */

/*#define NAMISTATS	/* turn on to collect statistics */

#define NAMISET 4	/* associative set size */

struct nami_cache {
	short n_count;				/* set reuse count */
	struct nami_entry {
		ino_t		n_dino;		/* Directory inode num */
		ino_t		n_nino;		/* File inode number */
#ifdef NAMISTATS
		unsigned short	n_nuse;		/* Statistics */
#endif NAMISTATS
		dev_t		n_dev;		/* Directory device */
		char		n_name[DIRSIZ];	/* Directory entry */
		unsigned char   n_lock;		/* Lock bit */
	} n_set[NAMISET];
};

extern struct nami_cache nami_cache[];

/*
 *	This hash function works on
 *		Directory Inode number
 *		Directory Device
 *		Entry name		(taking care so that things like
 *					 vi and c temp files get unique hashes -
 *					 most start with the same 2-4 letter
 *					 string followed by a 5 digit pid)
 */

#define NAMI_HASH(d,i,cp) ((d + i + cp[0] + cp[1] + cp[2] +\
		cp[6] + cp[7] + cp[8]) & (v.v_namisize-1)) 

#ifdef NAMISTATS
extern int nami_hits;
extern int nami_attempt;
extern int nami_total;
#endif NAMISTATS
