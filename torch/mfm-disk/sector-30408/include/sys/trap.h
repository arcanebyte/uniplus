/* @(#)trap.h	6.1 */
/*
 * Trap type values
 */

#define	BUSERR	2	/* bus error fault */
#define	ADDRERR	3	/* address error fault */
#define	ILLINST	4	/* illegal instruction fault */
#define	DIVZERO	5	/* zero divide fault */
#define	CHK	6	/* CHK instruction */
#define	TRAPV	7	/* TRAPV instruction */
#define	PRIVVIO	8	/* privilege violation fault */
#define	TRCTRAP	9	/* trace trap */
#define	L1010	10	/* line 1010 emulation trap */
#define	L1111	11	/* line 1111 emulation trap */
#define	SPURINT	24	/* spurious interrupt trap */
#define	PRTYFLT	28	/* memory parity fault */
#define	SYSCLL0	32	/* trap instruction (syscall trap) - m68k generic */
#define	TRAP1	33	/* TRAP 1 */
#define	TRAP2	34	/* TRAP 2 */
#define	TRAP3	35	/* TRAP 3 */
#define	TRAP4	36	/* TRAP 4 */
#define	TRAP5	37	/* TRAP 5 */
#define	TRAP6	38	/* TRAP 6 */
#define	TRAP7	39	/* TRAP 7 */
#define	TRAP8	40	/* TRAP 8 */
#define	TRAP9	41	/* TRAP 9 */
#define	TRAP10	42	/* TRAP 10 */
#define	TRAP11	43	/* TRAP 11 */
#define	TRAP12	44	/* TRAP 12 */
#define	TRAP13	45	/* TRAP 13 */
#define	TRAP14	46	/* TRAP 14 */
#define	SYSCLL1	47	/* TRAP 15 - system call - UniSoft */
#define	RESCHED	256	/* software level 1 trap (reschedule trap) */
