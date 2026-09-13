#! /bin/sh
#
# _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.1
# _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/16 Copyr 1985 Sun Micro
# sccsid = @(#)ypxfr_1d.sh	UniPlus V.2.1.1 (Sun 2.1)
#
# ypxfr_1perday.sh - Do daily yp map check/updates
#

# set -xv
/etc/yp/ypxfr group.byname
/etc/yp/ypxfr group.bygid 
/etc/yp/ypxfr protocols.byname
/etc/yp/ypxfr protocols.bynumber
/etc/yp/ypxfr networks.byname
/etc/yp/ypxfr networks.byaddr
/etc/yp/ypxfr services.byname
/etc/yp/ypxfr ypservers
