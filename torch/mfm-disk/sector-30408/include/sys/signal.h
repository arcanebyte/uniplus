/* NFSSRC @(#)signal.h	2.1 86/04/11 */
/* @(#)signal.h	6.1 */

#ifndef	NSIG
#define	NSIG	32

#define	SIGHUP		1	/* hangup */
#define	SIGINT		2	/* interrupt (rubout) */
#define	SIGQUIT		3	/* quit (ASCII FS) */
#define	SIGILL		4	/* illegal instruction (not reset when caught)*/
#define	SIGTRAP		5	/* trace trap (not reset when caught) */
#define	SIGIOT		6	/* IOT instruction */
#define	SIGEMT		7	/* EMT instruction */
#define	SIGFPE		8	/* floating point exception */
#define	SIGKILL		9	/* kill (cannot be caught or ignored) */
#define	SIGBUS		10	/* bus error */
#define	SIGSEGV		11	/* segmentation violation */
#define	SIGSYS		12	/* bad argument to system call */
#define	SIGPIPE		13	/* write on a pipe with no one to read it */
#define	SIGALRM		14	/* alarm clock */
#define	SIGTERM		15	/* software termination signal from kill */
#define	SIGUSR1		16	/* user defined signal 1 */
#define	SIGUSR2		17	/* user defined signal 2 */
#define	SIGCLD		18	/* death of a child */
#define	SIGCHLD		SIGCLD
#define	SIGPWR		19	/* power-fail restart */
#define	SIGTSTP		20	/* stop signal from tty */
#define	SIGTTIN		21	/* to readers pgrp upon background tty read */
#define	SIGTTOU		22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGSTOP		23	/* sendable stop signal not from tty */
#define	SIGXCPU		24	/* exceeded CPU time limit */
#define	SIGXFSZ		25	/* exceeded file size limit */
#define	SIGVTALRM	26	/* virtual time alarm */
#define	SIGPROF		27	/* profiling time alarm */
#define	SIGWINCH	28	/* window changed */
#define	SIGCONT		29	/* continue a stopped process */
#define	SIGURG		30	/* urgent condition on IO channel */
#define	SIGIO		31	/* input/output possible signal */

/* SIGFPE codes */
#define	KINTOVF		1	/* integer overflow */
#define	KINTDIV		2	/* integer divide by zero */
#define	KFLTOVF		3	/* floating overflow */
#define	KFLTDIV		4	/* floating divide by zero */
#define	KDECDIV		KFLTDIV	/* decimal divide by zero */
#define	KFLTUND		5	/* floating underflow */
#define	KDECOVF		6	/* decimal overflow */
#define	KSUBRNG		7	/* subscript range */

/*
 * Signal vector "template" used in sigvec call.
 */
struct	sigvec {
	int	(*sv_handler)();	/* signal handler */
	int	sv_mask;		/* signal mask to apply */
	int	sv_onstack;		/* if non-zero, take on signal stack */
};

/*
 * Structure used in sigstack call.
 */
struct	sigstack {
	char	*ss_sp;			/* signal stack pointer */
	int	ss_onstack;		/* current status */
};

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to properly restore state if
 * a non-standard exit is performed.
 */
struct	sigcontext {
	int	sc_onstack;		/* sigstack state to restore */
	int	sc_mask;		/* signal mask to restore */
	int	sc_sp;			/* sp to restore */
	int	sc_pc;			/* pc to retore */
	int	sc_ps;			/* psl to restore */
};

#define	BADSIG		(int (*)())-1
#define	SIG_DFL		(int (*)())0
#ifdef	lint
#define	SIG_IGN		(int (*)())0
#else
#define	SIG_IGN		(int (*)())1
#endif

#ifdef	KERNEL
#define	SIG_CATCH	(int (*)())2
#define	SIG_HOLD	(int (*)())3
#else	KERNEL
#define	sigmask(n)	(1<<((n)-1))
#endif	KERNEL
#endif	!NSIG
