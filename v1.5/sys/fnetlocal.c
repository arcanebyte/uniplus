/* local defs for no network */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/var.h"
#include "errno.h"

#include "net/misc.h"
#include "net/mbuf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/ubavar.h"

/* causes make to complain so this module gets included. "Called" from fnet.c */
fnetlocal(){}

/* ..driver structure */
struct uba_driver ebdriver;

/* interrupt routine */
ebintr(){}
