/* @(#)dir.h	6.1 */
#define	SVFSDIRSIZ	14
struct	svfsdirect
{
	ino_t	d_ino;
	char	d_name[SVFSDIRSIZ];
};
/* so user programs can just include dir.h */
#if !defined(KERNEL) && !defined(DEV_BSIZE)
#define	DEV_BSIZE	512
#endif
#define	DIRBLKSIZ	DEV_BSIZE
