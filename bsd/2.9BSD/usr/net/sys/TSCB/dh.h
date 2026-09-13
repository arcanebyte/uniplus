/*
 * NDH and NDM are in units of boards (16 lines each).
 * LOWDM is the unit number of the first unit with a DM-11
 * (e.g. 16 if the first DH has no DM, the second does have one).
 * All units from LOWDM through LOWDM + (NDM*16) are assumed to have
 * modem control (with the exception of those with 0200 bit on in
 * their minor device numbers if DH_SOFTCAR is defined).
 */
#define	NDH		1
#define	NDM		1
#define	LOWDM		0
#define	DH_SOFTCAR
#define	DH_SILO
