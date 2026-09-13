				/* Triple X Graphics for System V GPS */
				/* some standard defs */

#define RANGE_X		(64 * 40)
#define LOWLEFTX	0
#define RANGE_Y		(24 * 64)
#define LOWLEFTY	RANGE_Y

		/* selectable colours */
#define	DARKBLUE	0
#define WHITE		5
#define GREEN_GREY	6
#define RED_GREEN	7
#define DARKRED		8
#define GREY		9
#define BROKENGREY	10
#define VLIGHTGREY	11
#define SPECLEGREY	12
#define DARKGREEN	13
#define PINK		14
#define CHERRYRED	15
#define MUDDYYELLOW	16
#define GREEN		17
#define HALFGREY	18
#define LIMEGREEN	19
#define SALMON		20
#define BEIGE		24
#define PURPLE		25

				/* weights for lines */
#define	NARROW_W	4	/* narrow line width for pen */		
#define	NARROW_H	8	/* narrow line height for pen */		
#define	MEDIUM_W	8	/* medium line width for pen */		
#define	MEDIUM_H	8	/* medium line height for pen */		
#define	BOLD_W		12	/* bold line width for pen */		
#define	BOLD_H		12	/* bold line height for pen */		

#define X_CH_ADJ	-16	/* adjustment of character ref point */
#define Y_CH_ADJ	32	/* adjustment of character ref point */

#define	TEXLEN		256
#define	MAXLINES	256
#define	LINES		0
#define	MOVE		1
#define	TEXT		2
#define	ARCS		3
#define	ALPHA		4
#define	COMMENT		15
#define	LIMIT		32768

union cmdword {
	struct {
		short pntw;
	} cma;
	struct {
		char cmbyte2;
		char cmbyte1;
	} cmb;
	struct fbits {
		unsigned cn : 12;	/* count of words in cmd */
		unsigned cm : 4;	/* command */
	} cmc;
};

union styleword {
	struct {
		short in;
	} sta;
	struct sbits {
		unsigned st : 4;	/* style */
		unsigned wt : 4;	/* weight */
		unsigned col : 8;	/* colour */
	} stb;
	struct sbits1 {
		unsigned fon : 8;	/* font */
		unsigned col : 8;	/* colour */
	} stc;
};

union szorient {
	struct {
		short in;
	} sza;
	struct tbits {
		unsigned tr : 8;	/* text rotation */
		unsigned ts : 8;	/* text size */
	} szb;
};


struct command {
	int cmd, cnt, color, weight, style, font, trot, tsize;
	int *aptr, array[MAXLINES *2];
	char *tptr, text[TEXLEN];
};


#define	XYMIN		(-32767)
#define XYMID		0
#define XYMAX		32767
#define XDIM		(XYMAX - XYMIN)
#define YDIM		(XYMAX - XYMIN)
#define ROWS		5
#define COLUMNS		5
#define REGIONS		25
#define DEF_REGION	13
#define FCH		'\037'

#define INVISIBLE	0
#define NARROW		1
#define MEDIUM		2
#define WIDE		4

#define SOLID		0
#define DOTTED		1
#define DOTDASH		2
#define DASHED		3
#define LONGDASH	4

#define GPSBLACK	0
#define GPSBLUE		1
#define GPSGREEN	2
#define GPSRED		4
#define FONTNSO		0

#define FONTNDO		1
#define FONTNDD		2
#define FONTNDA		3
#define FONTNLD		4

#define FONTBSO		16
#define FONTBSD		17
#define FONTBDD		18
#define FONTBDA		19
#define FONTBLD		20

#define FONTMSO		32
#define FONTMSD		33
#define FONTMDD		34
#define FONTMDA		35
#define FONTMLD		36
