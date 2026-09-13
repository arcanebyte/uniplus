/* @(#)errno.h	6.1 */
/*
 * Error codes
 */

#define	EPERM	1	/* Not super-user			*/
#define	ENOENT	2	/* No such file or directory		*/
#define	ESRCH	3	/* No such process			*/
#define	EINTR	4	/* interrupted system call		*/
#define	EIO	5	/* I/O error				*/
#define	ENXIO	6	/* No such device or address		*/
#define	E2BIG	7	/* Arg list too long			*/
#define	ENOEXEC	8	/* Exec format error			*/
#define	EBADF	9	/* Bad file number			*/
#define	ECHILD	10	/* No children				*/
#define	EAGAIN	11	/* No more processes			*/
#define	ENOMEM	12	/* Not enough core			*/
#define	EACCES	13	/* Permission denied			*/
#define	EFAULT	14	/* Bad address				*/
#define	ENOTBLK	15	/* Block device required		*/
#define	EBUSY	16	/* Mount device busy			*/
#define	EEXIST	17	/* File exists				*/
#define	EXDEV	18	/* Cross-device link			*/
#define	ENODEV	19	/* No such device			*/
#define	ENOTDIR	20	/* Not a directory			*/
#define	EISDIR	21	/* Is a directory			*/
#define	EINVAL	22	/* Invalid argument			*/
#define	ENFILE	23	/* File table overflow			*/
#define	EMFILE	24	/* Too many open files			*/
#define	ENOTTY	25	/* Not a typewriter			*/
#define	ETXTBSY	26	/* Text file busy			*/
#define	EFBIG	27	/* File too large			*/
#define	ENOSPC	28	/* No space left on device		*/
#define	ESPIPE	29	/* Illegal seek				*/
#define	EROFS	30	/* Read only file system		*/
#define	EMLINK	31	/* Too many links			*/
#define	EPIPE	32	/* Broken pipe				*/
#define	EDOM	33	/* Math arg out of domain of func	*/
#define	ERANGE	34	/* Math result not representable	*/
#define	ENOMSG	35	/* No message of desired type		*/
#define	EIDRM	36	/* Identifier removed			*/
#define	ECHRNG	37	/* Channel number out of range		*/
#define	EL2NSYNC 38	/* Level 2 not synchronized		*/
#define	EL3HLT	39	/* Level 3 halted			*/
#define	EL3RST	40	/* Level 3 reset			*/
#define	ELNRNG	41	/* Link number out of range		*/
#define	EUNATCH 42	/* Protocol driver not attached		*/
#define	ENOCSI	43	/* No CSI structure available		*/
#define	EL2HLT	44	/* Level 2 halted			*/
#define EDEADLK 45	/* Deadlock Condition.			*/

/*
 * Network error messages
 */

/* non-blocking and interrupt i/o */
#define	EWOULDBLOCK	55		/* Operation would block */
#define	EINPROGRESS	56		/* Operation now in progress */
#define	EALREADY	57		/* Operation already in progress */

	/* argument errors */
#define	ENOTSOCK	58		/* Socket operation on non-socket */
#define	EDESTADDRREQ	59		/* Destination address required */
#define	EMSGSIZE	60		/* Message too long */
#define	EPROTOTYPE	61		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	62		/* Protocol not available */
#define	EPROTONOSUPPORT	63		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	64		/* Socket type not supported */
#define	EOPNOTSUPP	65		/* Operation not supported on socket */
#define	EPFNOSUPPORT	66		/* Protocol family not supported */
#define	EAFNOSUPPORT	67		/* Address family not supported by protocol family */
#define	EADDRINUSE	68		/* Address already in use */
#define	EADDRNOTAVAIL	69		/* Can't assign requested address */

	/* operational errors */
#define	ENETDOWN	70		/* Network is down */
#define	ENETUNREACH	71		/* Network is unreachable */
#define	ENETRESET	72		/* Network dropped connection on reset */
#define	ECONNABORTED	73		/* Software caused connection abort */
#define	ECONNRESET	74		/* Connection reset by peer */
#define	ENOBUFS		75		/* No buffer space available */
#define	EISCONN		76		/* Socket is already connected */
#define	ENOTCONN	77		/* Socket is not connected */
#define	ESHUTDOWN	78		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	79		/* Too many references: can't splice */
#define	ETIMEDOUT	80		/* Connection timed out */
#define	ECONNREFUSED	81		/* Connection refused */

	/* */
#define	ELOOP		82		/* Too many levels of symbolic links */
#define	ENAMETOOLONG	83		/* File name too long */
#define	EHOSTDOWN	84		/* Host is down */
#define EHOSTUNREACH	85		/* No route to host */
#define	ENOTEMPTY	86		/* Directory not empty */


/* streams errors */

#define	ENOSTR		87		/* Device not a stream */
#define	ENODATA		88		/* No data (for no delay IO) */
#define	ETIME		89		/* Timer expired */
#define	ENOSR		90		/* Out of streams resources */

/* Network File System */
#define	ESTALE		95		/* Stale NFS file handle */
#define	EREMOTE		96		/* Too many levels of remote in path */
#define	EPROCLIM	97		/* Too many processes */
#define	EUSERS		98		/* Too many users */
#define	EDQUOT		99		/* Disc quota exceeded */

/*
 * Other errors
 */
#define	EDEADLOCK	100	/* Locking deadlock error		*/
