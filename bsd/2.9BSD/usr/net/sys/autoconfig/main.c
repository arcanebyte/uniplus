/*
 * sysconfig -- Program to auto configure a kernel to the devices which
 * are present.  Needs the ucall() system call and special kernel to work.
 *
 * Michael Toy -- Italy 1983
 *
 * added internal, user-level probe facilities,
 * Michael Karels -- Berkeley 1983
 */

#include	<stdio.h>
#include	<sys/autoconfig.h>
#include	"args.h"

char		*dtab_name = "/etc/dtab";
char		*nlist_name = "/unix";
char		*kmem_name = "/dev/kmem";
int		kmem;
int		verbose;
int		debug;
int		complain;
FILE		*dtab_fp;
FILE		*fopen();

main(argc, argv)
int argc;
char **argv;
{
	setbuf(stdout, NULL);

	parse_args(argc, argv, "cdvP", "ink");

	/* -c -- Complain about bad vectors */
	complain = bools('c');

	/* -v -- Verbose output */
	verbose = bools('v');

	/* -d -- Debugging run */
	debug = bools('d');

	/* -i file -- Read in file instead of "dtab" */
	if (strings('i'))
		dtab_name = strings('i');
	if ((dtab_fp = fopen(dtab_name, "r")) == NULL) {
		perror(dtab_name);
		exit(AC_SETUP);
	}

	/* -k file -- use file instead of "/dev/kmem" */
	if (! debug) {
		if (strings('k'))
			kmem_name = strings('k');
		if ((kmem = open(kmem_name, 2)) < 0) {
			perror(kmem_name);
			exit(AC_SETUP);
		}
	}

	/* -n file -- Namelist is in file instead of /unix */
	if (strings('n'))
		nlist_name = strings('n');

	/* Read the dtab into internal tables so we can play with it */
	read_dtab();

	/* Now set up for and call nlist so we can get kernel symbols */
	read_nlist();

	/* And at last change the kernel to suit ourselves */
	auto_config();

	/* All done go bye bye now */
	exit(AC_OK);
}

char *strsave(cp)
{
	return strcpy(malloc(strlen(cp) + 1), cp);
}
