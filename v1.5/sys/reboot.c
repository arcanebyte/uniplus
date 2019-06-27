char _Version_[] = "(C) Copyright 1984 UniSoft Corp., Version V.1.0";
char _Origin_[] = "UniSoft Systems of Berkeley";

/*
 * reboot rootdev
 *	Change rootdev, pipedev, dumpdev, swapdev and nswap
 *	in the incore copy of unix and then restart unix.
 *	This is used during installation after a minimal
 *	filesystem has been set up on the hard disk.  It
 *	results in the first boot of unix from the hard disk.
 */

#include "stdio.h"
#include "nmaddrs.h"
#include "sys/types.h"
#include "sys/config.h"
#include "sys/sysmacros.h"
#include "sys/swapsz.h"
#include "sys/reboot.h"

#define USAGE "usage: reboot rootdev"

main(argc, argv)
char **argv;
{
	register fp, cfp;
	short rootdev;
	long nswap;
	int i;

	if (argc != 2)
		perr(USAGE);
	rootdev = (short)strtol(*(argv+1), (char **)NULL, 16);
	if ((fp = open("/dev/mem", 2)) < 0)
		perr("cannot open /dev/mem");

	if (lseek(fp, ROOTDEV, 0) < 0)
		perr("lseek to rootdev %x failed", ROOTDEV);
	if (write(fp, &rootdev, 2) != 2)
		perr("write of rootdev 0x%x at %x failed", rootdev, ROOTDEV);
	printf("rootdev = 0x%x\n", rootdev);

	if (lseek(fp, PIPEDEV, 0) < 0)
		perr("lseek to pipedev %x failed", PIPEDEV);
	if (write(fp, &rootdev, 2) != 2)
		perr("write of pipedev 0x%x at %x failed", rootdev, PIPEDEV);
	printf("pipedev = 0x%x\n", rootdev);

	if (lseek(fp, DUMPDEV, 0) < 0)
		perr("lseek to dumpdev %x failed", DUMPDEV);
	if (write(fp, &rootdev, 2) != 2)
		perr("write of dumpdev 0x%x at %x failed", rootdev, DUMPDEV);
	printf("dumpdev = 0x%x\n", rootdev);

	rootdev++;	/* now it's swapdev */
	if (lseek(fp, SWAPDEV, 0) < 0)
		perr("lseek to swapdev %x failed", SWAPDEV);
	if (write(fp, &rootdev, 2) != 2)
		perr("write of swapdev 0x%x at %x failed", rootdev, SWAPDEV);
	printf("swapdev = 0x%x\n", rootdev);

	if (lseek(fp, NSWAP, 0) < 0)
		perr("lseek to nswap %x failed", NSWAP);
	if (major(rootdev) == PR0)
		nswap = PRNSWAP;
	else if (major(rootdev) == CV2)
		nswap = CVNSWAP;
	else if (major(rootdev) == PM3)
		nswap = PMNSWAP;
	else
		perr("cannot determine size of swapdev");
	if (write(fp, &nswap, 4) != 4)
		perr("write of nswap %d at %x failed", nswap, NSWAP);
	printf("nswap = %d\n", nswap);
	for (i=0; i<200000; i++) ;

	if ((cfp = open("/dev/console", 2)) < 0)
		perr("cannot open /dev/console");
	ioctl(cfp, RESTART, (caddr_t)0);	/* jump to start of unix */
	perr("restart failed");
}

perr(mes, par)
char *mes, *par;
{
	fprintf(stderr, mes, par);
	fprintf(stderr, "\n");
	perror("reboot");
	exit(1);
}
