/*
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Interrupt handler for level 2 interrupts.
 *	keyboard, mouse, real time clock, on/off switch
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/reg.h"
#include "sys/local.h"
#include "sys/cops.h"
#include "sys/keyboard.h"
#include "sys/mouse.h"
#include "sys/ms.h"
#include "sys/l2.h"
#include "sys/kb.h"

extern struct rtime rtime;

char *kb_keytab = ToLA;
char kb_altkp;			/* are we in alternate keypad mode? (set in vt100.c) */

char pportplug;
char ms_plg, ms_btn;
short ms_row, ms_col;

kbintr()
{
	register char i;
	register struct device_e *p = COPSADDR;
	register char a, ud;
	register int tmp;
	extern time_t lbolt;
#ifdef SUNIX
	extern char kb_getchr;	/* flag for polling keyboard */
#endif SUNIX

	a = p->e_ifr;			/* Read and save reason for interrupt */
	if (a & FCA1) {			/* keyboard input */
		i = p->e_ira;			/* get keyboard/mouse input */
		if (a & FTIMER1)
			a = COPSADDR->e_t1cl;	/* prime timer */
	} else {			/* no character input */
		a = COPSADDR->e_t1cl;		/* prime timer */
		if (--kb_reptrap == 0)
			kbrepeat();		/* possible char repeat */
		return;		
	}

	switch (kb_state) {
	case NORMALWAIT:		/* IDLE LOOP */
		ud = i & 0x80;		/* whether up or down keycode */
		l2_dtrap = lbolt + l2_dtime;	/* reset dim delay */
		if (l2_dimmed)		/* restore screen intensity */
			l2undim();
		a = kb_keytab[i & 0x7F];/* convert to ascii */
		if (ud) {		/* "key went down" bit */
/* click(); /* key click */
			if (ARROW(i,a)) {	/* check arrow keys */
				kb_chrbuf = Esc;    /* send 3-char sequence */
				cointr(0);
				kb_chrbuf = '[';
				cointr(0);
				kb_chrbuf = a;
				cointr(0);
				goto out;
			}
			if (kb_altkp) {	/* send special sequence for keypad chars */
				if (a = altkpad[i & 0x7F]) {/* is it in the keypad? */
					kb_chrbuf = Esc;    /* send 3-char sequence */
					cointr(0);
					kb_chrbuf = 'O';
					cointr(0);
					kb_chrbuf = a;
					cointr(0);
					goto out;
				}
				a = kb_keytab[i & 0x7F];/* reset to ascii */
			}
			if (a >= 0) {	/* ascii ? */
				kb_keycount++;
				if (kb_ctrl) a &= 0x1F;
				else if (kb_keycount == 1) {
					kb_reptrap = kb_repwait;
					kb_lastc = a;
				} else kb_reptrap = 0;
				kb_chrbuf = a;
				cointr(0);
				goto out;
			}
		} else {		/* key went up */
			kb_reptrap = 0;
			if (a >= 0) {
				if (kb_keycount-- < 0) {
					kb_keycount = 0;
				}
				goto out;
			}
		}
		switch (a & 0xF) {
		case KB_CTRL:	kb_ctrl = ud; kb_reptrap = 0; msintr(M_CTL);
								goto out;
		case KB_SHFT:	kb_shft = ud; kbsetcvtab(); msintr(M_SFT);
								goto out;
		case KB_LOCK:	kb_lock = ud; kbsetcvtab();	goto out;
		case KB_OFF:	printf("[SOFT OFF %x]\n",i);	goto out;
		case KB_MSP:	ms_plg = ud; msintr(M_PLUG);	goto out;
		case KB_MSB:	ms_btn = ud; msintr(M_BUT);	goto out;
		case KB_PPORT:	pportplug = ud;			goto out;
		case KB_D2B:	printf("[disk1button %x]\n",i);	goto out;
		case KB_D2P:	printf("[disk1 %x]\n",i);	goto out;
		case KB_D1B:	printf("[disk2button %x]\n",i);	goto out;
		case KB_D1P:	printf("[disk2 %x]\n",i);	goto out;
		case KB_STATE:	if (ud == 0) {
					kb_state = MOUSERD;
				} else
					kb_state = RESETCODE;
				goto out;
		default:	printf("invalid key[0x%x]",i);
		}
		goto out;
	case MOUSERD:		/* PICKUP Y axis change in mouse pos */
		kb_state = YMOUSE;
		ms_col = (short)i;
		goto out;
	case YMOUSE:		/* PICKUP Y axis change in mouse pos */
		kb_state = NORMALWAIT;
		ms_row = (short)i;
		msintr(M_MOVE);
		goto out;
	case RESETCODE:		/* special condition */
		switch (i & 0xFF) {
		case KB_KBCOPS:	/* keyboard cops failure detected */
			printf("KEYBOARD COPS FAILURE\n");
			break;
		case KB_IOCOPS:	/* IO board cops failure detected */
			printf("IO BOARD COPS FAILURE\n");
			break;
		case KB_UNPLUG:	/* keyboard unplugged */
			kb_chrbuf = 's'&0x1F;	/* cntl S */
			cointr(0);
			break;
		case KB_CLOCKT:	/* clock timer interrupt */
			printf("Real Time Clock interrupt\n");
			break;
		case KB_SFTOFF:	/* soft power switch */
			kb_state = SHUTDOWN;
			printf("Shutting down...\n");
			update();
			for (tmp = 0; tmp < 500000; tmp++);
			l2_crate = 2;
			l2_desired = TOTALDIM;	/* completely blacken screen */
			l2ramp(0);
			l2ramp(0);
			SPL7();			/* extreme priority */
			l2copscmd(SHUTOFF);
			rom_mon();		/* return to the ROM monitor */
			/*NOTREACHED*/
		default: switch (i & 0xF0) {
			case KB_RESERV:
				printf("[Reserved keycode 0x%x]\n",i);
				break;
			case KB_RDCLK:
				rtime.rt_year = (i & 0xF) + 10;	
				kb_state = CLKREAD;
				goto out;
			default:
				kb_idcode = i;
				kb_chrbuf = 'q'&0x1F;	/* cntl Q */
				cointr(0);
				printf("Keyboard type 0x%x\n",i);
			}
		}
		kb_state = NORMALWAIT;
		goto out;
	case CLKREAD:
		rtime.rt_day = (((i&0xF0)>>4)*10 + (i&0xF))*10;
		kb_state++;
		goto out;
	case CLKREAD+1:
		rtime.rt_day += (i&0xF0) >> 4;
		rtime.rt_hour = (i & 0xF) * 10;
		kb_state++;
		goto out;
	case CLKREAD+2:
		rtime.rt_hour += (i & 0xF0) >> 4;
		rtime.rt_min = (i & 0x0F) * 10;
		kb_state++;
		goto out;
	case CLKREAD+3:
		rtime.rt_min += (i & 0xF0) >> 4;
		rtime.rt_sec = (i & 0x0F) * 10;
		kb_state++;
		goto out;
	case CLKREAD+4:
		rtime.rt_sec += (i & 0xF0) >> 4;
		rtime.rt_tenth = i & 0x0F;
		kb_state = NORMALWAIT;
		rtcsettod();
		goto out;
	case SHUTDOWN:
		goto out;
	}
out:	;
#ifdef SUNIX
	if (!ud)
		kb_getchr = 0;
#endif SUNIX
}

kbrepeat ()
{
	kb_reptrap = kb_repdlay;		/* reset repeat timeout */

	if (kb_keycount == 1) {
		kb_chrbuf = kb_lastc;
/* click(); /* key click */
		cointr(0);
	} else
		kb_reptrap = 0;		/* reset repeat timeout */
}

kbsetcvtab()
{
	kb_keytab = ccvtab[(kb_shft?2:0)+(kb_lock?1:0)];
	kb_reptrap = 0;
}
