/* @(#)sysent.c	1.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 */

int	alarm();
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dup();
int	exec();
int	exece();
int	fcntl();
int	fork();
int	fstat();
int	getgid();
int	getpid();
int	getuid();
int	gtime();
int	gtty();
int	ioctl();
int	kill();
int	link();
int	lock();
int	mknod();
int	msgsys();
int	nice();
int	nosys();
int	nullsys();
int	open();
int	pause();
int	pipe();
int	profil();
int	ptrace();
int	read();
int	rexit();
int	saccess();
int	sbreak();
int	seek();
int	semsys();
int	setgid();
int	setpgrp();
int	setuid();
int	shmsys();
int	smount();
int	ssig();
int	stat();
int	stime();
int	stty();
int	sumount();
int	sync();
int	sysacct();
int	times();
int	ulimit();
int	umask();
int	unlink();
int	utime();
int	utssys();
int	wait();
int	write();

#ifdef UCB_NET
#include "net/misc.h"
/* net stuff */
int	select();	/* not implimented for character devices yet */
int	gethostname();
int	sethostname();
int	ssocket();
int	sconnect();
int	saccept();
int	ssend();
int	sreceive();
int	ssocketaddr();
int	netreset();
#endif

/*
 * Local system calls
 */
int	locking();
int	phys();
int	reboot();

struct sysent sysent[] =
{
	nosys,			/*  0 = indir */
	rexit,			/*  1 = exit */
	fork,			/*  2 = fork */
	read,			/*  3 = read */
	write,			/*  4 = write */
	open,			/*  5 = open */
	close,			/*  6 = close */
	wait,			/*  7 = wait */
	creat,			/*  8 = creat */
	link,			/*  9 = link */
	unlink,			/* 10 = unlink */
	exec,			/* 11 = exec */
	chdir,			/* 12 = chdir */
	gtime,			/* 13 = time */
	mknod,			/* 14 = mknod */
	chmod,			/* 15 = chmod */
	chown,			/* 16 = chown; now 3 args */
	sbreak,			/* 17 = break */
	stat,			/* 18 = stat */
	seek,			/* 19 = seek */
	getpid,			/* 20 = getpid */
	smount,			/* 21 = mount */
	sumount,		/* 22 = umount */
	setuid,			/* 23 = setuid */
	getuid,			/* 24 = getuid */
	stime,			/* 25 = stime */
	ptrace,			/* 26 = ptrace */
	alarm,			/* 27 = alarm */
	fstat,			/* 28 = fstat */
	pause,			/* 29 = pause */
	utime,			/* 30 = utime */
	stty,			/* 31 = stty */
	gtty,			/* 32 = gtty */
	saccess,		/* 33 = access */
	nice,			/* 34 = nice */
	nosys,			/* 35 = sleep; inoperative */
	sync,			/* 36 = sync */
	kill,			/* 37 = kill */
	nosys,			/* 38 = x */
	setpgrp,		/* 39 = setpgrp */
	nosys,			/* 40 = tell - obsolete */
	dup,			/* 41 = dup */
	pipe,			/* 42 = pipe */
	times,			/* 43 = times */
	profil,			/* 44 = prof */
	lock,			/* 45 = proc lock */
	setgid,			/* 46 = setgid */
	getgid,			/* 47 = getgid */
	ssig,			/* 48 = sig */
	msgsys,			/* 49 = IPC Messages */
	nosys,			/* 50 = reserved for local use */
	sysacct,		/* 51 = turn acct off/on */
	shmsys,			/* 52 = IPC Shared Memory */
	semsys,			/* 53 = IPC Semaphores */
	ioctl,			/* 54 = ioctl */
	phys,			/* 55 = phys */
	locking,		/* 56 = file locking */
	utssys,			/* 57 = utssys */
	nosys,			/* 58 = reserved for USG */
	exece,			/* 59 = exece */
	umask,			/* 60 = umask */
	chroot,			/* 61 = chroot */
	fcntl,			/* 62 = fcntl */
	ulimit,			/* 63 = ulimit */

	reboot,			/* 64 = reboot */
	nosys,			/* 65 = x */
	nosys,			/* 66 = x */
	nosys,			/* 67 = x */
	nosys,			/* 68 = x */
	nosys,			/* 69 = x */
#ifdef UCB_NET
	select,			/* 70 = select */
	gethostname,		/* 71 = gethostname */
	sethostname,		/* 72 = sethostname */
	ssocket,		/* 73 = socket */
	saccept,		/* 74 = accept */
	sconnect,		/* 75 = connect */
	sreceive,		/* 76 = receive */
	ssend,			/* 77 = send */
	ssocketaddr,		/* 78 = socketaddr */
	netreset,		/* 79 = netreset */
#else
	nosys,			/* 70 = x */
	nosys,			/* 71 = x */
	nosys,			/* 72 = x */
	nosys,			/* 73 = x */
	nosys,			/* 74 = x */
	nosys,			/* 75 = x */
	nosys,			/* 76 = x */
	nosys,			/* 77 = x */
	nosys,			/* 78 = x */
	nosys,			/* 79 = x */
#endif UCB_NET
	nosys,			/* 80 = x */
	nosys,			/* 81 = x */
	nosys,			/* 82 = x */
	nosys,			/* 83 = x */
	nosys,			/* 84 = x */
	nosys,			/* 85 = x */
	nosys,			/* 86 = x */
	nosys,			/* 87 = x */
	nosys,			/* 88 = x */
	nosys,			/* 89 = x */
	nosys,			/* 90 = x */
	nosys,			/* 91 = x */
	nosys,			/* 92 = x */
	nosys,			/* 93 = x */
	nosys,			/* 94 = x */
	nosys,			/* 95 = x */
	nosys,			/* 96 = x */
	nosys,			/* 97 = x */
	nosys,			/* 98 = x */
	nosys,			/* 99 = x */
	nosys,			/* 100 = x */
	nosys,			/* 101 = x */
	nosys,			/* 102 = x */
	nosys,			/* 103 = x */
	nosys,			/* 104 = x */
	nosys,			/* 105 = x */
	nosys,			/* 106 = x */
	nosys,			/* 107 = x */
	nosys,			/* 108 = x */
	nosys,			/* 109 = x */
	nosys,			/* 110 = x */
	nosys,			/* 111 = x */
	nosys,			/* 112 = x */
	nosys,			/* 113 = x */
	nosys,			/* 114 = x */
	nosys,			/* 115 = x */
	nosys,			/* 116 = x */
	nosys,			/* 117 = x */
	nosys,			/* 118 = x */
	nosys,			/* 119 = x */
	nosys,			/* 120 = x */
	nosys,			/* 121 = x */
	nosys,			/* 122 = x */
	nosys,			/* 123 = x */
	nosys,			/* 124 = x */
	nosys,			/* 125 = x */
	nosys,			/* 126 = x */
	nosys,			/* 127 = x */
};
