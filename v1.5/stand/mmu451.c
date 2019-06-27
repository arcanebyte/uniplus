/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#define MMUADDR	((struct mmu *)0x7E1800)
#include <sys/mmu.h>

main()
{
	struct mmu *mm = MMUADDR;
	int laddr, phaddr, segsize, asn, asmask;
	int i, j;
	int (* monitor )() = (int (*)()) 0x7c0400;
	printf("68451 tester\n\n");
	laddr = 0;
	phaddr = 0x40000;
	asmask = 0xFF;
    for (segsize = 0x800; segsize < 0x1000000; segsize <<= 1) {
	for (i = 1; i < 32; i++) {
		printf("dp %x segsize=%x\n", i, segsize);
		mm->m_dp = i;
		mm->m_lba = laddr;
		mm->m_pba = phaddr;
		mm->m_lam = -segsize;
		mm->m_asn = i;
		mm->m_asm = asmask;
		mm->m_ssr = MMUSSRE;
		if (mm->m_ldo) {
			printf("unsuccessful mmu load\n");
			printf("lba=%x lam=%x pba=%x asn=%x asm=%x\n",
			laddr<<8, (-segsize)<<8,
			phaddr<<8, asn, asmask);
		}
	}
    }
    printf("DONE TESTING\n");
	(*monitor)();
}
