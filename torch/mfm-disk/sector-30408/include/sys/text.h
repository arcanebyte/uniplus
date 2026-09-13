/* NFSSRC @(#)text.h	2.1 86/04/11 */
/* @(#)text.h	6.1 */
/*
 * Text structure.
 * One allocated per pure procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
	daddr_t	x_daddr;	/* disk address of segment (relative to swplo) */
	int	x_size;		/* size (clicks) */
	union {
		int	X_caddr;	/* address of text (clicks) */
		short	X_scat;		/* scatter load starting index */
	} X_ad;
#define x_caddr X_ad.X_caddr
#ifndef NONSCATLOAD
#define x_scat X_ad.X_scat
#endif
	struct vnode *x_vptr;	/* vnode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	char	x_flag;		/* traced, written flags */
	union {
		struct {
			int	X_cxaddr;	/* context address (0-63) */
		} X_cxst;
#define x_cxaddr X_un.X_cxst.X_cxaddr
		struct {
			struct dp *X_dp; /* pointer to mmu descriptor pointer */
			char	X_dpcount; /* descriptor reference count */
			char	X_asn;	/* address space number for this text */
		} X_dpst;
#define x_dp X_un.X_dpst.X_dp
#define x_dpcount X_un.X_dpst.X_dpcount
#define x_asn X_un.X_dpst.X_asn
	} X_un;
};

extern struct text text[];

#define	XTRC	01		/* Text may be written, exclusive use */
#define	XWRIT	02		/* Text written into, must swap out */
#define	XLOAD	04		/* Currently being read from file */
#define	XLOCK	010		/* Being swapped in or out */
#define	XWANT	020		/* Wanted for swapping */
#define	XERROR	040		/* Error in reading text image */
#define XDIRTY	080		/* Text has been modified by ptrace */
				/* Cannot be "loitered" by xmsave */

/*
 * Save memory text structure.
 * Holds memory addresses of text regions that
 * are in memory and may be linked to.
 */
struct svtext {
	char	x_svflag;
	int	x_svsize;	/* size of text (clicks) */
	union {
		int	X_svcaddr;	/* address of text (clicks) */
		short	X_svscat;	/* scatter load starting index */
	} SV_ad;
#define x_svcaddr SV_ad.X_svcaddr
#ifndef NONSCATLOAD
#define x_svscat SV_ad.X_svscat
#endif
	dev_t	x_svdev;	/* inode device */
	ino_t	x_svnumber;	/* inode number */
};

extern struct svtext svtext[];

#define XSVBUSY	1		/* Save text structure is busy */
