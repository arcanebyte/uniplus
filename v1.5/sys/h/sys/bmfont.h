#define FONTHORZ 8#define FONTVERT 8char bmfont[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* space */0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x10, 0x00,	/* ! */0x48, 0x48, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00,	/* " */0x48, 0x48, 0xFC, 0x48, 0xFC, 0x48, 0x48, 0x00,	/* # */0x10, 0x3C, 0x50, 0x38, 0x14, 0x78, 0x10, 0x00,	/* $ */0x00, 0xC4, 0xC8, 0x10, 0x20, 0x4C, 0x8C, 0x00,	/* % */0x60, 0x90, 0x90, 0x60, 0x94, 0x88, 0x74, 0x00,	/* & */0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,	/* ' */0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00,	/* ( */0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00,	/* ) */0x10, 0x54, 0x38, 0x7C, 0x38, 0x54, 0x10, 0x00,	/* * */0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00,	/* + */0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60,	/* , */0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,	/* - */0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00,	/* . */0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,	/* / */0x78, 0x84, 0x8C, 0xB4, 0xC4, 0x84, 0x78, 0x00,	/* 0 */0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x7C, 0x00,	/* 1 */0x78, 0x84, 0x04, 0x18, 0x60, 0x80, 0xFC, 0x00,	/* 2 */0x78, 0x84, 0x04, 0x38, 0x04, 0x84, 0x78, 0x00,	/* 3 */0x08, 0x18, 0x28, 0x48, 0xFC, 0x08, 0x08, 0x00,	/* 4 */0xFC, 0x80, 0xF0, 0x08, 0x04, 0x88, 0x70, 0x00,	/* 5 */0x38, 0x40, 0x80, 0xF8, 0x84, 0x84, 0x78, 0x00,	/* 6 */0xFC, 0x84, 0x08, 0x10, 0x20, 0x20, 0x20, 0x00,	/* 7 */0x78, 0x84, 0x84, 0x78, 0x84, 0x84, 0x78, 0x00,	/* 8 */0x78, 0x84, 0x84, 0x7C, 0x04, 0x08, 0x70, 0x00,	/* 9 */0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00,	/* : */0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x60,	/* ; */0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00,	/* < */0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00,	/* = */0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00,	/* > */0x78, 0x84, 0x04, 0x18, 0x20, 0x00, 0x20, 0x00,	/* ? */0x38, 0x44, 0x94, 0xAC, 0x98, 0x40, 0x3C, 0x00,	/* @ */0x30, 0x48, 0x84, 0xFC, 0x84, 0x84, 0x84, 0x00,	/* A */0xF8, 0x44, 0x44, 0x78, 0x44, 0x44, 0xF8, 0x00,	/* B */0x78, 0x84, 0x80, 0x80, 0x80, 0x84, 0x78, 0x00,	/* C */0xF8, 0x44, 0x44, 0x44, 0x44, 0x44, 0xF8, 0x00,	/* D */0xFC, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xFC, 0x00,	/* E */0xFC, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0x00,	/* F */0x78, 0x84, 0x80, 0x9C, 0x84, 0x84, 0x78, 0x00,	/* G */0x84, 0x84, 0x84, 0xFC, 0x84, 0x84, 0x84, 0x00,	/* H */0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,	/* I */0x1C, 0x08, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00,	/* J */0x84, 0x88, 0x90, 0xE0, 0x90, 0x88, 0x84, 0x00,	/* K */0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFC, 0x00,	/* L */0x84, 0xCC, 0xB4, 0xB4, 0x84, 0x84, 0x84, 0x00,	/* M */0x84, 0xC4, 0xA4, 0x94, 0x8C, 0x84, 0x84, 0x00,	/* N */0x78, 0x84, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00,	/* O */0xF8, 0x84, 0x84, 0xF8, 0x80, 0x80, 0x80, 0x00,	/* P */0x78, 0x84, 0x84, 0x84, 0x94, 0x88, 0x74, 0x00,	/* Q */0xF8, 0x84, 0x84, 0xF8, 0x90, 0x88, 0x84, 0x00,	/* R */0x78, 0x84, 0x80, 0x78, 0x04, 0x84, 0x78, 0x00,	/* S */0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,	/* T */0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00,	/* U */0x84, 0x84, 0x84, 0x48, 0x48, 0x30, 0x30, 0x00,	/* V */0x84, 0x84, 0x84, 0xB4, 0xB4, 0xCC, 0x84, 0x00,	/* W */0x84, 0x84, 0x48, 0x30, 0x48, 0x84, 0x84, 0x00,	/* X */0x44, 0x44, 0x44, 0x38, 0x10, 0x10, 0x10, 0x00,	/* Y */0xFC, 0x04, 0x08, 0x30, 0x40, 0x80, 0xFC, 0x00,	/* Z */0x78, 0x40, 0x40, 0x40, 0x40, 0x40, 0x78, 0x00,	/* [ */0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00,	/* \ */0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x78, 0x00,	/* ] */0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00,	/* ^ */0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,	/* _ */0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,	/* ` */0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x74, 0x00,	/* a */0x80, 0x80, 0xB8, 0xC4, 0x84, 0xC4, 0xB8, 0x00,	/* b */0x00, 0x00, 0x78, 0x80, 0x80, 0x80, 0x78, 0x00,	/* c */0x04, 0x04, 0x74, 0x8C, 0x84, 0x8C, 0x74, 0x00,	/* d */0x00, 0x00, 0x78, 0x84, 0xFC, 0x80, 0x78, 0x00,	/* e */0x18, 0x24, 0x20, 0xF8, 0x20, 0x20, 0x20, 0x00,	/* f */0x00, 0x00, 0x74, 0x8C, 0x8C, 0x74, 0x04, 0x78,	/* g */0x80, 0x80, 0xB8, 0xC4, 0x84, 0x84, 0x84, 0x00,	/* h */0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00,	/* i */0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x88, 0x70,	/* j */0x80, 0x80, 0x88, 0x90, 0xA0, 0xD0, 0x88, 0x00,	/* k */0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,	/* l */0x00, 0x00, 0xE8, 0x54, 0x54, 0x54, 0x54, 0x00,	/* m */0x00, 0x00, 0xF8, 0x44, 0x44, 0x44, 0x44, 0x00,	/* n */0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00,	/* o */0x00, 0x00, 0xB8, 0xC4, 0xC4, 0xB8, 0x80, 0x80,	/* p */0x00, 0x00, 0x74, 0x8C, 0x8C, 0x74, 0x04, 0x04,	/* q */0x00, 0x00, 0xB8, 0xC4, 0x80, 0x80, 0x80, 0x00,	/* r */0x00, 0x00, 0x7C, 0x80, 0x78, 0x04, 0xF8, 0x00,	/* s */0x20, 0x20, 0xF8, 0x20, 0x20, 0x24, 0x18, 0x00,	/* t */0x00, 0x00, 0x84, 0x84, 0x84, 0x8C, 0x74, 0x00,	/* u */0x00, 0x00, 0x84, 0x84, 0x84, 0x48, 0x30, 0x00,	/* v */0x00, 0x00, 0x44, 0x44, 0x54, 0x54, 0x6C, 0x00,	/* w */0x00, 0x00, 0x84, 0x48, 0x30, 0x48, 0x84, 0x00,	/* x */0x00, 0x00, 0x84, 0x84, 0x8C, 0x74, 0x04, 0x78,	/* y */0x00, 0x00, 0xFC, 0x08, 0x30, 0x40, 0xFC, 0x00,	/* z */0x38, 0x40, 0x40, 0xC0, 0x40, 0x40, 0x38, 0x00,	/* { */0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,	/* | */0xE0, 0x10, 0x10, 0x18, 0x10, 0x10, 0xE0, 0x00,	/* } */0x00, 0x62, 0xD6, 0x8C, 0x00, 0x00, 0x00, 0x00,	/* ~ */0xAA, 0x54, 0xAA, 0x54, 0xAA, 0x54, 0xAA, 0x54,	/* . */};