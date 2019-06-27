/* Flags in conversion tables to pick out non-character generating
 * keys.
 */

/* When a character is received from the COPS (depending on the state of
 * the shift keys) a table is used to convert it into the ascii equiv.
 * If the result is one of the following then it wasn't a keycode at all
 * but one of the reset/mouse/clock/plug codes.
 */
#define Imp 0xFF	/* invalid keycode */
#define KSC 0xFE	/* mouse or reset state flag */
#define D1P 0xFD	/* disk inserted into drive one */
#define D1B 0xFC	/* disk 1 button pushed */
#define D2P 0xFB	/* disk inserted into lower drive */
#define D2B 0xFA	/* disk 2 button pushed */
#define PPP 0xF9	/* parrallel port plug */
#define MSB 0xF8	/* mouse button */
#define MSP 0xF7	/* mouse plug */
#define OFF 0xF6	/* soft off */

#define LCK 0xF2	/* Alpha lock on */
#define SFT 0xF1	/* shift key */
#define CMD 0xF0	/* command key */

#define KB_CTRL  0x0
#define KB_SHFT  0x1
#define KB_LOCK  0x2
#define KB_OFF   0x6
#define KB_MSP   0x7
#define KB_MSB   0x8
#define KB_PPORT 0x9
#define KB_D2B   0xA
#define KB_D2P   0xB
#define KB_D1B   0xC
#define KB_D1P   0xD
#define KB_STATE 0xE
#define KB_IMP   0xF

/* Cops reset codes */
#define KB_KBCOPS 0xFF
#define KB_IOCOPS 0xFE
#define KB_UNPLUG 0xFD
#define KB_CLOCKT 0xFC
#define KB_SFTOFF 0xFB
#define KB_RESERV 0xF0
#define KB_RDCLK  0xE0

/* Case change keys
 */
#define ROpt 0x4E
#define LOpt 0x7C			/* mapped onto escape */
#define Lck 0x7D			/* SHIFT key */
#define Sft 0x7E			/* SHIFT key */
#define Cmd 0x7F			/* COMMAND key */

/* ASCII codes */
#define Del 0x7F
#define Bkp 0x08			/** backspace key */
#define Esc 0x1B
#define Nl  0x0A
#define Cr  0x0D
#define Tab 0x09

/** NOTES on character mapping
 ** Left Option	-> Esc
 ** Clear	-> Del
 ** Enter	-> Nl
 ** arrows      -> cursor motion in appropriate direction (sequence \E[<Arrow-value>)
 **/
#define	Alt	'D'
#define	Art	'C'
#define	Aup	'A'
#define	Adn	'B'
#define ARROW(i,v)	( ((i&0x70)==0x20) && (v>='A') && (v<='D') )
/** left arrow  -> control h
 ** right arrow -> control l
 ** up arrow    -> control k
 ** down arrow  -> control j
#define Cth	('h'&0x1F)
#define Ctl	('l'&0x1F)
#define Ctk	('k'&0x1F)
#define Ctj	('j'&0x1F)
 **/

char ToLA[] = {	/* convert keycode to lowercase ascii character
 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F*/
KSC, D1P, D1B, D2P, D2B, PPP, MSB, MSP, OFF, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
Del, '-', Alt, Art, '7', '8', '9', Aup, '4', '5', '6', Adn, '.', '2', '3',  Nl,
Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
'-', '=','\\', Imp, 'p', Bkp,  Nl, Imp,  Cr, '0', Imp, Imp, '/', '1', Esc, Imp,
'9', '0', 'u', 'i', 'j', 'k', '[', ']', 'm', 'l', ';','\'', ' ', ',', '.', 'o',
'e', '6', '7', '8', '5', 'r', 't', 'y', '`', 'f', 'g', 'h', 'v', 'c', 'b', 'n',
'a', '2', '3', '4', '1', 'q', 's', 'w', Tab, 'z', 'x', 'd', Esc, LCK, SFT, CMD,
};

char ToCC[] = {	/* convert keycode to shift-locked ascii character
 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F*/
KSC, D1P, D1B, D2P, D2B, PPP, MSB, MSP, OFF, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
Del, '-', Alt, Art, '7', '8', '9', Aup, '4', '5', '6', Adn, '.', '2', '3',  Nl,
Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
'-', '=','\\', Imp, 'P', Bkp,  Nl, Imp,  Cr, '0', Imp, Imp, '/', '1', Esc, Imp,
'9', '0', 'U', 'I', 'J', 'K', '[', ']', 'M', 'L', ';','\'', ' ', ',', '.', 'O',
'E', '6', '7', '8', '5', 'R', 'T', 'Y', '`', 'F', 'G', 'H', 'V', 'C', 'B', 'N',
'A', '2', '3', '4', '1', 'Q', 'S', 'W', Tab, 'Z', 'X', 'D', Esc, LCK, SFT, CMD,
};

char ToUA[] = {	/* Convert keycode into uppercase ascii character
 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F*/
KSC, D1P, D1B, D2P, D2B, PPP, MSB, MSP, OFF, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
Del, '-', '+', '*', '7', '8', '9', '/', '4', '5', '6', ',', '.', '2', '3',  Nl,
Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp, Imp,
'_', '+', '|', Imp, 'P', Bkp,  Nl, Imp,  Cr, '0', Imp, Imp, '?', '1', Esc, Imp,
'(', ')', 'U', 'I', 'J', 'K', '{', '}', 'M', 'L', ':', '"', ' ', '<', '>', 'O',
'E', '^', '&', '*', '%', 'R', 'T', 'Y', '~', 'F', 'G', 'H', 'V', 'C', 'B', 'N',
'A', '@', '#', '$', '!', 'Q', 'S', 'W', Tab, 'Z', 'X', 'D', Esc, LCK, SFT, CMD,
};

#define	Nil	0
char altkpad[] = {	/* convert keycode into alternate keypad mode values
 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F*/
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
Nil, 'm', 'P', 'Q', 'w', 'x', 'y', 'R', 't', 'u', 'v', 'l', 'n', 'r', 's', 'M',
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, 'p', Nil, Nil, Nil, 'q', Nil, Nil,
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
};

char *ccvtab[] = {
	ToLA,		/* LCK up, SFT up */
	ToCC,		/* LCK down, SFT up */
	ToUA,		/* LCK up, SFT down */
	ToUA,		/* LCK down, SFT down */
};
