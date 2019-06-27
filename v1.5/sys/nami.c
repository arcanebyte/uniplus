/* #define HOWFAR */

/* @(#)nami.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/var.h"

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 *
 * func = function called to get next char of name
 *	&uchar if name is in user space
 *	&schar if name is in system space
 * flag = 0 if name is sought
 *	1 if name is to be created
 *	2 if name is to be deleted
 */
struct inode *
namei(func, flag)
int (*func)();
{
	register struct user *up;
	register struct inode *dp;
	register c;
	register char *cp;
	register struct buf *bp;
	register i;
	dev_t d;
	off_t eo;

	/*
	 * If name starts with '/' start from
	 * root; otherwise start from current dir.
	 */

	up = &u;
	sysinfo.namei++;
	c = (*func)();
	if (c == '\0') {
		up->u_error = ENOENT;
		return(NULL);
	}
	if (c == '/') {
		if ((dp = up->u_rdir) == NULL)
			dp = rootdir;
		while(c == '/')
			c = (*func)();
		if(c == '\0' && flag != 0) {
			up->u_error = ENOENT;
			return(NULL);
		}
	} else
		dp = up->u_cdir;
	(void) iget(dp->i_dev, dp->i_number);

cloop:
	/*
	 * Here dp contains pointer
	 * to last component matched.
	 */

	if(up->u_error)
		goto out;
	if(c == '\0')
		return(dp);

	/*
	 * If there is another component,
	 * gather up name into users' dir buffer.
	 */


	cp = &up->u_dent.d_name[0];
	while(c != '/' && c != '\0' && up->u_error == 0) {
		if(cp < &up->u_dent.d_name[DIRSIZ])
			*cp++ = c;
		c = (*func)();
	}
	while(cp < &up->u_dent.d_name[DIRSIZ])
		*cp++ = '\0';
#ifdef HOWFAR
	printf("nami:about to scan for '%s'\n", up->u_dent.d_name);
#endif
	while(c == '/')
		c = (*func)();

seloop:
	/*
	 * dp must be a directory and
	 * must have X permission.
	 */
#ifdef HOWFAR
	/* printf("nami:directory mode = 0%o\n", dp->i_mode&0xFFFF); */
#endif
	if ((dp->i_mode&IFMT) != IFDIR || dp->i_nlink==0)
		up->u_error = ENOTDIR;
	else
		(void) access(dp, IEXEC);
	if (up->u_error)
		goto out;

	/*
	 * set up to search a directory
	 */
	up->u_offset = 0;
	up->u_count = dp->i_size;
	up->u_pbsize = 0;
	eo = 0;
	bp = NULL;
	if (dp == up->u_rdir)
	if (up->u_dent.d_name[0] == '.')
	if (up->u_dent.d_name[1] == '.')
	if (up->u_dent.d_name[2] == '\0')
		goto cloop;

eloop:

	/*
	 * If at the end of the directory,
	 * the search failed. Report what
	 * is appropriate as per flag.
	 */

	if (up->u_count == 0) {
		if(bp != NULL)
			brelse(bp);
		if(flag==1 && c=='\0') {
			if(access(dp, IWRITE))
				goto out;
			up->u_pdir = dp;
			if (eo)
				up->u_offset = eo - sizeof(struct direct);
			up->u_count = sizeof(struct direct);
			(void) bmap(dp, B_WRITE);
			if (up->u_error)
				goto out;
			return(NULL);
		}
		up->u_error = ENOENT;
		goto out;
	}

	/*
	 * If done with current block,
	 * read the next directory block.
	 * Release previous if it exists.
	 */

	if (up->u_pbsize == 0) {
		daddr_t bn;

		if(bp != NULL)
			brelse(bp);
		sysinfo.dirblk++;
		bn = bmap(dp, B_READ);
		if (up->u_error)
			goto out;
		if (bn < 0) {
			up->u_error = EIO;
			goto out;
		}
		bp = bread(dp->i_dev, bn);
		if (up->u_error) {
			brelse(bp);
			goto out;
		}
	}

	/*
	 * Note first empty directory slot
	 * in eo for possible creat.
	 * String compare the directory entry
	 * and the current component.
	 * If they do not match, go back to eloop.
	 */

	cp = bp->b_un.b_addr + up->u_pboff;
	up->u_offset += sizeof(struct direct);
	up->u_pboff += sizeof(struct direct);
	up->u_pbsize -= sizeof(struct direct);
	up->u_count -= sizeof(struct direct);
	up->u_dent.d_ino = ((struct direct *)cp)->d_ino;
	if(up->u_dent.d_ino == 0) {
		if(eo == 0)
			eo = up->u_offset;
		goto eloop;
	}
	cp = &((struct direct *)cp)->d_name[0];
	for(i=0; i<DIRSIZ; i++)
		if(*cp++ != up->u_dent.d_name[i])
			goto eloop;

	/*
	 * Here a component matched in a directory.
	 * If there is more pathname, go back to
	 * cloop, otherwise return.
	 */

	if(bp != NULL)
		brelse(bp);
	if(flag==2 && c=='\0') {
		if(access(dp, IWRITE))
			goto out;
		return(dp);
	}
	d = dp->i_dev;
	if(up->u_dent.d_ino == ROOTINO)
	if(dp->i_number == ROOTINO)
	if(up->u_dent.d_name[1] == '.')
		for(i=1; i<v.v_mount; i++)
			if(mount[(short)i].m_flags == MINUSE)
			if(mount[(short)i].m_dev == d) {
				iput(dp);
				dp = mount[(short)i].m_inodp;
				dp->i_count++;
				plock(dp);
				goto seloop;
			}
	iput(dp);
	dp = iget(d, up->u_dent.d_ino);
	if(dp == NULL)
		return(NULL);
	goto cloop;

out:
	iput(dp);
	return(NULL);
}

/*
 * Return the next character from the
 * kernel string pointed at by dirp.
 */
schar()
{

	return(*u.u_dirp++ & 0377);
}

/*
 * Return the next character from the
 * user string pointed at by dirp.
 */
uchar()
{
	register c;

	c = fubyte((caddr_t)u.u_dirp++);
	if(c == -1)
		u.u_error = EFAULT;
	return(c);
}
