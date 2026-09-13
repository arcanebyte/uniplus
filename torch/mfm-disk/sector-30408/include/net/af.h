/*	@(#)af.h 1.1 86/02/03 SMI; from UCB 4.3 82/03/30        */
/*	@(#)af.h	2.1 86/04/15 NFSSRC */

/*
 * Address family routines,
 * used in handling generic sockaddr structures.
 *
 * Hash routine is called
 *	af_hash(addr, h);
 *	struct sockaddr *addr; struct afhash *h;
 * producing an afhash structure for addr.
 *
 * Netmatch routine is called
 *	af_netmatch(addr1, addr2);
 * where addr1 and addr2 are sockaddr *.  Returns 1 if network
 * values match, 0 otherwise.
 */
struct afswitch {
	int	(*af_hash)();
	int	(*af_netmatch)();
};

struct afhash {
	u_int	afh_hosthash;
	u_int	afh_nethash;
};

#ifdef KERNEL
extern struct	afswitch afswitch[];
#endif
