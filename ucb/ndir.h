/*
 * ndir.h -- 4.2BSD-style directory reading (opendir, readdir, closedir)
 * for UniPlus+ System V, whose directories are arrays of 16-byte entries
 * with 14-character names (<sys/dir.h>).  readdir() returns an entry
 * whose name is always null-terminated.  Include <sys/types.h> first
 * (System V's has no guard against being included twice).
 */
#ifndef	DIRSIZ
#define	DIRSIZ	14
#endif

struct	ndirect {
	ino_t	d_ino;
	char	d_name[DIRSIZ+1];
};
#define	direct	ndirect		/* as BSD programs spell it */

typedef struct {
	int	dd_fd;
	struct	ndirect dd_ent;
} DIR;

DIR	*opendir();
struct	ndirect *readdir();
