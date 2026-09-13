/*	@(#)file.h 1.1 85/05/30 SMI; from UCB 6.2 83/09/23	*/

/*
 * Descriptor table entry.
 * One for each kernel object.
 */
struct	file {
	int	f_flag;		/* see below */
	short	f_type;		/* descriptor type */
	short	f_count;	/* reference count */
	short	f_msgcount;	/* references from message queue */
	struct	fileops {
		int	(*fo_rw)();
		int	(*fo_ioctl)();
		int	(*fo_select)();
		int	(*fo_close)();
		int	(*fo_stat)();
	} *f_ops;
	caddr_t	f_data;		/* ptr to file specific struct (vnode/socket) */
	off_t	f_offset;
	struct	ucred *f_cred;	/* credentials of user who opened file */
};

#ifdef KERNEL
#ifndef AUTOCONFIG
extern struct file file[];	/* The file table itself */
#else
extern struct file *file;	/* The file table itself */
#endif AUTOCONFIG

struct	file *getf();
struct	file *falloc();
#endif

/*
 * flags- also for fcntl call.
 */
#define	FOPEN		(-1)
#define	FREAD		00001		/* descriptor read/receive'able */
#define	FWRITE		00002		/* descriptor write/send'able */
#define	FNDELAY		00004		/* no delay */
#define	FAPPEND		00010		/* append on each write */

#define	FSHLOCK		00020		/* shared lock present */
#define	FEXLOCK		00040		/* exclusive lock present */
#define	FASYNC		00100		/* signal pgrp when data ready */

/* open only modes */
#define	FCREAT		00400		/* create if nonexistent */
#define	FTRUNC		01000		/* truncate to zero length */
#define	FEXCL		02000		/* error if already created */

#define	FKERNEL		04000		/* kernel is doing open */

#ifdef	TORCH
#define FDOPEN		0040000
#define FUOPEN		0100000
#endif

/* bits to save after open */
#define FMASK		(FREAD | FWRITE | FNDELAY | FAPPEND | FASYNC)

#define	FCNTLCANT	(FREAD | FWRITE | FSHLOCK | FEXLOCK | FCREAT | FTRUNC | FEXCL | FKERNEL)

#ifndef	F_DUPFD
/* fcntl(2) requests--from <fcntl.h> */
#define	F_DUPFD		0	/* Duplicate fildes */
#define	F_GETFD		1	/* Get fildes flags */
#define	F_SETFD		2	/* Set fildes flags */
#define	F_GETFL		3	/* Get file flags */
#define	F_SETFL		4	/* Set file flags */
#define	F_GETLK		5	/* Get file lock */
#define	F_SETLK		6	/* Set file lock */
#define	F_SETLKW	7	/* Set file lock and wait */
#define	F_GETOWN	8	/* Get owner */
#define	F_SETOWN	9	/* Set owner */
#endif

/*
 * User definitions.
 */

/*
 * Open call.
 */
#define	O_RDONLY	000		/* open for reading */
#define	O_WRONLY	001		/* open for writing */
#define	O_RDWR		002		/* open for read & write */
#define	O_NDELAY	FNDELAY		/* non-blocking open */
#define	O_APPEND	FAPPEND		/* append on each write */
#define	O_CREAT		FCREAT		/* open with file create */
#define	O_TRUNC		FTRUNC		/* open with truncation */
#define	O_EXCL		FEXCL		/* error on create if file exists */

/*
 * Flock call.
 */
#define	LOCK_SH		1	/* shared lock */
#define	LOCK_EX		2	/* exclusive lock */
#define	LOCK_NB		4	/* don't block when locking */
#define	LOCK_UN		8	/* unlock */

/*
 * Access call.
 */
#define	F_OK		0	/* does file exist */
#define	X_OK		1	/* is it executable by caller */
#define	W_OK		2	/* writable by caller */
#define	R_OK		4	/* readable by caller */

/*
 * Lseek call.
 */
#define	L_SET		0	/* absolute offset */
#define	L_INCR		1	/* relative to current offset */
#define	L_XTND		2	/* relative to end of file */

#ifdef KERNEL
#define	GETF(fp, fd) { \
	if ((unsigned)(fd) >= NOFILE || ((fp) = u.u_ofile[fd]) == NULL) { \
		u.u_error = EBADF; \
		return; \
	} \
}

#define	DTYPE_VNODE	1	/* file */
#define	DTYPE_SOCKET	2	/* communications endpoint */
#endif
