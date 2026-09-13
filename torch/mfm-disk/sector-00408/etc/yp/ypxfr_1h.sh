#! /bin/sh
#
# _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.1
# _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/16 Copyr 1985 Sun Micro
# sccsid = @(#)ypxfr_1h.sh	UniPlus V.2.1.1 (Sun 2.1)
#
# ypxfr_1perhour.sh - Do hourly yp map check/updates
#

# set -xv
/etc/yp/ypxfr passwd.byname
/etc/yp/ypxfr passwd.byuid 
