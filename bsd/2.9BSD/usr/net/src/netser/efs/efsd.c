#ifndef lint
static char sccsid[] = "@(#)efsd.c	4.2 10/10/82";
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/in.h>
#include <errno.h>
#include <pwd.h>
#include <wait.h>
#include <signal.h>
#include <sgtty.h>
#include <stdio.h>
#include <sys/efs.h>

#define EXIT	(1)
#define NO_EXIT	(2)

#define dprintf if(debug) printf2("efsd: "); \
		if(debug) printf2
#define efserror(f, exit) {efssenderror(f, exit); break;}

extern	errno;
char	*raddr();
int	options = SO_ACCEPTCONN|SO_KEEPALIVE|SO_DONTLINGER;
struct	sockaddr_in sin = { AF_INET, IPPORT_EFSSERVER };
int	debug = 0;

main(argc, argv)
	int argc;
	char **argv;
{
	union wait status;
	int f;
	struct sockaddr_in from;

#ifndef DEBUG
	if (fork())
		exit(0);
	for (f = 0; f < 10; f++)
		(void) close(f);
	(void) open("/", 0);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
	{ int tt = open("/dev/tty", 2);
	  if (tt > 0) {
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	  }
	}
#endif
#if vax
	sin.sin_port = htons(sin.sin_port);
#endif
	argc--, argv++;
	if (argc > 0 && !strcmp(argv[0], "-d")) {
		options |= SO_DEBUG;
		debug = 1;
	}
	nice(-4);	/* give it some help */
	for (;;) {
		f = socket(SOCK_STREAM, 0, &sin, options);
		if (f < 0) {
			perror("socket");
			sleep(5);
			continue;
		}
		if (accept(f, &from) < 0) {
			perror("accept");
			if(close(f) < 0)
				perror("close");
			sleep(1);
			continue;
		}
		if (fork() == 0)
			doit(f, &from);
		if(close(f) < 0)
			perror("close");
		while (wait3(status, WNOHANG, 0) > 0)
			continue;
	}
}

char	bigbuffer[32*1024];	/* is this big enough? */
extern	errno;
struct	efs_controlblock cb, retcb;
char	*getpath();

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char *rhost;
	short int mode, uid, gid, cmask;
	short int owner, group;
	register int i;
	long int offset, count, size, len;
	char *path;
	int fd;
	struct stat statb;

#if vax
	fromp->sin_port = htons(fromp->sin_port);
#endif
	rhost = raddr(fromp->sin_addr.s_addr);
	if (fromp->sin_family != AF_INET ||
			/* fromp->sin_port >= IPPORT_RESERVED || NOT FOR NOW */
			rhost == 0)
		abort("connection invalid");
	for(;;) {
		if(read(f, &cb, sizeof cb) != sizeof cb)
			abort("efsd: read1");
		dprintf("\ncmd: %d\n", cb.efs_cmd);
		switch(cb.efs_cmd) {
		case EFS_OPEN:
		case EFS_CREAT:
			mode = ntohs(cb.efs_mode);
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			cmask = ntohs(cb.efs_cmask);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			dprintf("O/C mode=%o,uid=%d,gid=%d,cmask=%o,path=%s\n",
				mode, uid, gid, cmask, path);
			umask(cmask);
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(cb.efs_cmd == EFS_OPEN) {
				if((fd = open(path, mode)) < 0)
					efserror(f, EXIT);
			} else if((fd = creat(path, mode)) < 0)
					efserror(f, EXIT);
			size = lseek(fd, 0L, 2);	/* pass size back */
			dprintf("O/C succeeded. size=%d\n", size);
			retcb.efs_size = htonl(size);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			break;
		case EFS_CLOSE:
			if(close(fd) < 0) {
				efserror(f, NO_EXIT);
			} else {
				retcb.efs_cmd = EFS_OK;
				efswrite(f, &retcb, sizeof retcb);
			}
			exit(0);
		case EFS_READ:
			offset = ntohl(cb.efs_offset);
			count = ntohl(cb.efs_count);
			if(count > sizeof bigbuffer)
				abort("EFS_READ: buffer too small");
			if(lseek(fd, offset, 0) < 0) {
dprintf("efs_read, lseek to %d failed\n", offset);
				efserror(f, NO_EXIT);
}
			if((len = read(fd, bigbuffer, count)) < 0) {
dprintf("efs_read, read %d failed\n", count);
				efserror(f, NO_EXIT);
}
			dprintf("read returns %d\n", len);
			retcb.efs_size = htonl(len);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			efswrite(f, bigbuffer, len);
			break;
		case EFS_WRITE:
			offset = ntohl(cb.efs_offset);
			count = ntohl(cb.efs_count);
			if(count > sizeof bigbuffer)
				abort("EFS_WRITE: buffer too small");
			if(lseek(fd, offset, 0) < 0)
				efserror(f, NO_EXIT);
			if(read(f, bigbuffer, count) != count)
				abort("efsd: read2");
			if((len = write(fd, bigbuffer, count)) < 0)
				efserror(f, NO_EXIT);
			retcb.efs_cmd = EFS_OK;
			retcb.efs_size = htonl(len);
			efswrite(f, &retcb, sizeof retcb);
			break;
		case EFS_IOCTL:
			if(ioctl(fd, FIONREAD, &size) < 0)
				efserror(f, NO_EXIT);
			retcb.efs_cmd = EFS_OK;
			retcb.efs_size = htonl(size);
			efswrite(f, &retcb, sizeof retcb);
			break;
		case EFS_FSTAT:
			if(fstat(fd, &statb) < 0)
				efserror(f, NO_EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			/* THIS IS NOT PORTABLE -- SHOULD htonl EACH FIELD */
			efswrite(f, &statb, sizeof statb);
			break;
		case EFS_STAT:
		case EFS_LSTAT:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(cb.efs_cmd == EFS_STAT) {
				if(stat(path, &statb) < 0)
					efserror(f, EXIT);
			} else if(lstat(path, &statb) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			/* THIS IS NOT PORTABLE -- SHOULD htonl EACH FIELD */
			efswrite(f, &statb, sizeof statb);
			exit(0);	/* once only function */
		case EFS_UNLINK:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(unlink(path) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			exit(0);	/* once only function */
		case EFS_READLINK:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if((len = readlink(path, bigbuffer,
							sizeof bigbuffer)) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			retcb.efs_size = htonl(len);
			efswrite(f, &retcb, sizeof retcb);
			efswrite(f, bigbuffer, len);
			exit(0);	/* once only function */
		case EFS_CHMOD:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			mode = ntohs(cb.efs_mode);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(chmod(path, mode) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			exit(0);	/* once only function */
		case EFS_CHOWN:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			owner = ntohs(cb.efs_owner);
			group = ntohs(cb.efs_group);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(chown(path, owner, group) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			exit(0);	/* once only function */
		case EFS_ACCESS:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			mode = ntohs(cb.efs_mode);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(access(path, mode) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			exit(0);	/* once only function */
		case EFS_UTIME:
			uid = ntohs(cb.efs_uid);
			gid = ntohs(cb.efs_gid);
			for(i = 0; i < (sizeof cb.efs_grps)/(sizeof(int)); i++)
				cb.efs_grps[i] = ntohl(cb.efs_grps);
			cb.efs_time[0] = ntohl(cb.efs_time[0]);
			cb.efs_time[1] = ntohl(cb.efs_time[1]);
			path = getpath(f);
			if(path == NULL || strlen(path) <= 0) {
				errno = ENOENT;
				efserror(f, EXIT);
			}
			setgid(gid);
			setgrp(cb.efs_grps, 0);
			setuid(uid);
			if(utime(path, cb.efs_time) < 0)
					efserror(f, EXIT);
			retcb.efs_cmd = EFS_OK;
			efswrite(f, &retcb, sizeof retcb);
			exit(0);	/* once only function */
		}
	}
}

char pathbuf[MAXPATHLEN];

char *getpath(f)
	int f;
{
	register char *cp;
	char c;

	for(cp = pathbuf; cp < &pathbuf[MAXPATHLEN]; cp++) {
		if(read(f, &c, 1) < 0)
			abort("getpath: read1");
		*cp = c;
		if(c == '\0')
			return(pathbuf);
	}
	abort("getpath: file name too long");
}

efswrite(f, addr, size)
	int f;
	char *addr;
	int size;
{
	dprintf("efswrite: size=%d\n", size);
	if(write(f, addr, size) != size)
		abort("efswrite");
	dprintf("\twrite done\n");
}

efssenderror(f, exitflag)
	int f, exitflag;
{

	dprintf("efssenderror: errno=%d\n", errno);
	retcb.efs_cmd = EFS_ERROR;
	retcb.efs_error = errno;
	efswrite(f, &retcb, sizeof retcb);
	if(exitflag == EXIT)
		exit(0);
}

abort(str)
	char *str;
{
	perror(str);
	exit(1);
}

printf2(s, a, b, c, d, e, f, g, h)
	char *s;
{
	fprintf(stderr, s, a, b, c, d, e, f, g, h);
}
