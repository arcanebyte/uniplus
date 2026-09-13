/*	@(#)netisr.h 1.1 86/02/03 SMI; from UCB 4.4 83/07/06	*/
/*	@(#)netisr.h	2.1 86/04/15 NFSSRC */

/*
 * The networking code runs off software interrupts.
 *
 * You can switch into the network by doing splnet() and return by splx().
 * The software interrupt level for the network is higher than the software
 * level for the clock (so you can enter the network in routines called
 * at timeout time).
 */

/*
 * These definitions are only to provide compatibility
 * with old code; new stuff should use softcall directly
 */
#define	schednetisr(isrname)	softcall(isrname, (caddr_t)0)

#define	NETISR_RAW	rawintr		/* raw net intr */
#define	NETISR_IP	ipintr		/* IP net intr */
#define	NETISR_NS	nsintr		/* NS net intr */

int rawintr();

#ifdef INET
int ipintr();
#endif INET

#ifdef NS
int nsintr();
#endif NS
