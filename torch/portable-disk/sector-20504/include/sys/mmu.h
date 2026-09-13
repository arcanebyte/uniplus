/*
 * (C) Copyright 1983 UniSoft Corporation
 */

/*
 * header file for the Motorola MMU
 */

struct mmu {
	char	m_ast[32];	/* address space table */
#define m_astud	m_ast[0x2]
#define m_astup	m_ast[0x4]
#define m_astsd	m_ast[0xA]
#define m_astsp	m_ast[0xC]
#define m_astia	m_ast[0xE]
	short	m_lba;		/* logical base address */
	short	m_lam;		/* logical address mask */
	short	m_pba;		/* physical base address */
	char	m_asn;		/* adress space number */
	char	m_ssr;		/* segment status register */
	char	m_asm;		/* address space mask */
	char	m_dp;		/* descriptor pointer */
	char	m_mmu1;
	char	m_ivr;		/* interrupt vector register */
	char	m_mmu2;
	char	m_gsr;		/* global status register */
	char	m_mmu3;
	char	m_lsr;		/* local status register */
	char	m_mmu4;
	char	m_sstdo;	/* segment status and transfer descriptor op */
	char	m_mmu5[7]; 
	char	m_idp;		/* interrupt descriptor pointer */
	char	m_mmu6;
	char	m_rdp;		/* result descriptor pointer */
	char	m_mmu7;
	char	m_dto;		/* direct translation operation */
	char	m_mmu8;
	char	m_ldo;		/* load descriptor operation */
};

/* segment status register bits */
#define MMUSSRU  0x80		/* used */
#define MMUSSRI	 0x10		/* interrupt */
#define MMUSSRIP 0x08		/* interrupt pending */
#define MMUSSRM	 0x04		/* modified */
#define MMUSSRWP 0x02		/* write protect */
#define MMUSSRE	 0x01		/* enable */

/* global status register bits */
#define MMUGSRF	 0x80		/* fault */
#define MMUGSRDF 0x40		/* double fault */
#define MMUGSRIE 0x01		/* interrupt enable */

/* local status register bits */
#define MMULSRL	0xF0		/* status code */
#	define MMULSRNO	0x00		/* mmu was not source of last event */
#	define MMULSRDT	0x80		/* direct transloation successful */
#	define MMULSRLD	0x90		/* load descriptor fault */
#	define MMULSRUSA 0xA0		/* undefined segment access */
#	define MMULSRWV	0xC0		/* write violation */
#define MMULSRRW  0x08		/* fault on read */
#define MMULSRGAT 0x04		/* global accumulator for translate */
#define MMULSRGAL 0x02		/* global accumulator for load */
#define MMULSRLIP 0x01		/* local interrupt pending */

#define SEGSHIFT	22		/* shift of segment number */
#define PAGESHIFT	11		/* shift of page number */
#define ADDRMASK	(0x1FFFFF)	/* valid address bits */

	/* clicks to memory management units */
#define ctom(x) 	((ctob(x))>>8)

	/* size of the current process */
#define procsize(p)	((p)->p_dsize+(p)->p_ssize)

	/* mmu descriptor macros */
#define dptommu(x)	((x)>>5)
#define dptoo(x)	((x)&0x1F)

	/* macros to eliminate sep I/D */
#define fuiword(x)	fuword(x)
#define suiword(x,y)	suword(x,y)
