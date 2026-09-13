/* @(#)systm.h	6.1 */
/*
 * Random set of variables used by more than one routine.
 */
extern time_t lbolt;		/* time in HZ since last boot */
extern struct timeval time;	/* current time */
extern int ticks;		/* clock ticks since time last incremented */

extern char runin;		/* scheduling flag */
extern char runout;		/* scheduling flag */
extern char runrun;		/* scheduling flag */
extern char curpri;		/* current priority */
extern struct proc *curproc;	/* current proc */
extern struct proc *runq;	/* head of linked list of running processes */

extern	cputype;		/* type of cpu = 40, 45, 70, 750, 780, 68000 */
extern	maxmem;			/* max available memory */
extern	minmem;			/* max available memory small hole */
extern	physmem;		/* physical memory on this CPU */
extern daddr_t swplo;		/* block number of swap space */
extern	nswap;			/* size of swap space */
extern dev_t rootdev;		/* device of the root */
extern dev_t swapdev;		/* swapping device */
extern struct vnode *swapdev_vp;	/* vnode equivalent to above */
extern dev_t pipedev;		/* pipe device */
extern struct vnode *pipedev_vp;	/* vnode equivalent to above */
extern char *panicstr;		/* panic string pointer */
extern	blkacty;		/* active block devices */
extern	sspeed;			/* default console speed */
extern	short icode[];		/* user init code */
extern	szicode;		/* init code size */
extern	char regloc[];		/* locs of saved user registers (trap.c) */
extern	pwr_cnt, pwr_act;
extern int (*putchar)();
extern int (*pwr_clr[])();

dev_t getmdev();
dev_t getpdev();
caddr_t vtop();
daddr_t	bmap();
struct inode *ialloc();
struct inode *iget();
struct inode *owner();
struct inode *maknode();
struct inode *namei();
struct buf *alloc();
struct buf *getblk();
struct buf *geteblk();
struct buf *bread();
struct buf *breada();
struct filsys *trygetfs();
struct file *getf();
struct file *falloc();
int	uchar();
caddr_t		kmem_alloc();
struct vnode *devtovp();

extern	char	hostname[];
extern	int	hostnamelen;
int	selwait;			/* wchan for select waits */
extern struct proc	*pfind();
extern struct file	*getsock();

#define	nofault	u.u_nofault	/* kludge for backward compatibility */
