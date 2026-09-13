/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Window Library Error Code User Include File              *
 *                                                                            *
 * COPYRIGHT : (c) 1986 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) werrno.h 1.5@(#)" */

/* offset for first eioctl call type error message for use with WLIBSYSERR */
/* error number formed by (WEM_IOCTLOFFSET + EINWIND) etc.                 */

#define	WEM_IOCTLOFFSET		1000

/* error message numbers for use with WLIBERR */

#define	WEM_ACTCTRL		2000
#define	WEM_AUTODRAW		2001
#define	WEM_CLCTRL		2002
#define	WEM_CTRLEVENT		2003
#define	WEM_DEACTCTRL		2004
#define	WEM_DEFCTRLTYPE		2005
#define	WEM_DISCTRL		2006
#define	WEM_DODRAG		2007
#define	WEM_DRAWCTRL		2008
#define	WEM_DRAWSCROLL		2009
#define	WEM_ENCTRL		2010
#define	WEM_FRAMECTRL		2011
#define	WEM_GETCACTR		2012
#define	WEM_GETCLABEL		2013
#define	WEM_GETCLIMIT		2014
#define	WEM_GETCPATS		2015
#define	WEM_GETCPOS		2016
#define	WEM_GETCSTATE		2017
#define	WEM_GETCTACTFNS		2018
#define	WEM_GETCTDRAWFN		2019
#define	WEM_GETCTPATS		2020
#define	WEM_GETCTUPDATEFNS	2021
#define	WEM_GETCVALUE		2022
#define	WEM_GETHSPROP		2023
#define	WEM_NOAUTODRAW		2024
#define	WEM_GETPAD		2025
#define	WEM_GETSCID		2026
#define	WEM_GETWAEFLAGS		2027
#define	WEM_HIDECTRL		2028
#define	WEM_INITAUTO		2029
#define	WEM_KBOFF		2030
#define	WEM_KBON		2031
#define	WEM_MAKEMENU		2032
#define	WEM_MOVEPAD		2033
#define	WEM_OPCTRL		2034
#define	WEM_PPOFF		2035
#define	WEM_PPON		2036
#define	WEM_PUTCACTR		2037
#define	WEM_PUTCLABEL		2038
#define	WEM_PUTCLIMIT		2039
#define	WEM_PUTCPATS		2040
#define	WEM_PUTCPOS		2041
#define	WEM_PUTCSTATE		2042
#define	WEM_PUTCVALUE		2043
#define	WEM_PUTHSPROP		2044
#define	WEM_PUTPAD		2045
#define	WEM_PUTVSPROP		2046
#define	WEM_SETWAEFLAGS		2047
#define	WEM_SETWINDOW		2048
#define	WEM_SHOWCTRL		2049
#define	WEM_UFRAMECTRL		2050
#define	WEM_UNDEFCTRLTYPE	2051
#define	WEM_WCLOSE		2052
#define	WEM_WHIDE		2053
#define	WEM_WOPEN		2054
#define	WEM_WSHOW		2055
#define	WEM_DOGROW		2056
#define	WEM_EVENTSON		2057
#define	WEM_GETCFORM		2058
#define	WEM_PUTCFORM		2059
#define	WEM_GETCTFLAGS		2060

/* reasons for failure - window library specific - for use with WLIBERR */

#define	WER_CORRUPT		1
#define	WER_INVALID		2
#define	WER_INVCTRL		3
#define	WER_INVCTYPE		4
#define	WER_NOCURWIN		5
#define	WER_NOEVENT		6
#define	WER_NOPAD		7
#define	WER_TYPEINUSE		8
#define	WER_EVENTSON		9
