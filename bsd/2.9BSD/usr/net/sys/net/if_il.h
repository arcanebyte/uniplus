/*	if_il.h	4.3	82/08/25	*/

/*
 * Structure of an Ethernet header -- transmit format
 *	(source address insertion disabled)
 */
struct	il_xheader {
	u_char	ilx_dhost[6];		/* Destination Host */
	u_char	ilx_shost[6];		/* Source Host */
	u_short	ilx_type;		/* Type of packet */
};

/*
 * Structure of an Ethernet header -- receive format
 */
struct	il_rheader {
	u_char	ilr_status;		/* Frame Status */
	u_char	ilr_fill1;
	u_short	ilr_length;		/* Frame Length */
	u_char	ilr_dhost[6];		/* Destination Host */
	u_char	ilr_shost[6];		/* Source Host */
	u_short	ilr_type;		/* Type of packet */
};

#define	ILPUP_PUPTYPE	0x0400		/* PUP protocol */
#define	ILPUP_IPTYPE	0x0800		/* IP protocol */
#define	ILPUP_ARPTYPE	0x0806		/* Address resolution protocol */

/*
 * The ILPUP_NTRAILER packet types starting at ILPUP_TRAIL have
 * (type-ILPUP_TRAIL)*512 bytes of data followed
 * by a PUP type (as given above) and then the (variable-length) header.
 */
#define	ILPUP_TRAIL	0x1000		/* Trailer PUP */
#define	ILPUP_NTRAILER	16

#if pdp11
#define ils_fill1 ils_f1
#define ils_fill2 ils_f2
#endif

/*
 * Structure of statistics record
 */
struct	il_stats {
	u_short	ils_fill1;
	u_short	ils_length;		/* Length (should be 62) */
	u_char	ils_addr[6];		/* Ethernet Address */
	u_short	ils_frames;		/* Number of Frames Received */
	u_short	ils_rfifo;		/* Number of Frames in Receive FIFO */
	u_short	ils_xmit;		/* Number of Frames Transmitted */
	u_short	ils_xcollis;		/* Number of Excess Collisions */
	u_short	ils_frag;		/* Number of Fragments Received */
	u_short	ils_lost;		/* Number of Times Frames Lost */
	u_short	ils_multi;		/* Number of Multicasts Accepted */
	u_short	ils_rmulti;		/* Number of Multicasts Rejected */
	u_short	ils_crc;		/* Number of CRC Errors */
	u_short	ils_align;		/* Number of Alignment Errors */
	u_short	ils_collis;		/* Number of Collisions */
	u_short	ils_owcollis;		/* Number of Out-of-window Collisions */
	u_short	ils_fill2[8];
	char	ils_module[8];		/* Module ID */
	char	ils_firmware[8];	/* Firmware ID */
};

/*
 * Structure of Collision Delay Time Record
 */
struct	il_collis {
	u_short	ilc_fill1;
	u_short	ilc_length;		/* Length (should be 0-32) */
	u_short	ilc_delay[16];		/* Delay Times */
};

u_char etherbroadcastaddr[6];			/* 6 bytes of 0xFF */

/*
 * Ethernet Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	ether_arp {
	u_short	arp_hrd;	/* format of hardware address */
#define ARPHRD_ETHER 	1	/* ethernet hardware address */
	u_short	arp_pro;	/* format of proto. address (ETHERPUP_IPTYPE) */
	u_char	arp_hln;	/* length of hardware address (6) */
	u_char	arp_pln;	/* length of protocol address (4) */
	u_short	arp_op;
#define	ARPOP_REQUEST	1	/* request to resolve address */
#define	ARPOP_REPLY	2	/* response to previous request */
	u_char	arp_sha[6];	/* sender hardware address */
	u_char	arp_spa[4];	/* sender protocol address */
	u_char	arp_tha[6];	/* target hardware address */
	u_char	arp_tpa[4];	/* target protocol address */
};

#ifdef	KERNEL
/*
 * Structure shared between the ethernet driver modules and
 * the address resolution code.  For example, each ec_softc or il_softc
 * begins with this structure.
 */
struct	arpcom {
	struct 	ifnet ac_if;	/* network-visible interface */
	u_char	ac_enaddr[6];	/* ethernet hardware address */
	struct	arpcom *ac_ac;	/* link to next ether driver */
};

struct	in_addr arpmyaddr();
struct	arptab *arptnew();
#endif
