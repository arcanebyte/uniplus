
	/* UniSoft IOCTLs */
#define UIOC	('U'<<8)
#define UIOCFORMAT	(UIOC|0)	/* format disk */
#define UIOCEXTE	(UIOC|1)	/* enable extended error messages */
#define UIOCNEXTE	(UIOC|2)	/* disable extended error messages */
#define UIOCWCHK	(UIOC|3)	/* enable write verification */
#define UIOCNWCHK	(UIOC|4)	/* disable write verification */
#define UIOCGETDT	(UIOC|8)	/* get disk tuning parameters */
#define UIOCSETDT	(UIOC|9)	/* set disk tuning parameters */
#define UIOCBDBK	(UIOC|10)	/* resync disk bad block table */
#define UIOCSIZE	(UIOC|11)	/* get size of disk (in 512-byte blks) */
