/*
 * Bad block remapping structure
 */

#define	NICALT		50	/* max alternate disk blocks */
#define ALTMAGIC	0xDBDF	/* bad block information is valid flag */

/*
 * structure for alternate block mapping
 */
struct a_map {
	long a_altbk;			/* bad block */
	long a_index;			/* relative bad block index */
};

/*
 * disk header block format for alternate block mapping
 */
struct altblk {
		/* fill to make structure DEV_BSIZE bytes long */
	char a_fill[DEV_BSIZE-sizeof(struct a_map)-4*sizeof(long)];
	struct a_map a_map[1];		/* mapping */
	long a_magic;			/* verification code (ALTMAGIC) */
	long a_count;			/* bad block count */
	long a_nicbad;			/* max number of bad blocks */
	long a_maxalt;			/* max alt block used so far */
};
