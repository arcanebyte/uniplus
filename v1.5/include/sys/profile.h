/* Disk Header Layout
 * Used by the lisa office system
 */
struct proheader {
	unsigned short	ph_version;	/* OS version number */
	unsigned short	ph_datastk:2,	/* kind of info in this block */
			ph_reserved:6,
			ph_volume:8;	/* disk volumn number */
	unsigned short	ph_fileid;	/* type of file */
	unsigned short	ph_havcsum:1,	/* whether ph_chksum is valid */
			ph_dataused:15;	/* valid byte count */
	unsigned int	ph_abspage:24,	/* page in filesystem */
			ph_chksum:8;	/* checksum */
	unsigned short	ph_relpage;	/* page in file */
	unsigned int	ph_forward:24,	/* next block */
			ph_bckhigh:8;	/* high byte of prev block */
	unsigned short	ph_bcklow;	/* low word of prev block */
};

#define HDRSIZE (sizeof (struct proheader))

struct proidblk {
	char	pi_devn[13];	/* device name */
	char	pi_dnum[3];	/* device number (currently 0) */
	short	pi_revn;	/* micro code revision number */
	char	pi_blkcnt[3];	/* number of useable blocks */
	char	pi_blksiz[2];	/* number of bytes per block */
	char	pi_spavail;	/* number of spare sectors avail */
	char	pi_spalloc;	/* number of spare sectors allocated */
	char	pi_bbcnt;	/* number of bad blocks below */
	char	pi_blist[486];	/* list of bad and spared sectors */
};
