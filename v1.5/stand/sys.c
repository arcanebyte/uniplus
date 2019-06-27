/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*  These routines expect getchar and putchar to be defined elsewhere
 */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/sysmacros.h>
#include	<sys/ino.h>
#include	<sys/inode.h>
#include	<sys/dir.h>
#include	<sys/filsys.h>
#include	<saio.h>

devread(io)
register struct iob *io;
{
	register n;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering devread()\n");
	if (debug & D_RW)
		printf("dev %d/%d Read block %d. %d bytes into 0x%x\n",
			io->i_device, io->i_unit, io->i_bn, io->i_cc, io->i_ma);
# endif
	n = (*devsw[io->i_device].dv_strategy)(io,READ);
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving devread()\n");
# endif
	return(n);
}

devwrite(io)
register struct iob *io;
{
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering devwrite()\n");
# endif
	return( (*devsw[io->i_device].dv_strategy)(io, WRITE) );
}

devopen(io)
register struct iob *io;
{
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering devopen()\n");
# endif
	return((*devsw[io->i_device].dv_open)(io));
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving devopen()\n");
# endif
}

devclose(io)
register struct iob *io;
{
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering devclose()\n");
# endif
	(*devsw[io->i_device].dv_close)(io);
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving devclose()\n");
# endif
}

ino_t dlook();

int
openi(n,io)
register struct iob *io;
{
	register struct dinode *dp;
	register dev = io->i_Fsdev;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering openi, inode=%d\n", n);
# endif
	io->i_offset = 0;
	io->i_bn = io->i_boff;
	io->i_bn += (daddr_t)FsLTOP(dev, (FsITOD(dev, n)));
	io->i_cc = FsBSIZE(dev);
	io->i_ma = io->i_buf;
	if (devread(io) != io->i_cc)
		return(-1);
# ifdef DEBUG
	if (debug & D_RW)
		printf("Read block %d: %d inodes\n", io->i_bn, FsINOPB(dev));
# endif

	dp = (struct dinode *)io->i_buf;
	dp += FsITOO(dev, n);
	io->i_ino.i_number = n;
	io->i_ino.i_mode = dp->di_mode;
	io->i_ino.i_size = dp->di_size;
	l3tol((char *)io->i_ino.i_addr, (char *)dp->di_addr, NADDR);
# ifdef DEBUG
	if (debug & D_RW)
		printf("inode %d, mode 0%o, size %d\n", io->i_ino.i_number,
		io->i_ino.i_mode, io->i_ino.i_size);
	if (debug & D_INOUT)
		printf("Leaving openi\n");
# endif
	return(0);
}


static
find(path, file)
register char *path;
register struct iob *file;
{
	register char *q;
	register char c;
	register n;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering find: path=%s\n", path);
# endif
	if (path==NULL || *path=='\0') {
		printf("find: null path\n");
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving find\n");
# endif
		return(0);
	}

	if (openi(2, file) < 0) {
		printf("find: can't read inode 2\n");
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving find\n");
# endif
		return(0);
	}
	while (*path) {
		while (*path == '/')
			path++;
		q = path;
		while(*q != '/' && *q != '\0')
			q++;
		c = *q;
		*q = '\0';

		if ((n=dlook(path, file))!=0) {
			if (c=='\0')
				break;
			if (openi(n, file) < 0) {
				printf("find: can't read inode %d\n", n);
# ifdef DEBUG
				if (debug & D_INOUT)
					printf("Leaving find\n");
# endif
				return(0);
			}
			*q = c;
			path = q;
			continue;
		} else {
			printf("%s not found\n",path);
# ifdef DEBUG
			if (debug & D_INOUT)
				printf("Leaving find\n");
# endif
			return(0);
		}
	}
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving find: n = %d\n", n);
# endif
	return(n);
}


static daddr_t
sbmap(io, bn)
register struct iob *io;
daddr_t bn;
{
	register i, j;
	register struct inode *ip = &io->i_ino;
	register dev = io->i_Fsdev;
	int sh;
	daddr_t nb, *bap;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering sbmap\n");
# endif
	if(bn < 0) {
		printf("bn negative\n");
		return((daddr_t)0);
	}

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if(bn < NADDR-3) {
		i = bn;
		nb = ip->i_addr[i];
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving sbmap\n");
# endif
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += FsNSHIFT(dev);
		nb <<= FsNSHIFT(dev);
		if(bn < nb)
			break;
		bn -= nb;
	}
	if(j == 0) {
		printf("bn ovf %D\n",bn);
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving sbmap\n");
# endif
		return((daddr_t)0);
	}

	/*
	 * fetch the address from the inode
	 */
	nb = ip->i_addr[NADDR-j];
	if(nb == 0) {
		printf("bn void %D\n",bn);
		return((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		if (blknos[j] != nb) {
			io->i_bn = FsLTOP(dev, nb) + io->i_boff;
			io->i_ma = b[j];
			io->i_cc = FsBSIZE(dev);
			if (devread(io) < io->i_cc) {
				printf("sbmap: can't read block %d\n", nb);
				return((daddr_t)0);
			}
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh -= FsNSHIFT(dev);
		i = (bn>>sh) & FsNMASK(dev);
		nb = bap[i];
		if(nb == 0) {
			printf("bn void %D\n",bn);
			return((daddr_t)0);
		}
	}
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving sbmap\n");
# endif
	return(nb);
}

static ino_t
dlook(s, io)
char *s;
register struct iob *io;
{
	register struct direct *dp;
	register struct inode *ip = &io->i_ino;
	register dev = io->i_Fsdev;
	register daddr_t bn;
	register n, dc;
	int dpb;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering dlook\n");
# endif
	if (s==NULL || *s=='\0')
		return(0);
	if ((ip->i_mode&IFMT)!=IFDIR) {
		printf("not a directory\n");
		printf("mode %o, loc 0%o\n",ip->i_mode,ip);
		return(0);
	}

	n = ip->i_size/sizeof(struct direct);
	if (n==0) {
		printf("zero length directory\n");
		return(0);
	}

	dpb = FsBSIZE(dev)/sizeof(struct direct);
	dc = dpb;
	bn = (daddr_t)0;
	while(n--) {
		if (++dc >= dpb) {
			io->i_bn = FsLTOP(dev, sbmap(io, bn++)) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = FsBSIZE(dev);
			if (devread(io) < io->i_cc) {
				printf("dlook: can't read block %d\n", bn-1);
				return(0);
			}
			dp = (struct direct *)io->i_buf;
			dc = 0;
		}

		if (match(s, dp->d_name)) {
# ifdef DEBUG
			if (debug & D_INOUT)
				printf("Leaving dlook: ino = %d\n", dp->d_ino);
# endif
			return(dp->d_ino);
		}
		dp++;
	}
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving dlook\n");
# endif
	return(0);
}

static
match(s1,s2)
register char *s1,*s2;
{
	register cc;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Match: '%s' '%s'\n", s1, s2);
# endif
	cc = DIRSIZ;
	while (cc--) {
		if (*s1 != *s2)
			return(0);
		if (*s1++ && *s2++)
			continue;
		else
			return(1);
	}
	return(1);
}

lseek(fdesc, addr, ptr)
int	fdesc;
off_t	addr;
int	ptr;
{
	register struct iob *io;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering lseek\n");
# endif
	if (ptr != 0) {
		printf("Seek not from beginning of file\n");
		return(-1);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
# ifdef DEBUG
		if (debug & D_INOUT) {
			printf("Leaving lseek: -1\n");
			printf("\tfdesc=%d  io->i_flgs=0x%x\n", fdesc, io->i_flgs);
		}
# endif
		return(-1);
	}
	io->i_offset = addr;
	io->i_bn = addr/PBSIZE + io->i_boff;
	io->i_cc = 0;
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving lseek: 0\n");
# endif
	return(0);
}

/* if tape mark encountered, set flag  (in driver) */
int tapemark;

getc(fdesc)
int	fdesc;
{
	register struct iob *io;
	register char *p;
	register dev;
	register  c;
	register off;
	register daddr_t bn;

# ifdef DEBUG_GC
	printf("Entering getc\n");
# endif
	if (fdesc >= 0 && fdesc <= 2) {
		c = getchar();
# ifdef DEBUG_GC
		printf("Leaving getc: c = 0%o\n", c & 0x7f);
# endif
		return(c);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
# ifdef DEBUG_GC
		printf("Leaving getc\n");
# endif
		return(-1);
	}
	p = io->i_ma;
	if (io->i_cc <= 0) {
		dev = io->i_Fsdev;
		bn = io->i_offset/(off_t)FsBSIZE(dev);
		if (io->i_flgs&F_FILE)
			io->i_bn = FsLTOP(dev, sbmap(io, bn)) + io->i_boff;
		else
			io->i_bn = FsLTOP(dev, bn) + io->i_boff;
		io->i_ma = io->i_buf;
		io->i_cc = FsBSIZE(dev);
		tapemark=0;
		if (devread(io) < io->i_cc) {
			io->i_cc = 0;
			if (!tapemark)
				printf("getc: can't read block %d\n", bn);
			return(-1);
		}
		if (io->i_flgs&F_FILE) {
			off = io->i_offset % (off_t)FsBSIZE(dev);
			if (io->i_offset+(FsBSIZE(dev)-off) >= io->i_ino.i_size)
				io->i_cc = io->i_ino.i_size - io->i_offset + off;
			io->i_cc -= off;
			if (io->i_cc <= 0) {
# ifdef DEBUG_GC
				printf("Leaving getc:io->i_cc <= 0\n");
# endif
				return(-1);
			}
		} else {
			off = 0;
			if(tapemark) {
# ifdef DEBUG_GC
				printf("Leaving getc:tapemark\n");
# endif
				io->i_cc = 0;
				return(-1);
			}
		}
		p = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = (unsigned)*p++ & 0xFF;
	io->i_ma = p;
# ifdef DEBUG_GC
	printf("Leaving getc at end: c = 0%o\n", c & 0x7f);
# endif
	return(c);
}

getw(fdesc)
int	fdesc;
{
	register char *cp;
	register w, i;
	int val;

	for (i = 0, val = 0, cp = (char *)&val; i < sizeof(val); i++) {
		w = getc(fdesc);
		if (w < 0) {
			if (i == 0)
				return(-1);
			else
				break;
		}
		*cp++ = w;
	}
	return(val);
}

read(fdesc, buf, count)
int	fdesc;
char	*buf;
int	count;
{
	register i;
	register struct iob *file;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering read\n");
# endif
	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		do {
			*buf = getchar();
		} while (--i && *buf++ != '\n');
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving read:1\n");
# endif
		return(count - i);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving read:2\n");
# endif
		return(-1);
	}
	if ((file->i_flgs&F_READ) == 0) {
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving read:3\n");
# endif
		return(-1);
	}
	if ((file->i_flgs&F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		if ((i = devread(file)) < 0) {
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving read: devread failed\n");
# endif
			return(i);
		}
		file->i_bn += (i+PBSIZE-1) >> PBSHIFT;
		file->i_offset += i;
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving read:4\n");
# endif
		return(i);
	}
	if (file->i_offset+count > file->i_ino.i_size)
		count = file->i_ino.i_size - file->i_offset;
	if ((i = count) <= 0) {
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving read:5\n");
# endif
		return(0);
	}
	do {
		*buf++ = getc(fdesc+3);
	} while (--i);
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving read:6\n");
# endif
	return(count);
}

write(fdesc, buf, count)
int	fdesc;
char	*buf;
int	count;
{
	register i;
	register struct iob *file;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering write\n");
# endif
	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		while (i--)
			putchar(*buf++);
		return(count);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_WRITE) == 0)
		return(-1);
	file->i_cc = count;
	file->i_ma = buf;
	if ((i = devwrite(file)) < 0)
		return(i);
	file->i_bn += (i+PBSIZE-1) >> PBSHIFT;
	file->i_offset += i;
	return(i);
}

finit()
{
	register int i;
	for (i = 0; i < NFILES; i++)
		iob[i].i_flgs = 0;
}

readsb(file)
register struct iob *file;
{
	register struct filsys *fp;

	file->i_offset = 0;
	file->i_bn = SUPERB + file->i_boff;
	file->i_cc = PBSIZE;
	file->i_ma = file->i_buf;
	if (devread(file) < PBSIZE) {
		printf("readsb: can't read superblock\n");
		return(-1);
	}

	fp = (struct filsys *)file->i_buf;
	if ((fp->s_magic == FsMAGIC) && (fp->s_type == Fs2b))
		file->i_Fsdev = Fs2BLK;
	else
		file->i_Fsdev = 0;
#ifdef DEBUG
	if ((fp->s_magic == FsMAGIC) && (fp->s_type == Fs2b))
		printf("readsb:  1K-byte block filesystem\n");
	else
		printf("readsb:  512-byte block filesystem\n");
#endif DEBUG
	return(0);
}

open(str, how)
char *str;
int	how;
{
	register char *cp;
	int i;
	register struct iob *file;
	register struct devsw *dp;
	int	fdesc;
	static first = 1;
	long	atol();

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering open\n");
# endif
	if (first) {
		finit();
		first = 0;
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (iob[fdesc].i_flgs == 0)
			goto gotfile;
	_stop("No more file slots");
gotfile:
	(file = &iob[fdesc])->i_flgs |= F_ALLOC;

	for (cp = str; *cp && *cp != '('; cp++)
			;
	if (*cp != '(') {
		printf("Bad device.\n");
		file->i_flgs = 0;
		goto out;
	}
	*cp++ = '\0';
	for (dp = devsw; dp->dv_name; dp++) {
		if (match(str, dp->dv_name))
			goto gotdev;
	}
	printf("Unknown device.\n");
out:
	printf("Valid devices are\n");
	for (dp = devsw; dp->dv_name; dp++)
		printf("  %s\n", dp->dv_name);
	file->i_flgs = 0;
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving open\n");
# endif
	return(-1);
gotdev:
	*(cp-1) = '(';
	file->i_device = dp-devsw;
	file->i_unit = atol(cp);
	for (;;) {
		if (*cp == ',')
			break;
		if (*cp++)
			continue;
	}
	if (file->i_unit < 0 || file->i_unit > 255) {
		printf("Bad unit specifier\n");
		file->i_flgs = 0;
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving open\n");
# endif
		return(-1);
	}
	if (*cp++ != ',') {
badoff:
		printf("Missing offset specification\n");
		file->i_flgs = 0;
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving open\n");
# endif
		return(-1);
	}
	file->i_boff = atol(cp);
	for (;;) {
		if (*cp == ')')
			break;
		if (*cp++)
			continue;
		goto badoff;
	}
	if(devopen(file) < 0)
		return (-1);
	if (*++cp == '\0') {
		file->i_flgs |= how+1;
		file->i_Fsdev = 0;	/* treat raw disk as 512-byte blk fs */
		file->i_offset = 0;
		file->i_bn = file->i_boff;
		file->i_cc = 0;
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving open:1\n");
# endif
		return(fdesc+3);
	}
	if (readsb(file) < 0) {
		file->i_flgs = 0;
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving open:2\n");
# endif
		return(-1);
	}
	if ((i = find(cp, file)) == 0) {
		file->i_flgs = 0;
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving open:3\n");
# endif
		return(-1);
	}
	if (how != 0) {
		printf("Can't write files yet.. Sorry\n");
		file->i_flgs = 0;
		return(-1);
	}
	if (openi(i, file) < 0) {
		printf("open: can't read inode %d\n", i);
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving open:4\n");
# endif
		return(-1);
	}
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | (how+1);
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving open:5\n");
# endif
	return(fdesc+3);
}

close(fdesc)
int	fdesc;
{
	struct iob *file;

# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering close\n");
# endif
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
# ifdef DEBUG
		if (debug & D_INOUT)
			printf("Leaving close:1\n");
# endif
		return(-1);
	}
	if ((file->i_flgs&F_FILE) == 0)
		devclose(file);
	file->i_flgs = 0;
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Leaving close:2\n");
# endif
	return(0);
}

exit()
{
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering exit\n");
# endif
	_stop("Exit called");
}

_stop(s)
char	*s;
{
# ifdef DEBUG
	if (debug & D_INOUT)
		printf("Entering stop\n");
# endif
	printf("%s\n", s);
	for (;;);
}
