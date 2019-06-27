/* data and code stubs for loading witout network */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "net/misc.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "net/socketvar.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "net/ubavar.h"
#include "sys/map.h"
#include "net/if.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "sys/var.h"

/* data */
struct protosw protosw[1];
char netstak[3000];
char * svstak;
int ifnetslow;
int protofast;
int protoslow;
short netoff;
int netisr;
extern int ptc_dev;
extern int selwait;
extern u_short ip_id;
extern struct ipq ipq;
extern struct ipstat ipstat;
extern struct ifqueue rawintrq;
extern struct uba_device ubdinit[];
extern struct protosw *protoswLAST;
#ifdef INET
extern struct ifqueue ipintrq;
#endif

/* routines */
ssocket()
{
#ifdef lint
	ifnet++;
	ifnetslow++;
	protofast++;
	protoslow++;
	ptc_dev++;
	selwait++;
	ip_id++;
	ipq.ipq_ttl++;
	*protoswLAST++;
	ubdinit[0].ui_unit++;
	ipintrq.ifq_len++;
	rawintrq.ifq_len++;
	protosw[0].pr_type++;
	ipstat.ips_toosmall++;
#endif
	u.u_error = ENETDOWN;
}
netintr()
{
	netisr = 0;
}
/*ARGSUSED*/
soclose(so, exiting) struct socket *so; int exiting;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
soreceive(so, asa) struct socket *so; struct sockaddr *asa;
{
	return(0);
}
/*ARGSUSED*/
sosend(so, asa) struct socket *so; struct sockaddr *asa;
{
	return(0);
}
/*ARGSUSED*/
sostat(so, sb) struct socket *so; struct stat *sb;
{
	return(0);
}
/*ARGSUSED*/
soioctl(so, cmd, cmdp) struct socket *so; int cmd; caddr_t cmdp;
{
	u.u_error = ENETDOWN;
}
sconnect()
{
	u.u_error = ENETDOWN;
}
ssend()
{
	u.u_error = ENETDOWN;
}
ssockad()
{
	u.u_error = ENETDOWN;
}
saccept()
{
	u.u_error = ENETDOWN;
}
netreset()
{
}
sethostname()
{
	u.u_error = ENETDOWN;
}
gethostname()
{
	u.u_error = ENETDOWN;
}
select()
{
	u.u_error = ENETDOWN;
}
sreceive()
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptswrite(dev) dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptcwrite(dev) dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptsioctl(dev, cmd, addr, flag) caddr_t addr; dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptcioctl(dev, cmd, addr, flag) caddr_t addr; dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptsopen(dev, flag) dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptcopen(dev, flag) dev_t dev; int flag;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptsread(dev) dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptcread(dev) dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptsclose(dev) dev_t dev;
{
	u.u_error = ENETDOWN;
}
/*ARGSUSED*/
ptcclose(dev) dev_t dev;
{
	u.u_error = ENETDOWN;
}
netinit()
{
}

#ifdef _NOTDEF
/* reference fnetlocal.c for inserting null interrupt associated stuff */
FAKE()
{
	fnetlocal();
}
#endif _NOTDEF
