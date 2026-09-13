/*	if_ec.h	4.2	82/04/11	*/

/*
 * Structure of an Ethernet header.
 */
struct	ec_header {
	u_char	ec_dhost[6];
	u_char	ec_shost[6];
	u_short	ec_type;
};

#define	ECPUP_PUPTYPE	0x0400		/* PUP protocol */
#define	ECPUP_IPTYPE	0x0800		/* IP protocol */

/*
 * The ECPUP_NTRAILER packet types starting at ECPUP_TRAIL have
 * (type-ECPUP_TRAIL)*512 bytes of data followed
 * by a PUP type (as given above) and then the (variable-length) header.
 */
#define	ECPUP_TRAIL	0x1000		/* Trailer PUP */
#define	ECPUP_NTRAILER	16
