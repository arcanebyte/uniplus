/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * reinit.c - reinitialize parts of data segment for restarting unix
 */

#include "sys/param.h"
#include "sys/config.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/reg.h"
#include "setjmp.h"
#include "sys/kb.h"
#include "sys/sysmacros.h"
#include "sys/iobuf.h"
#include "sys/map.h"
#define	CMAPSIZ	50	/* also in conf.c */
#define	SMAPSIZ	50	/* also in conf.c */

extern mpid;		/* used by newproc to decide whether first "init" or not */

extern struct iostat prostat[];
extern struct iobuf protab;
extern struct iostat snstat[];
extern struct iobuf sntab;
extern struct iostat cvstat[];
extern struct iobuf cvtab;

extern char *kb_keytab;
extern char ToLA[];
extern char kb_altkp;
extern int (*te_putc)();
extern int vt_putc();
extern char vt_tabset[];
/*
 *	reinit - called from RESTART console ioctl call in sunix
 */
reinit()
{
	extern int teslotsused;
	register i;

	mpid = 0;

	retabinit(&protab, PR0, prostat);
	retabinit(&sntab, SN1, snstat);
	retabinit(&cvtab, CV2, cvstat);

	kb_keytab = ToLA;
	kb_shft = kb_lock = kb_altkp = 0;
	te_putc = vt_putc;
	for (i = 0 ; i < 88 ; i++ )
		vt_tabset[i] = 0;

	remapinit(&coremap[0], CMAPSIZ);
	remapinit(&swapmap[0], SMAPSIZ);
	/*
	 * reset tecmar four port card
	 */
	teslotsused = 0;
}

retabinit(tab, dev, stat)
register struct iobuf *tab;
struct iostat stat[];
{
	tab->b_flags = 0;
	tab->b_forw = 0;
	tab->b_back = 0;
	tab->b_actf = 0;
	tab->b_actl = 0;
	tab->b_dev = makedev(dev, 0);
	tab->b_active = 0;
	tab->b_errcnt = 0;
	tab->io_erec = 0;
	tab->io_nreg = 0;
	tab->io_addr = 0;
	tab->io_stp = stat;
	tab->io_start = 0;
	tab->io_s1 = 0;
	tab->io_s2 = 0;
}

remapinit(map, szmap)
struct map *map;
{
	map->m_size = szmap - 2;
	map->m_addr = 0;
	for (map++, szmap--; szmap > 0; map++, szmap--) {
		map->m_size = 0;
		map->m_addr = 0;
	}
}
