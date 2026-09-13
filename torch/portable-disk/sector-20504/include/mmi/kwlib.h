/*****************************************************************************
 * INCLUDE FILE FOR WINDOW LIBRARY (wlib.a)                                  *
 * ========================================                                  *
 *                                                                           *
 * Written by C.Manning, 31-May-85.                                          *
 *                                                                           *
 *  Update history:                                                          *
 *                                                                           *
 *  WHAT                                      BY WHOM     WHEN               *
 * -------------------------------------------|------|-----------------      *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/

#include <termio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <wlib.h>
#include <sys/types.h>
#include <sys/stat.h>


#define IOCTL(f, c, p)	ioctl(f, ('J' << 8) | (c), p)

#define ReadDelay(p)		IOCTL (fd, ERPALDEL, p)
#define WriteDelay(p)   	IOCTL (fd, EWPALDEL, p)
#define ReadPalette(p)		IOCTL (fd, ERLCPAL, p)
#define WritePalette(p) 	IOCTL (fd, EWLCPAL, p)
#define WriteScreenMode(p)	IOCTL (fd, EWSMODE, p)
#define ReadScreenMode(p)	IOCTL (fd, ERSMODE, p)
#define OpenBitMap(brptr)	IOCTL (fd, EOBMAP, brptr)
#define CloseBitMap(brnum) 	{Flush ();IOCTL (fd, ECBMAP, brnum);}
#define GetWAreaState(p) 	IOCTL (fd, EGWAREAS, p)
#define SetWAreaState(p) 	IOCTL (fd, ESWAREAS, p)
#define GetWModState(p)  	IOCTL (fd, EGWMODS, p)
#define SetWModState(p)  	IOCTL (fd, ESWMODS, p)
#define DrawTitle()      	IOCTL (fd, EDRAWTITLE, 0)
#define ChangeWindow(p)  	IOCTL (fd, ECHWIND, p)
#define GetEdState(p)    	IOCTL (fd, EGEDSTAT, p)
#define ReadClipRect(p)  	IOCTL (fd, ERUCLIP, p)

#define NewMenu(p)       	IOCTL (fd, ENEWMENU, p)
#define DisposeMenu(h)   	IOCTL (fd, EDISMENU, h)
#define DeleteMenu(p)    	IOCTL (fd, EDELMENU, p)
#define ClearMBar()      	IOCTL (fd, ECLRMBAR, 0)
#define MenuSelect(p)    	IOCTL (fd, EMENUSEL, p)
#define InitMenSel(p)    	IOCTL (fd, EINITMS, p)
#define SetBRect(p)      	IOCTL (fd, EBRECT, p)

#define GCurState(p)    	IOCTL (fd, EGCARSTAT, p)
#define GPenState(p)    	IOCTL (fd, EGPENS, p)
#define GPatState(p)    	IOCTL (fd, EGPATS, p)
#define GFontState(p)   	IOCTL (fd, EGFONTS, p)
#define RdBeep(p)       	IOCTL (fd, ERBEEP, p)
#define WtBeep(p)       	IOCTL (fd, EWBEEP, p)
#define WtDefKey(p)     	IOCTL (fd, EWDEFK, p)
#define RdCurPos(p)     	IOCTL (fd, ERPTR, p)
#define WtCurPos(p)     	IOCTL (fd, EWPTR, p)
#define HideMCurs()     	IOCTL (fd, EHPTR, 0)
#define ShowMCurs()     	IOCTL (fd, ESPTR, 0)
#define ChangeCursor(p) 	IOCTL (fd, ECHCUR, p)
#define RdCSpace(p)     	IOCTL (fd, ERSTHRESH, p)
#define WtCSpace(p)     	IOCTL (fd, EWSTHRESH, p)
#define RdCTime(p)      	IOCTL (fd, ERTTHRESH, p)
#define WtCTime(p)      	IOCTL (fd, EWTTHRESH, p)
