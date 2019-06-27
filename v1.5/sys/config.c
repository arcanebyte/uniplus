/*
 * This file contains
 *	1. oem modifiable configuration personality parameters
 *	2. oem modifiable system specific kernel personality code
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/mmu.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/reg.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/termio.h"

#include "sys/conf.h"
#include "sys/cops.h"
#include "sys/pport.h"
#include "sys/local.h"
#include "sys/l2.h"
#include "sys/kb.h"
#include "sys/swapsz.h"

/*char oemmsg[] =	"UniSoft Systems distribution system release 1.5";*/
char oemmsg[] =	"UniSoft Systems pre-distribution system (release 1.5+)";

int	sspeed = B9600;		/* default console speed */
int	parityno = 28;		/* parity interrupt vector */
int	cmask = CMASK;		/* default file creation mask */
int	cdlimit = CDLIMIT;	/* default file size limit */
char	slot[NSLOTS];		/* card ID numbers for expansion cards */

/*
 * Kernel initialization functions.
 * Called from main.c while at spl7 in the kernel.
 */
oem7init()	/* alias (formerly) "lisainit" */
{
#ifdef SUNIX
	int dev;
	extern dev_t swapdev;
	extern int nswap;
#endif SUNIX
	extern struct rtime rtime;
	extern int pmvect[];
	extern int tevect[];
	register short *sidp;		/* slot ID pointer */
	register slotid, i;
	register long *ip;

	l2init();			/* setup the COPS ports */

	/* This mess disables the verticle retrace interrupt, for now.
	 */
	do {
		VRON = 1;
	} while ((STATUS & S_VR) != 0);
	do {
		VROFF = 1;
	} while ((STATUS & S_VR) == 0);

	/* Some of the initialization requires that interrupts be enabled to
	 * pick up coded sequences from the keyboard cops.  If interrupts were
	 * masked out then the time returned by READCLOCK would fill the
	 * buffer and KBENABLE, which also returns a value, would have trouble.
	 */
	SPL1();				/* ok, do it to me */
	l2copscmd(MOUSEOFF);		/* shut off mouse interrupts */
	l2copscmd(READCLOCK);		/* get time of day */
	l2copscmd(KBENABLE);		/* enable keyboard */

	sninit();			/* Sony initialization */

	/* Wait 'til the clock data (from READCLOCK) and keyboard ID (from 
	   KBENABLE) have come in, and the keyboard is back in NORMALWAIT */
	while (kb_state);
	time = rtime.rt_tod;

	SPL7();				/* it should be at level 7 for the rest (?) */

	/* Find out what's in each of the expansion slots.
	 */
	for (i = 0, sidp = SLOTIDS; i < NSLOTS; i++, sidp++) {
		slot[i] = 0xFF;			/* not supported */
		slotid = *sidp & SLOTMASK;
		if (!slotid) {
			if (iocheck((caddr_t)(STDIO+i*0x4000+1))) {
				printf("Expansion slot %d: quad serial card\n",
					i+1);
				if (teinit(i) == 0) {
					/*
					 * point to interrupt vector,
					 * set tecmar quad serial board inter loc,
					 * and initialize hdwr
					 */
					ip = &((long *) 0)[EXPIVECT+devtoslot(i)];
					*ip = (long)tevect + (long)(devtoslot(i)<<2);
				}
			}
			continue;
		}
		printf("Expansion slot %d: ", i+1);
		switch (slotid) {
		case ID_APLNET:
			printf("applenet card\n");
			break;
		case ID_PRO:
			printf("ProFile card\n");
			break;
		case ID_2PORT:
			printf("two port card\n");
			slot[i] = PR0;		/* valid */
			break;
		case ID_PRIAM:
			printf("Priam card\n");
			ip = &((long *) 0)[EXPIVECT+devtoslot(i)];	/* point to int vector */
			*ip = (long)pmvect + (long)(devtoslot(i)<<2);	/* set to Priam intr */
			if (pmcinit(i) == 0)	/* initialize controller */
				slot[i] = PM3;	/* valid */
			break;
		default:
			printf("card ID 0x%x\n", slotid);
		}
	}
	scinit();			/* SCC serial initialization */
#ifdef UCB_NET
	netinit();
#endif
	/* Now enable the verticle retrace interrupt, used for the system clock.
	 */
	do {
		VRON = 1;
	} while ((STATUS & S_VR) != 0);
#ifdef SUNIX
	SPL0();
	/* This is the first unix booted during installation so find swapdev. */
	if (rootdev == makedev(SN1, 0)) {
		while (chkdev(dev = getdevnam()))
			printf("Unable to use that device\nTry again:\n");
		printf("\n\nswapdev = 0x%x\n\n", dev);
		swapdev = dev;
		if (major(dev) == PR0) nswap = PRNSWAP;
		else if (major(dev) == PM3) nswap = PMNSWAP;
		else if (major(dev) == CV2) nswap = CVNSWAP;
		else panic("cannot determine size of swapdev");
	}
#endif SUNIX
}

/*
 * Kernel initialization functions.
 * Called from main.c while at spl0 in the kernel.
 */
oem0init()
{
}


/*
 * parityerror()
 *	Called from trap for parity error traps via
 *	interrupt level "parityno" (conf.c).
 *	Should return non-zero for fatal errors.
 *	Should return zero for a transient warning error.
 */
parityerror()
{
	printf("parity error\n");
	return(-1);
}

/*
 * reboot the system
 * called from reboot function
 */
doboot()
{
	kb_state = SHUTDOWN;	/* SHUTDOWN (see kb.c)*/
	SPL7();			/* extreme priority */
	rom_mon();		/* return to the ROM monitor */
	/*NOTREACHED*/
}

/*
 * OEM supplied subroutine called on process exit
 */
/* ARGSUSED */
oemexit(p)
register struct proc *p;
{
#ifdef lint
	/* for lint use p */
	p->p_flag++;
#endif
}

struct device_d *pro_da[NPPDEVS] = {		/* DEV	Description */
	PPADDR,					/* 0x00	parallel port */
	(struct device_d *)(STDIO+0x2000),	/* 0x10	FPC port 0 slot 1 */
	(struct device_d *)(STDIO+0x2800),	/* 0x20	FPC port 1 slot 1 */
	(struct device_d *)(STDIO+0x3000),	/* 0x30	FPC port 2 slot 1 !!!*/
	(struct device_d *)(STDIO+0x6000),	/* 0x40	FPC port 0 slot 2 */
	(struct device_d *)(STDIO+0x6800),	/* 0x50	FPC port 1 slot 2 */
	(struct device_d *)(STDIO+0x7000),	/* 0x60	FPC port 2 slot 2 !!!*/
	(struct device_d *)(STDIO+0xA000), 	/* 0x70	FPC port 0 slot 3 */
	(struct device_d *)(STDIO+0xA800),	/* 0x80	FPC port 1 slot 3 */
	(struct device_d *)(STDIO+0xB000)	/* 0x90	FPC port 2 slot 3 !!!*/
};
int (*pi_fnc[NPPDEVS])();	/* slots for interrupt handler addresses */

/* Set the interrupt handler for a given parallel port controller.
 */
setppint(addr, fnc)
struct device_d *addr;
int (*fnc)();
{
	register int i;
	extern int cvint(), prointr(), lpintr();

	for (i=0; i<NPPDEVS; i++)
		if (pro_da[i] == addr) {	/* found dev number */
			if (pi_fnc[i]) {	/* in use */
				if (pi_fnc[i] == fnc) /* same handler */
					return 0;
     				if (pi_fnc[i] == prointr)
					printf("ALREADY assigned to profile\n");
				else if (pi_fnc[i] == lpintr)
					printf("ALREADY assigned to lp\n");
				else if (pi_fnc[i] == cvint)
					printf("ALREADY assigned to corvus\n");
				else
					printf("Assigned to unknown handler at 0x%x\n",pi_fnc[i]);
				break;
			}
			pi_fnc[i] = fnc;
			return 0;
		}
	return 1;
}

/* Free the interrupt handler slot for a given controller.
 */
freeppin(addr)
	struct device_d *addr;
{
	register int i;

	for (i=0; i<NPPDEVS; i++)
		if (pro_da[i] == addr) {
			pi_fnc[i] = 0;
			return;
		}
}

/*
 * ppintr - handle interrupt from parallel port controllers
 */
ppintr(ap)
struct args *ap;
{
	register int i, j;
	register char a;
	register struct device_d *dp;
	int (*fnc)(), ebintr(), prointr(), cvint(), lpintr();
	extern char lpflg[];

	if((i = ap->a_dev) == 0) {		/* special case for pp 0 */
		if(fnc = pi_fnc[i]) {
			fnc(i);
			return;
		}
	}
	j = i + 2;
	while (i < j) {
		dp = pro_da[i];
		if ((a = dp->d_ifr) & FCA1) {
			asm("	nop ");
			dp->d_ifr = a;	/* reset interrupt */
			if (fnc = pi_fnc[i]) {
				if (fnc == lpintr)
					lpflg[i] = 0;
				else if (fnc != ebintr &&
					fnc != prointr &&
					fnc != cvint) {
					printf("pi_fnc[%d] = 0x%x invalid!!\n",
						i,fnc);
					return;
				}
				fnc(i);
				return;
			}
#ifdef INTDUMP
			ppdump(i,dp);
#endif INTDUMP
			return;
		}
		i++;
	}
}

#ifdef INTDUMP
ppdump(n, p)
register struct device_d *p;
{
	printf("pport %d: ",n);
	printf("ifr=%x acr=%x pcr=%x ddra=%x ddrb=%x irb=%x\n",
		p->d_ifr&0xFF, p->d_acr&0xFF, p->d_pcr&0xFF, p->d_ddra&0xFF,
		p->d_ddrb&0xFF, p->d_irb&0xFF);
}
#endif INTDUMP

/*
 *	called from clock if there's a panic in progress
 */
clkstop()
{
	VROFF = 1;		/* disable vertical retrace intr */
}

nmikey()
{
	int i;
	register short status;

	/* added 7/25/84 to provide more info than "NMI key".
	 * (taken from section 2.8 of Lisa Theory of Operations)
	 */
	printf("non-maskable interrupt: ");
	status = STATUS;
	if (status & S_SMEMERR)
		printf("soft memory error\n");
	else if (status & S_HMEMERR)
		printf("hard memory error\n");
	else
		printf("power failure/keyboard reset\n");
#ifdef HOWFAR
	showbus();
#endif HOWFAR
	for (i=0xC00000; i>0; i--) ;	/* delay */
}

#ifdef SUNIX
/* Get swap device name
 */
getdevnam ()
{
	char *p, *gets();
	int unit, dev;

retry:
	printf("\n\nWhere is the swap area?\n");
	printf("Enter:  'p' for builtin disk or a profile disk\n");
	printf("        'c' for Corvus disk\n");
	printf("        'pm' for Priam disk\n");
	p = gets();
	switch (p[0]) {
	case 'p':
		dev = PR0;
		if (p[1] == 'm')
			dev = PM3;
		break;
	case 'c':
		dev = CV2;
		break;
	default:
		printf("Invalid input. Try again.\n");
		goto retry;
	}
	printf("Where will the disk be?\n");
	if ((dev == PR0) || (dev == CV2)) {
		printf("Enter:  '0' for builtin port\n");
		printf("        '1' for Expansion Slot 1, Bottom Port\n");
		printf("        '2' for Expansion Slot 1, Top Port\n");
		printf("        '4' for Expansion Slot 2, Bottom Port\n");
		printf("        '5' for Expansion Slot 2, Top Port\n");
		printf("        '7' for Expansion Slot 3, Bottom Port\n");
		printf("        '8' for Expansion Slot 3, Top Port\n");
		p = gets();
		switch (p[0]) {
		case '0':
		case '1':
		case '2':
		case '4':
		case '5':
		case '7':
		case '8':
			unit = p[0] - '0';
			break;
		default:
			printf("Invalid input. Try again.\n");
			goto retry;
		}
	} else { /* dev == PM3 */
		printf("Enter:  '0' for Slot 1\n");
		printf("        '1' for Slot 2\n");
		printf("        '2' for Slot 3\n");
		p = gets();
		switch (p[0]) {
		case '0':
		case '1':
		case '2':
			unit = p[0] - '0';
			break;
		default:
			printf("Invalid input. Try again.\n");
			goto retry;
		}
	}
	return makedev(dev, (unit<<4) | 1 );
}

chkdev(d)
{
	return(*bdevsw[bmajor(d)].d_open)(minor(d), FREAD | FWRITE);
}

/*
 * This version of getchar reads directly from the keyboard in order to get
 * swapdev when the parallel port is not available.  It will not work once
 * the console has been formally opened.
 */
char kb_getchr;
cogetchar()
{
	SPL0();
	while(kb_state) ;	/* wait for kb driver to finish special cmd */
	kb_getchr = 1;		/* wait flag */
	while (kb_getchr) ;	/* wait for it to happen */
	return kb_chrbuf;
}

/* Kernel get string routine.
 * Useful for getting information from the console before the system
 * comes up.  The getchar routine will not work once the console has
 * been opened.
 */
int (*getchar)() = cogetchar;
extern int (*putchar)();

char getsbuf[100];
char *
gets ()
{
	register char *p;
	register char c;
	extern short kb_keycount;

	p = getsbuf;
	while (c = (*getchar)()) {
		switch (c) {
		case '\r':
		case '\n':
			goto out;
		case '\b':
			if (p > getsbuf) {
				p--;
			}
			break;
		case '@':
		case 'X'&0x1F:		/* line kill */
			if (p > getsbuf) {
				p = getsbuf;
				c = '\n';	/* echo a newline */
			}
			break;
		default:
			*p++ = c;
		}
		(*putchar)(c);
		if (p >= getsbuf + sizeof(getsbuf)) {
			printf("\nInput line too long, try again ...\n");
			p = getsbuf;
		}
	}
out:
	*p = '\0';
	(*putchar)('\n');
	return getsbuf;
}
#endif SUNIX
