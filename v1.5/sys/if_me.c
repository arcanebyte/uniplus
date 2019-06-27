

/*
 * Structure of an Ethernet header.
 *
 */

#include "net/misc.h"
#include "sys/param.h"
#include "sys/config.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "net/mbuf.h"
#include "sys/buf.h"
#include "net/protosw.h"
#include "net/socket.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "net/ubavar.h"
#include "errno.h"

#include "net/if.h"
#include "net/route.h"
#include "net/in.h"
#include "net/in_systm.h"
#include "net/ip.h"
#include "net/ip_var.h"
#include "net/pup.h"
#include "net/if_ether.h"

/*
 * 3Com Ethernet controller registers.
 */
struct medevice {
	u_short	me_csr;		/* control and status */
	u_short	me_back;	/* backoff value */
	u_char	me_pad1[0x400-2*2];
	u_char	me_arom[6];	/* address ROM */
	u_char	me_pad2[0x200-6];
	u_char	me_aram[6];	/* address RAM */
	u_char	me_pad3[0x200-6];
	u_char	me_tbuf[2048];	/* transmit buffer */
	u_char	me_abuf[2048];	/* rmeeive buffer A */
	u_char	me_bbuf[2048];	/* receive buffer B */
};

/*
 * Control and status bits
 */
#define	ME_BBSW		0x8000		/* buffer B belongs to ether */
#define	ME_ABSW		0x4000		/* buffer A belongs to ether */
#define	ME_TBSW		0x2000		/* transmit buffer belongs to ether */
#define	ME_JAM		0x1000		/* Ethernet jammed (collision) */
#define	ME_AMSW		0x0800		/* address RAM belongs to ether */
#define	ME_RBBA		0x0400		/* buffer B older than A */
#define	ME_RESET	0x0100		/* reset controller */
#define	ME_BINT		0x0080		/* buffer B interrupt enable */
#define	ME_AINT		0x0040		/* buffer A interrupt enable */
#define	ME_TINT		0x0020		/* transmitter interrupt enable */
#define	ME_JINT		0x0010		/* jam interrupt enable */
#define	ME_PAMASK	0x000f		/* PA field */

/* with old VAX 4.1a, keep range errors, since vax sends too-small packets. */
#define	ME_PA_OLDVAX	0x0008		/* receive mine+broadcast-(fcs+frame)*/
#define	ME_PA		0x0002		/*  all- fcs and frame errors  */

/*
 * Receive status bits
 */
#define	ME_FCSERR	0x8000		/* FCS error */
#define	ME_BROADCAST	0x4000		/* packet was broadcast packet */
#define	ME_RGERR	0x2000		/* range error */
#define	ME_ADDRMATCH	0x1000		/* address match */
#define	ME_FRERR	0x0800		/* framing error */
#define	ME_DOFF		0x07ff		/* first free byte */

#define	MERDOFF		2		/* packet offset in read buffer */
#ifdef notdef
#define	MEMAXTDOFF	(2048-60)	/* max packet offset (min size) */
#endif
#define	MEMAXTDOFF	(2048-512)	/* max packet offset (min size) */

#define NME 1

/*
 * 3Com Ethernet Controller interface
 */

#define	MEMTU	1500

int nulldev(), meattach(), meintr();
struct	uba_device *meinfo[NME];


struct	uba_driver medriver = {
	nulldev, meattach, (u_short * )0, meinfo
};

#define	MEUNIT(x)	minor(x)

int	meinit(),meoutput(),mewatch();
struct mbuf *meget();

extern struct ifnet loif;

#ifdef	HAS8259
#define ME_EOI 2
int meeoi = ME_EOI;
#endif

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * es_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct	me_softc {
	struct	arpcom es_ac;		/* common Ethernet structures */
#define	es_if	es_ac.ac_if		/* network-visible interface */
#define	es_enaddr es_ac.ac_enaddr	/* hardware Ethernet address */
	short	es_mask;		/* mask for current output delay */
	short	es_oactive;		/* is output active? */
} me_softc[NME];


/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
meattach(md)
	struct uba_device *md;
{
	struct me_softc *es = &me_softc[md->ui_unit];
	register struct ifnet *ifp = &es->es_if;
	register struct medevice *addr;
	struct sockaddr_in *sin;
	int i;
	u_char *cp, *ap;
	char *memap();

	/* map controller into kernel space for Unisoft. memap is in config.c */
	addr = (struct medevice *) memap((int)md->ui_addr, btoc(8192));
	/* replace the value of ui_addr for everyone else. */
	md->ui_addr = (caddr_t)addr;

	ifp->if_unit = md->ui_unit;
	ifp->if_name = "me";
	ifp->if_mtu = MEMTU;
	ifp->if_net = md->ui_flags & 0xff000000;

	if (!iocheck((caddr_t)addr)) {
printf("**\nCould not find me%d; am not initializing network device...**\n",
			ifp->if_unit);
		return;
	}

	addr->me_csr |= ME_RESET;	/* reset the board */
	medelay(1);			/* wait for it to reset (1 sec) */
	/*
	 * Read the ethernet address off the board, one byte at a time.
	 */
	cp = es->es_enaddr;
	ap = addr->me_arom;

	/* check for mem. board mistakenly mapped to same addr. as me */
	if ((*ap = 'g') == 'g') {
		printf (
"\n**Able to write the rom on me%d; probable memory addressing confilct.**\nNot initializing interface.\n\n", ifp->if_unit);
		return;
	}

	for (i = 0; i < 6; i++)
		*cp++ = *ap++;
	printf("Ethernet address = ");
	{
		char * p = &es->es_enaddr[0];
		int i, j = 0;
		char buf[14];

		for (i = 0; i < 6; i ++) {
			buf[j++] = "0123456789ABCDEF"[((*p >> 4)&0xf)];
			buf[j++] = "0123456789ABCDEF"[((*p++)&0xf)];
		}
		buf[j++] = '\n';
		buf[j] = 0;
		printf(buf);
	}
	sin = (struct sockaddr_in *)&es->es_if.if_addr;
	sin->sin_family = AF_INET;

	/* this is a way to set addresses for now (without an ioctl()) */
	sin->sin_addr.s_addr = (u_long)md->ui_flags;
	mesetaddr(ifp, sin);

	ifp->if_init = meinit;
	ifp->if_output = meoutput;
	ifp->if_watchdog = mewatch;
	if_attach(ifp);
}

mewatch()
{}

/*
 * Initialization of interface; clear recorded pending
 * operations.
 */
meinit(unit)
	int unit;
{
	struct me_softc *es = &me_softc[unit];
	register struct ifnet *ifp = &es->es_if;
	register struct sockaddr_in *sin;
	struct medevice *addr;
	int i, s;
	u_char *cp, *ap;

	sin = (struct sockaddr_in *)&ifp->if_addr;
	if (sin->sin_addr.s_addr == 0)		/* address still unknown */
	    return;
	if ((es->es_if.if_flags & IFF_RUNNING) == 0) {
		addr = (struct medevice *)meinfo[unit]->ui_addr;
		s = splimp();
		/*
		 * Initialize the address RAM
		 */
		cp = es->es_enaddr;
		ap = addr->me_aram;
		for (i = 0; i < 6; i++)
			*ap++ = *cp++;
		addr->me_csr |= ME_AMSW;

		/*
		 * Hang receive buffers and start any pending writes.
		 */
		addr->me_csr |= ME_ABSW|ME_AINT|ME_BBSW|ME_BINT
			| ME_PA_OLDVAX;
		es->es_oactive = 0;
		es->es_mask = ~0;
		es->es_if.if_flags |= IFF_UP|IFF_RUNNING;
		if (es->es_if.if_snd.ifq_head)
			mestart(unit);
		splx(s);
	}
	if_rtinit(&es->es_if, RTF_UP);
	arpattach(&es->es_ac);
	arpwhohas(&es->es_ac, &sin->sin_addr);
}

/*
 * Start or restart output on interface.
 * If interface is already active, then this is a retransmit
 * after a collision, and just restuff registers.
 * If interface is not already active, get another datagram
 * to send off of the interface queue, and map it to the interface
 * before starting the output.
 */
mestart(dev)
	register dev_t dev;
{
        register int unit = MEUNIT(dev);
	register struct me_softc *es = &me_softc[unit];
	register struct medevice *addr;
	register struct mbuf *m;

	addr = (struct medevice *)meinfo[unit]->ui_addr;
	if (es->es_oactive)
		goto restart;

	IF_DEQUEUE(&es->es_if.if_snd, m);
	if (m == 0) {
		es->es_oactive = 0;
		return;
	}
	meput(addr->me_tbuf, m);
	addr->me_csr |= ME_TBSW|ME_TINT|ME_JINT;

restart:
	es->es_oactive = 1;
}

/*
 * Ethernet interface interrupt.
 * If received packet examine 
 * packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
meintr()
{
	register struct me_softc *es;
	register struct medevice *addr;
	register int unit;
	extern short netoff;
	register unsigned short meenables = 0;

	if (netoff)
		return;
	(void) splnet();
	for (unit = 0; unit < NME; unit++) {
	es = &me_softc[unit];
	addr = (struct medevice *)meinfo[unit]->ui_addr;
	switch (addr->me_csr & (ME_ABSW|ME_BBSW|ME_RBBA)) {
	case ME_ABSW:
	case ME_ABSW|ME_RBBA:
		/* BBSW == 0, receive B packet */
		addr->me_csr &= ~ME_BINT;
		meread(es, addr->me_bbuf);
		addr->me_csr |= ME_BBSW|ME_BINT;
		break;

	case ME_BBSW:
	case ME_BBSW|ME_RBBA:
		/* ABSW == 0, rmeeive A packet */
		addr->me_csr &= ~ME_AINT;
		meread(es, addr->me_abuf);
		addr->me_csr |= ME_ABSW|ME_AINT;
		break;

	case ME_RBBA:
		/* ABSW == 0, BBSW == 0, RBBA, receive B, then A */
		addr->me_csr &= ~(ME_AINT|ME_BINT);
		meread(es, addr->me_bbuf);
		addr->me_csr |= ME_BBSW|ME_BINT;
		meread(es, addr->me_abuf);
		addr->me_csr |= ME_ABSW|ME_AINT;
		break;

	case 0:
		/* ABSW == 0, BBSW == 0, RBBA == 0, receive A, then B */
		addr->me_csr &= ~(ME_AINT|ME_BINT);
		meread(es, addr->me_abuf);
		addr->me_csr |= ME_ABSW|ME_AINT;
		meread(es, addr->me_bbuf);
		addr->me_csr |= ME_BBSW|ME_BINT;
		break;

	case ME_ABSW|ME_BBSW:
	case ME_ABSW|ME_BBSW|ME_RBBA:
		/* no input packets */
		addr->me_csr &= ~(ME_AINT|ME_BINT);
		addr->me_csr |= ME_AINT|ME_BINT;
		break;

	default:
		panic("meintr: impossible value");
		/* NOT REACHED */
	}
	} /* end of "for unit" */

	for (unit = 0; unit < NME; unit++) {
	es = &me_softc[unit];
	addr = (struct medevice *)meinfo[unit]->ui_addr;

	if (es->es_oactive == 0)
		continue;
	if (addr->me_csr & ME_JAM) {
		/*
		 * Collision on ethernet interface.  Do exponential
		 * backoff, and retransmit.  If have backed off all
		 * the way print warning diagnostic, and drop packet.
		 */
		es->es_if.if_collisions++;
		medocoll(unit);
		continue;
	}
	if ((addr->me_csr & ME_TBSW) == 0) {
		addr->me_csr &= ~(ME_TINT|ME_JINT);
		es->es_if.if_opackets++;
		es->es_oactive = 0;
		es->es_mask = ~0;
		if (es->es_if.if_snd.ifq_head)
			mestart(unit);
	}
	} /* end of "for unit" */

#ifdef HAS8259
	/* try to force level change by exciting current ints on bd */
	meenables = addr->me_csr & (ME_TINT|ME_AINT|ME_BINT|ME_JINT);
	addr->me_csr &= ~meenables;
	sendeoi(meeoi);
	addr->me_csr |= meenables;
#endif

	return ;
}

meread(es, mebuf)
	register struct me_softc *es;
	register caddr_t mebuf;
{
	register struct ether_header *me;
    	register struct mbuf *m;
	register int len, off, meoff;
	short resid;
	register struct ifqueue *inq;
/*
int i;

printf("mer.");
me = (struct ether_header *)(mebuf + MERDOFF);
printf("s:");
for (i = 0; i < 6; i++)
    printf("%x.",me->me_shost[i]&0xff);
printf("  ");
printf("d:");
for (i = 0; i < 6; i++)
    printf("%x.",me->me_dhost[i]&0xff);
*/

	es->es_if.if_ipackets++;
	meoff = *(short *)mebuf;

	if(meoff & (ME_FRERR|ME_RGERR|ME_FCSERR))
		    goto err;

	meoff &= ME_DOFF;
	
	if (meoff <= MERDOFF || meoff > 2046) {
err:
		es->es_if.if_ierrors++;
		return;
	}

	/*
	 * Get input data length.
	 * Get pointer to ethernet header (in input buffer).
	 * Deal with trailer protocol: if type is PUP trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	len = meoff - MERDOFF - sizeof (struct ether_header) - 4; /* 4 == FCS */
	me = (struct ether_header *)(mebuf + MERDOFF);
#define	medataaddr(me, off, type)	((type)(((caddr_t)((me)+1)+(off))))

	if (me->ether_type >= ETHERPUP_TRAIL &&
	    me->ether_type < ETHERPUP_TRAIL+ETHERPUP_NTRAILER) {
		off = (me->ether_type - ETHERPUP_TRAIL) * 512;
		if (off >= MEMTU)
			return;		/* sanity */
		me->ether_type = *medataaddr(me, off, u_short *);
		resid = *(medataaddr(me, off+2, u_short *));
		if (off + resid > len)
			return;		/* sanity */
		len = off + resid;
	} else
		off = 0;
	if (len == 0)
		return;

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; meget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	m = meget(mebuf, len, off);
	if (m == 0)
		return;
	if (off) {
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}
	switch (me->ether_type) {

#ifdef INET
	case ETHERPUP_IPTYPE:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERPUP_ARPTYPE:
		arpinput(&es->es_ac, m);
		return;
#endif
	default:
		m_freem(m);
		return;
	}

	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
		return;
	}
	IF_ENQUEUE(inq, m);
}

medocoll(unit)
	int unit;
{
	register struct me_softc *es = &me_softc[unit];
	register struct medevice *addr =
	    (struct medevice *)meinfo[unit]->ui_addr;
	int delay;

	/*
	 * Es_mask is a 16 bit number with n low zero bits, with
	 * n the number of backoffs.  When es_mask is 0 we have
	 * backed off 16 times, and give up.
	 */
	if (es->es_mask == 0) {
		es->es_if.if_oerrors++;
		printf(
"\nme%d: 16 collisions detected on ethernet.  Dropping current packet...\n\n",
			unit);
		/*
		 * Reset and transmit next packet (if any).
		 */
		es->es_oactive = 0;
		es->es_mask = ~0;
		if (es->es_if.if_snd.ifq_head)
			mestart(unit);
		return;
	}
	/*
	 * Do exponential backoff.  Compute delay based on low bits
	 * of the time.  A slot time is 51.2 microseconds (rounded to 51).
	 * This does not take into account the time already used to
	 * process the interrupt.
	 */
	es->es_mask <<= 1;
	delay = (time&0xffff) &~ es->es_mask;
	addr->me_back = delay * 51;
	addr->me_csr |= ME_JAM|ME_JINT;
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 * If destination is this address or broadcast, send packet to
 * loop device to kludge around the fact that 3com interfaces can't
 * talk to themselves.
 */
meoutput(ifp, m0, dst)
	register struct ifnet *ifp;
	register struct mbuf *m0;
	register struct sockaddr *dst;
{
	int type,  s, error;
	u_char edst[6];
	struct in_addr idst;
	register struct me_softc *es = &me_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *me;
	register int i;
	struct mbuf *mcopy = (struct mbuf *) 0;		/* Null */

	s = splnet();
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&es->es_ac, m, &idst, edst))
			return (0);	/* if not yet resolved */
		if (in_lnaof(idst) == INADDR_ANY)
			mcopy = m_copy(m, 0, (int)M_COPYALL);
		type = ETHERPUP_IPTYPE;
		goto gottype;
#endif

	case AF_UNSPEC:
		me = (struct ether_header *)dst->sa_data;
		bcopy((caddr_t)me->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = me->ether_type;
		goto gottype;

	default:
		printf("me%d: can't handle af%d\n", ifp->if_unit,
			dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
	}

gottype:
	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	me = mtod(m, struct ether_header *);
	bcopy((caddr_t)edst, (caddr_t)me->ether_dhost, sizeof (edst));
	me->ether_type = htons((u_short)type);
	bcopy((caddr_t)es->es_enaddr, (caddr_t)me->ether_shost, 6);
	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		error = ENOBUFS;
		goto qfull;
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	if (es->es_oactive == 0)
		mestart(ifp->if_unit);
	splx(s);

gotlocal:
	return(mcopy ? looutput(&loif, mcopy, dst) : 0);

qfull:
	m0 = m;
bad:
	m_freem(m0);
	splx(s);
	return(error);
}

/*
 * Routine to copy from mbuf chain to transmitter
 * buffer in Multibus memory.
 */
meput(mebuf, m)
	register u_char *mebuf;
	register struct mbuf *m;
{
	register struct mbuf *mp;
	register short off;
	register u_char *bp;
	register flag = 0;

	for (off = 2048, mp = m; mp; mp = mp->m_next)
		off -= mp->m_len;
	if (off > MEMAXTDOFF)		/* enforce minimum packet size */
		off = MEMAXTDOFF;

	if (off & 01) {
		off--;
		flag++;
	}
    
	*(u_short *)mebuf = off;
	bp = (u_char *)(mebuf + off);
	for (mp = m; mp; mp = mp->m_next) {
		register unsigned len = mp->m_len;
		u_char *mcp;

		if (len == 0)
			continue;
		mcp = mtod(mp, u_char *);
		bcopy(mcp, bp, (int)len);
		bp += len;
	}

/*
if ((Off & 01) && (Off > (2048-70)))
{int i; int j; printf("meput(%x):\n", Off);for(j=0;j<4;j++){for(i=0;i<16;i++)printf("%x ",(((char *)(off+mebuf))[i+16*j])&0xff);printf("\n");}}
*/

	if (flag)
		*bp = 0;

	m_freem(m);
}

/*
 * Routine to copy from Multibus memory into mbufs.
 *
 * Warning: This makes the fairly safe assumption that
 * mbufs have even lengths.
 */
struct mbuf *
meget(mebuf, totlen, off0)
	register u_char *mebuf;
	register int totlen, off0;
{
	register struct mbuf *m;
	struct mbuf *top = 0, **mp = &top;
	register int off = off0, len;
	register u_char *cp;

	cp = mebuf + MERDOFF + sizeof (struct ether_header);
	/*
	{int i; int j; printf("meget:\n");for(j=0;j<6;j++){for(i=0;i<16;i++)printf("%x ",(((char *)cp)[i+16*j])&0xff);printf("\n");}}
	*/
	while (totlen > 0) {
		u_char *mcp;

		MGET(m, 0);
		if (m == 0){
			goto bad;
		}
		if (off) {
			len = totlen - off;
			cp = mebuf + MERDOFF + sizeof (struct ether_header) + off;
		} else
			len = totlen;
		m->m_len = len = MIN(MLEN, len);
		m->m_off = MMINOFF;
		mcp = mtod(m, u_char *);
		bcopy(cp, mcp, len);
		cp += len;
		*mp = m;
		mp = &m->m_next;
		if (off == 0) {
			totlen -= len;
			continue;
		}
		off += len;
		if (off == totlen) {
			cp = mebuf + MERDOFF + sizeof (struct ether_header);
			off = 0;
			totlen = off0;
		}
	}
	return (top);
bad:
	m_freem(top);
	return (0);
}

/* medelay -- wait about tim secs */
medelay(tim)
register tim;
{
	register i;

	while (tim--){
		i = 100000;
		while (i--)
			;
	}
}

mesetaddr(ifp, sin)
	register struct ifnet *ifp;
	register struct sockaddr_in *sin;
{

	ifp->if_addr = *(struct sockaddr *)sin;
	ifp->if_net = in_netof(sin->sin_addr);
	ifp->if_host[0] = in_lnaof(sin->sin_addr);
	sin = (struct sockaddr_in *)&ifp->if_broadaddr;
	sin->sin_family = AF_INET;
	sin->sin_addr = if_makeaddr(ifp->if_net, INADDR_ANY);
	ifp->if_flags |= IFF_BROADCAST;
}
