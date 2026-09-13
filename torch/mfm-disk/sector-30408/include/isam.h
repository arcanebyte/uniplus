/*
 *	SCCS:	@(#)isam.h	1.7	14/8/84	14:08:54
 *	ISAM User program include header.
 */

#ifdef	V7
#define void
#define NULL	0
#endif	V7

#define MAXRSIZE	12000	/*  Maximum record size	 */
#define MAXLKS		6	/*  Maximum no of records lockable  */
#define MAXKEYS		12	/*  Maximum no of keys	*/
#define MAXKFLDS	6	/*  Maximum no of key fields  */
#define MAXKNAME	8	/*  Key name length  */
#define IKTCHARS	16	/*  Chars of key in index file	*/
#define IKLCHARS	20	/*  Ditto for leaf  */
#define NUMIBUFS	6	/*  Number of index buffers  */

typedef long	STAMP;

/*
 *	Key description.
 */

typedef	 struct	 {
	short  ik_nflds;		/* Number of fields */
	struct	ik_tstrct  {
		unsigned short	ik_boff;       /* Byte offset */
		char		ik_flags;      /* Type etc */
#ifdef	V7
		char		ik_lng;
#else
		unsigned char	ik_lng;	       /* Elements in field */
#endif V7
	}  ik_ftypes[MAXKFLDS];
}  iskd;

/*
 *	"File descriptor" for ISAM routines.
 */

typedef	 struct	 {
	int	if_ifile;	/*  Index file fd  */
	int	if_dfile;	/*  Data file fd  */
	int	if_afile;	/*  Audit file fd  */
	unsigned  short	 if_reclen;	/*  Max record length  */
	short	if_ckeynum;	/*  Key number of current key */
	short	if_mode;	/*  Open mode  */
	char	*if_buffer;		/*  Buffer allocated if any  */
	struct	iI_posn *if_posn;	/*  Indication of current/next record */
	short	if_lrs;		/*  No of records locked  */
	long	if_lrc[MAXLKS];	     /*	 Offsets in file  */
}  isfd;

/*
 *	Information block.
 */

typedef struct {
	long	       is_made;		/* Date time file made */
	STAMP	       is_changes;	/* Changes sequence  */
	unsigned short is_knumb;	/* No of keys */
	unsigned short is_maxrec;	/* Maximum record size */
	long	       is_rnumb;	/* No of records */
	short	       is_audit;	/* Auditing active */
	short	       is_majkey;	/* Major key no */
}  isis;

/* Audit trail file information */

typedef enum	{A_OFF, A_ON, A_KDEL, A_KCR, A_DEL,
		 A_WRITE, A_UPD1, A_UPD2, A_ARCH, A_CMAJ}  AHTYPE;

typedef struct	{
	STAMP	Ah_stamp;		/*  Change sequence  */
	long	Ah_time;		/*  Time done  */
	unsigned short	Ah_magic;		/*  Magic number for ISAM audit	 */
	AHTYPE	Ah_action;		/*  Operation  */
	unsigned  short	 Ah_reclen;	/*  Length of following record	*/
}  AFHDR;

/* Data types */

#define IK_CHAR 0
#define IK_SHORT 1
#define IK_UNSIG 2
#define IK_LONG 3
#define IK_FLOAT 4
#define IK_DOUBLE 5
#define IK_UNIQ 7
#define IK_TYPE 7
#define IK_DESC 8
#define IK_NULLT 16

/*
 *	Open and read flags.
 */

#define IO_RDONLY	0
#define IO_WEXC		1
#define IO_APPEND	2
#define IO_UPDATE	6
#define IO_RDAUDIT	16

#define IR_EQ	0
#define IR_LE	1
#define IR_GE	2
#define IR_LOCK 4
#define IR_LCOND	8

/*
 *	Error numbers.	Note that they are all negative to avoid confusion
 *	with UNIX error numbers.
 */

#define INAME	-1
#define IEXIST	-2
#define IPARAM	-3
#define IPERM	-4
#define INEXIST -5
#define IACCESS -6
#define IFORMAT -7
#define INOTEXC -8
#define IFLDCK	-9
#define INOTKEY -10
#define INOREC	-11
#define ILOCK	-12
#define INLOCK	-13
#define ISELECT -14
#define IDUPKEY -15
#define INSELECT	-16
#define INWRITE -17
#define IMLOCKS -18
#define IMKEYS	-19
#define IMFLDS	-20
#define IRECLEN -21
#define IBUG	-22

isfd	*Iopen();
char	*Irread(), *Irreadv(), *Irreads(), *Ierror();
STAMP	Istamp();
