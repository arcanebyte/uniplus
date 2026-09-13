/* _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.1 */
/* _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/14 */
/* sccsid = @(#)yppasswd.h	UniPlus V.2.1.1 (Sun 2.1) */

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#define YPPASSWDPROG 100009
#define YPPASSWDPROC_UPDATE 1
#define YPPASSWDVERS_ORIG 1
#define YPPASSWDVERS 1

struct yppasswd {
	char *oldpass;		/* old (unencrypted) password */
	struct passwd newpw;	/* new pw structure */
};

int xdr_yppasswd();
