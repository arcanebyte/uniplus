/*
 * Copyright 1982 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement with
 * AT&T, Western Electric and UniSoft Corporation.
 *
 * Definitions relating te the COPS and Keyboard SY6522 interface.
 */

	/* Breakdown of e_ifr (Interrupt Flag Register) of 6522 */
#define FCA2 	0x61		/* ca2 -- handshake with COPS, not intr source */
#define FCA1 	0x02		/* cal -- intr latch for input from COPS */
#define FSHFT 	0x04		/* shft -- completed eight shifts */
#define FCB2 	0x08		/* cb2 -- output to speaker (if suitably programmed) */
#define FCB1 	0x10		/* cbl -- unused */
#define FTIMER2 0x20		/* timeout of timer two -- unused */
#define FTIMER1 0x40		/* timeout of timer one -- 10ms clock */
#define FIRQ 	0x80		/* interrupt happened (cleared by clearing intr) */

struct device_e {				/* Keyboard SY6522 VIA */
  char e_f0[1]; char e_irb;			/* I/O register B */
  char e_f1[1]; char e_ira;			/* I/O register A */
  char e_f2[1]; char e_ddrb;			/* Data Dir reg B */
  char e_f3[1]; char e_ddra;			/* Data Dir reg A */
  char e_f4[1]; char e_t1cl;			/* T1 low Latches Counter */
  char e_f5[1]; char e_t1ch;			/* T1 hi Latches Counter */
  char e_f6[1]; char e_t1ll;			/* Tl low Latches */
  char e_f7[1]; char e_t1lh;			/* T1 hi Latches */
  char e_f8[1]; char e_t2cl;			/* T2 low Latches Counter */
  char e_f9[1]; char e_t2ch;			/* T2 hi Latches Counter */
  char e_fa[1]; char e_sr;			/* Shift Register */
  char e_fb[1]; char e_acr;			/* Aux Ctrl Reg */
  char e_fc[1]; char e_per;			/* Perif ctrl Reg */
  char e_fd[1]; char e_ifr;			/* Int Flag Reg */
  char e_fe[1]; char e_ier;			/* Int Ena Reg */
  char e_ff[1]; char e_aira;			/* Alt e_ira (no handshake) */
};
#define COPSADDR ((struct device_e *) (STDIO+0xDD86))

/* A port definitions */
/* Connects PR (reset pulse line) to the controller reset switch if low,
* or the parity reset latch if high (when set as an output).
*/
#define CR 	0x80
#define CRDY 	0x46			/* goes low when cops ready for command */
#define PR 	0x20			/* works with CR to get pport parity error */
#define FDIR 	0x10			/* floppy dir interrupt request */
#define VC2 	0x08
#define VCl 	0x04
#define VCO 	0x02
#define KBIN 	0x01

