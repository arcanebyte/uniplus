/*	@(#)ar.h UniPlus+ v.0.1.1 */
#define	ARMAG	0177545
struct	ar_hdr {
	char	ar_name[14];
	long	ar_date;
	short	ar_uid;
	short	ar_gid;
	short	ar_mode;
	long	ar_size;
};
