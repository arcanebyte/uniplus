/*
 * mkfspm device
 *	Get the size of the Priam disk "device" and make a root
 *	filesystem in partition 0.  This makes a filesystem using the
 *	entire disk except for the boot and swap areas.
 *
 *	The raw device is specified, but both the block and character
 *	devices are expected to be available (created).  The filesystem
 *	is made on /dev/pm?a where /dev/rpm?a is the argument.
 */

#include "stdio.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/uioctl.h"
#include "sys/stat.h"
#include "sys/swapsz.h"

main(argc, argv)
char **argv;
{
	register fp;
	register char *device;
	register char *cp;
	int pmsize;
	char spmsize[10];

	device = *(argv+1);
	if ((fp = open(device, 2)) < 0)
		goto out;
	if (ioctl(fp, UIOCSIZE, (caddr_t)&pmsize) < 0)
		goto out;
	cp = (char *)strchr(device, 'r');	/* cp points to 'r' in /dev/rpm?a */
	for ( ; *cp; cp++)			/* change to /dev/pm?a */
		*cp = *(cp+1);
	sprintf(spmsize, "%d", pmsize-PMNSWAP-101);
	printf("%s: %s blocks\n", device, spmsize);
	execl("/etc/mkfs1b", "mkfs1b", device, spmsize, 0);
out:
	perror("mkfspm");
	exit(1);
}
