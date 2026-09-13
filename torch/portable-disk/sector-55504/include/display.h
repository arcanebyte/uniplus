#define  NUMHTONES 7    /* 4 */

#define  FLASHINT  15

#define  DISPLAY_INTERVAL 1 
#define  SLANT_INTERVAL   1
#define  SHADE_INTERVAL   6

#define  ICMIDWAY  20

#define  BANX      60
#define  BANY      10

#define  MAXLAYERS 10
#define  XSTEP     3

#define  XWIDTH   96
#define  XHEIGHT  48

#define  ALERTY   (BANY + BANHEIGHT - 25)
#define  ALERTWIDTH 270
#define  ALERTHEIGHT 95
#define  ALERTX   ((720 - ALERTWIDTH)>>1)
#define  ALERTROWWORDS (( ALERTWIDTH + 17) >> 4)

#define  XOFFSWIDTH  (XWIDTH + XSTEP*MAXLAYERS)
#define  XOFFSHEIGHT (XHEIGHT + MAXLAYERS)
#define  XOFFSROWWORDS ((XOFFSWIDTH + 15) >> 4)

#define  X1X      (220 + BANX - XSTEP*MAXLAYERS) 
#define  X1Y      (25  + BANY)

#define  X2X      (X1X + XOFFSWIDTH + 3)
#define  X2Y      (25  + BANY)

#define  X3X      (X2X + XOFFSWIDTH + 3)
#define  X3Y      (25  + BANY)

#define  BANWIDTH 600
#define  BANHEIGHT  (XHEIGHT + MAXLAYERS + 70)

#define  PSWDX    30
#define  PSWDY    168
#define  PSWDLEN  8

#define  PLETTERWID  16
#define  PLETTERHEI  16

#define  PSWDBOXX    PSWDX
#define  PSWDBOXY    PSWDY + 15
#define  PSWDBOXWID  (PSWDLEN*PLETTERWID)
#define  PSWDBOXHEI  PLETTERHEI

#define  LOWFREE     0x1a0000
#define  LIMITFREE   0x1f8000
#define  HIGHFREE    0x1fffff

#define  CORRUPTED   5

#define  XXXTX  (BANX + 20)
#define  XXXTY  (BANY + 37)

#define  XXXrX  (XXXTX + 71)
#define  XXXrY  (XXXTY + 21)

#define  XXXiX  (XXXrX + 50)
#define  XXXiY  (XXXTY + 5)

#define  XXXpX  (XXXiX + 18)
#define  XXXpY  (XXXTY + 12)

#define  XXXlX  (XXXpX + 71)
#define  XXXlY  (XXXTY - 3)

#define  XXXeX  (XXXlX + 33)
#define  XXXeY  (XXXTY + 19)

#define  XXXXX  (XXXeX + 31)
#define  XXXXY  (XXXTY + 3)

typedef struct {

  char flag, count, onfront;
  rectangle *frect;
  addrptr   front, back;
} flashstruct;


struct iconhdr {
  
  int   checksum;
  short x, y;
  short width, height;
  short rowwords;
  short format;
  short planes;
};


struct iconentry {

  char *name;
  unsigned short inumber;
  form **formpp;
  word **bitspp;
};


#ifdef OVERRIDE

#define BYPASS  999

#endif OVERRIDE

