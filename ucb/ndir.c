/*
 * ndir.c -- 4.2BSD-style directory reading for UniPlus+ System V.
 * See ndir.h.
 */
#include <sys/types.h>
#include <sys/dir.h>
#include <stdio.h>

struct	ndirect {		/* as in ndir.h, which can't be included here */
	ino_t	d_ino;		/* because it redefines "direct" */
	char	d_name[DIRSIZ+1];
};

typedef struct {
	int	dd_fd;
	struct	ndirect dd_ent;
} DIR;

char	*malloc();

DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	int fd;

	if ((fd = open(name, 0)) < 0)
		return (NULL);
	if ((dirp = (DIR *)malloc(sizeof (DIR))) == NULL) {
		close(fd);
		return (NULL);
	}
	dirp->dd_fd = fd;
	return (dirp);
}

struct ndirect *
readdir(dirp)
	register DIR *dirp;
{
	struct direct raw;

	while (read(dirp->dd_fd, (char *)&raw, sizeof (raw)) == sizeof (raw)) {
		if (raw.d_ino == 0)
			continue;
		dirp->dd_ent.d_ino = raw.d_ino;
		strncpy(dirp->dd_ent.d_name, raw.d_name, DIRSIZ);
		dirp->dd_ent.d_name[DIRSIZ] = '\0';
		return (&dirp->dd_ent);
	}
	return (NULL);
}

closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	free((char *)dirp);
}
