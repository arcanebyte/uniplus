
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


