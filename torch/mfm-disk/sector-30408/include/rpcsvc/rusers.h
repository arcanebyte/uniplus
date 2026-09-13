/* _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.1 */
/* _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/14 */
/* sccsid = @(#)rusers.h	UniPlus V.2.1.1 (Sun 2.1) */

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#define RUSERSPROC_NUM 1
#define RUSERSPROC_NAMES 2
#define RUSERSPROC_ALLNAMES 3
#define RUSERSPROG 100002
#define RUSERSVERS_ORIG 1
#define RUSERSVERS_IDLE 2
#define RUSERSVERS 2

#define MAXUSERS 100

struct utmparr {
	struct utmp **uta_arr;
	int uta_cnt
};

struct utmpidle {
	struct utmp ui_utmp;
	unsigned ui_idle;
};

struct utmpidlearr {
	struct utmpidle **uia_arr;
	int uia_cnt
};

int xdr_utmparr();
int xdr_utmpidlearr();
