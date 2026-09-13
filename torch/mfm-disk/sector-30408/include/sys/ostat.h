/* @(#)ostat.h	6.1 */
/*
 * Structure of the result of V.2 (non-NFS) stat
 */

struct	ostat
{
	dev_t	ost_dev;
	ino_t	ost_ino;
	ushort	ost_mode;
	short	ost_nlink;
	short	ost_uid;
	short	ost_gid;
	dev_t	ost_rdev;
	off_t	ost_size;
	time_t	ost_atime;
	time_t	ost_mtime;
	time_t	ost_ctime;
};
