/*
 * The node header structure.
 * 
 * To reduce storage consumption, a header block is associated with
 * free blocks only, not allocated blocks.
 * When a free block is allocated, its header block is put on 
 * a free header block list.
 *
 * This creates a header space and a free block space.
 * The left pointer of a header blocks is used to chain free header
 * blocks together.
 */

typedef enum {false,true} bool;
typedef struct	freehdr	*Freehdr;
typedef struct	dblk	*Dblk;

/*
 * Description of a header for a free block
 * Only free blocks have such headers.
 */
struct 	freehdr	{
	Freehdr	left;			/* Left tree pointer */
	Freehdr	right;			/* Right tree pointer */
	Dblk	block;			/* Ptr to the data block */
	u_int	size;			/* Size of the data block */
};

#define NIL		((Freehdr) 0)
#define	NULL		0
#define WORDSIZE	sizeof (int)
#define	SMALLEST_BLK	1	 	/* Size of smallest block */

/*
 * Description of a data block.  
 */
struct	dblk	{
	char	data[1];		/* Addr returned to the caller */
};

/*
 * weight(x) is the size of a block, in bytes; or 0 if and only if x
 *	is a null pointer. It is the responsibility of kmem_alloc() and
 *	kmem_free() to keep zero-length blocks out of the arena.
 */

#define	weight(x)	((x) == NIL? 0: (x->size))
#define	nextblk(p, size) ((Dblk) ((char *) (p) + (size)))
#define	max(a, b)	((a) < (b)? (b): (a))

Freehdr	getfreehdr();
bool	morecore();
caddr_t	kmem_alloc();

/*
 * Structure containing various info about allocated memory.
 */
#define	NEED_TO_FREE_SIZE	10
struct kmem_info {
	Freehdr free_root;
	Freehdr free_hdr_list;
	struct need_to_free {
		caddr_t addr;
		u_int	nbytes;
	} need_to_free_list,need_to_free[NEED_TO_FREE_SIZE];
};
