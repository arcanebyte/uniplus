#! /bin/sh
#
# _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.1
# _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/16 Copyr 1985 Sun Micro
# sccsid = @(#)ypxfr_2d.sh	UniPlus V.2.1.1 (Sun 2.1)
#
# ypxfr_2perday.sh - Do twice-daily yp map check/updates
#

# set -xv
/etc/yp/ypxfr hosts.byname
/etc/yp/ypxfr hosts.byaddr
/etc/yp/ypxfr ethers.byaddr
/etc/yp/ypxfr ethers.byname
/etc/yp/ypxfr netgroup
/etc/yp/ypxfr netgroup.byuser
/etc/yp/ypxfr netgroup.byhost
/etc/yp/ypxfr mail.aliases 
