/*
 *	SCCS:	%W%	%G%	%U%
 *	Copyright Root Computers Ltd 1982.
 *	ISAM System types etc.
 */

#define INDNAME 'i'
#define DATNAME 'd'
#define AUDNAME 'a'

#define IMAGIC	0150201
#define DMAGIC	0150202
#define AMAGIC	0150203
#define IFMAGIC 0150204
#define DUMAGIC 0150205
#define DFMAGIC 0150206

typedef long	I_NODE;
typedef long	D_RECORD;

#define NODESIZE	512
#define NODESTART	(((sizeof(IFHDR)+NODESIZE-1)/NODESIZE*NODESIZE))

/*
 *	B-Tree node layout, consisting of header followed by either root
 *	branch or leaf data.  No subnode pointers for leaves.
 */

typedef struct	{
	short	Nd_level;		/*  Level in B-Tree  */
	short	Nd_tagno;		/*  Number of entries  */
	STAMP	Nd_stamp;		/*  Update sequence  */
	I_NODE	Nd_father;		/*  Father node	 */
}  NODEHDR;

/*
 *	Leaf layout ... no subnode pointers.
 */

typedef struct	{
	char		Lf_key[IKLCHARS];
	D_RECORD	Lf_tag;
}  LEAFTAG;

/*
 *	Nonleaf layout ... pointers to son for keys > tag.
 */

typedef struct	{
	char		Tr_key[IKTCHARS];
	D_RECORD	Tr_tag;
	I_NODE		Tr_son;
}  TREETAG;

typedef union	{
	LEAFTAG		Ta_leaf;
	TREETAG		Ta_tree;
}  TAG;

#ifdef	DEBUG

#define LEAFPNODE	7
#define TREEPNODE	6
#define MINLTAGS	2
#define MINTTAGS	2

#else

#define LEAFPNODE ((NODESIZE - sizeof(NODEHDR))/sizeof(LEAFTAG))
#define TREEPNODE ((NODESIZE - sizeof(NODEHDR) - sizeof(I_NODE))/sizeof(TREETAG))
#define MINLTAGS  (LEAFPNODE/3)
#define MINTTAGS  (TREEPNODE/3)

#endif

/*
 *	Data area layout for nodes.
 */

typedef union	{

	LEAFTAG Tg_leaf[LEAFPNODE];		/*  For when leaf  */

	struct	{				/*  For when branch  */
		I_NODE	Tg_lowkey;
	       TREETAG	Tg_tree[TREEPNODE];
	}  Tg_treelist;

	char	Tg_cvec[NODESIZE-sizeof(NODEHDR)];

}  TAGLIST;

typedef struct	{
	NODEHDR Nd_hdr;
	TAGLIST Nd_tags;
}  NODE;

typedef NODE	*NODEPTR;

#define NUMTAGS(NP)	(NP)->Nd_hdr.Nd_tagno
#define LOWTREE(NP)	(NP)->Nd_tags.Tg_treelist.Tg_lowkey
#define TTAG(NP,I)	(NP)->Nd_tags.Tg_treelist.Tg_tree[I]
#define LTAG(NP,I)	(NP)->Nd_tags.Tg_leaf[I]

/*
 *	Data record layout.
 */

typedef struct	{
	STAMP		 Rh_stamp;	/*  This bit gets locked  */
	unsigned short		 Rh_magic;
	unsigned  short	 Rh_leng;
}  RECHDR;

#define LENRLOCK	sizeof(STAMP)

typedef struct	{
	RECHDR	Rc_hdr;
	char	Rc_data[1];
}  RECORD;

typedef RECORD	*RECPTR;

typedef union	{
	RECPTR	Rb_rec;
	char	*Rb_charp;
}  RECBUF;

typedef struct	{
	unsigned short	Dh_magic;		/*  Magic number for ISAM data	*/
	char	Dh_versn[100];		/*  Version no - reserved area	*/
}  DFHDR;

typedef struct	{
	long	Ih_time;		/*  Time/date file made	 */
	unsigned short	Ih_magic;		/*  Magic number for ISAM index	 */
	STAMP	Ih_stamp;		/*  Changes sequence  */
	short	Ih_numkeys;		/*  Number of keys  */
	char	Ih_knam[MAXKEYS][MAXKNAME];	/*  Key names  */
	iskd	Ih_key[MAXKEYS];	/*  Key information  */
	I_NODE	Ih_root[MAXKEYS];	/*  Root node for each B-Tree  */
	I_NODE	Ih_free;		/*  Free list  */
	long	Ih_numrecs;		/*  Number of records in file  */
	unsigned short	Ih_reclen;	/*  Maximum record length  */
	unsigned short	Ih_minlen;	/*  Minimum record length  */
	unsigned short	Ih_maxkoff;	/*  Maximum offset in any key  */
	short	Ih_isaudit;		/*  Currently auditing file  */
	short	Ih_majkey;		/*  Major key  */
}  IFHDR;

/*
 *	Argument block, consisting of a vector of vectors
 */

typedef union  {
	char	*arg_char;
	int	*arg_int;
	unsigned	*arg_uns;
	long	*arg_long;
	float	*arg_flt;
	double	*arg_doub;
}  POSSARG;

typedef union	{
	POSSARG		*karg;
	RECBUF		krec;
}  KEY;

typedef KEY	*KEYPTR;

/*
 *	File position structure.
 *	This is not a typedef as we dont want to have to define all the
 *	types in isam.h
 */

struct	iI_posn {
	STAMP		Fp_stamp;	/*  Changes sequence  */
	D_RECORD	Fp_recnum;	/*  Record number  */
	I_NODE		Fp_inode;	/*  Index file node  */
	short		Fp_index;	/*  Index pointer  */
	enum  { Fp_free, Fp_read, Fp_hold }  Fp_lock;	/*  Lock status	 */
};

extern	int	errno;

/*
 *	Record locking stuff.  At present allow ROOTLOCK and BASSLOCK.
 *	Try not to refer to actual error numbers.
 */

#ifdef	ROOTLOCK
#define FLOCK(a,b,c)	flock(a,b,c)
#define LK_OFF	0
#define LK_READ 3
#define LK_WRITE 1
#define LK_CREAD 4
#define LK_CWRITE 2
#endif

#ifdef	BASSLOCK
#define FLOCK(a,b,c)	locking(a,b,(long)(c))
#define LK_OFF	0
#define LK_READ 1
#define LK_WRITE 1
#define LK_CREAD 2
#define LK_CWRITE 2
#endif

#ifdef	XENIXLOCK
#define FLOCK(a,b,c)	locking(a,b,(long)(c))
#define LK_OFF	0
#define LK_READ 3
#define LK_WRITE 1
#define LK_CREAD 4
#define LK_CWRITE 2
#endif
#ifdef NOLOCK
#define FLOCK(a,b,c)	0
#define LK_OFF 	0
#define LK_READ 3
#define LK_WRITE 1
#define LK_CREAD 4
#define LK_CWRITE 2
#endif
