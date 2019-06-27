
/*
 * Data Copy disc packet parameters structure
 */
struct datacopyparms {
	unsigned char	pm_opcode;	/* operation code = PMFMT */
#define	PMDC		0x01		/* opcode for data copy */
	unsigned char	pm_scr1;	/* not used */
	unsigned char	pm_sc0;		/* step control: OCD, FB, EOF ACT, ERR ACT */
	unsigned char	pm_sc1;		/* step control: SR, ELM, IEC, TIE */
	unsigned short	pm_tsize;	/* transfer length */
	unsigned char	pm_sdev;	/* source device */
	unsigned char	pm_stah;	/* source xfer address hi byte */
	unsigned char	pm_stam;	/* source xfer address mid byte */
	unsigned char	pm_stal;	/* source xfer address low byte */
	unsigned char	pm_scr2;	/* not used */
	unsigned char	pm_scr3;	/* not used */
	unsigned char	pm_ddev;	/* destination device */
	unsigned char	pm_dtah;	/* destination xfer address hi byte */
	unsigned char	pm_dtam;	/* destination xfer address mid byte */
	unsigned char	pm_dtal;	/* destination xfer address low byte */
};

struct results {
	int errcode;
	int nblks;
};

/* various data copy states */

#define ISSUE	7
#define RESUME	8
#define RPSTAT	9
#define RPSTAT2	10
#define ABORT	11
