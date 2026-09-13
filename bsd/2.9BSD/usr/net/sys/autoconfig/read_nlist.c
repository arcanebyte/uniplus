/*
 * Read all the symbols we'll need for the devices we want to configure
 * These are -- The probe, attach and handler routines for each device,
 * and a few symbols that we'll need later on.
 */

#include	<stdio.h>
#include	<a.out.h>
#include	<sys/autoconfig.h>
#include	<sys/param.h>
#include	"dtab.h"
#include	"args.h"

extern char	*nlist_name;	/* File we read the namelist from */
extern int	guess_ndev;	/* Number of lines read from dtab */
extern int	debug;
extern int	kmem;
struct nlist	*nl, *np;	/* Pointers to nlist structures */
struct nlist *int_nl, *good_nl, *bad_nl, *add_nlist(), *end_vector;
struct nlist *trap_nl, *sep_nl, *vers_nl;

#define END_NAME	"endvec"
#define INT_NAME	"_conf_int"
#define GOOD_NAME	"CGOOD"
#define BAD_NAME	"CBAD"
#define TRAP_NAME	"trap"
#define SEPID_NAME	"KERN_NONS"	/* KERN_NONSEP */
#define VERSION		"_version"

read_nlist()
{
	register struct nlist **lp;
	register struct dtab_s *dp;
	register struct nlist *nnp;
	struct handlers_s *already = NULL, *sp;
	char tname[20];
	int unix_fd;
	struct exec head;
	struct ovlhdr ovhead;
	off_t offst;
	char unix_vers[100], core_vers[100];

	np = nl = (struct nlist *) calloc(guess_ndev + 8, sizeof *nl);
	for (dp = devs; dp != NULL; dp = dp->dt_next) {
		sprintf(tname, "_%sprobe", dp->dt_name);
		dp->dt_probe = add_nlist(tname);
		sprintf(tname, "_%sattach", dp->dt_name);
		dp->dt_attach = add_nlist(tname);
		for (sp = dp->dt_handlers; sp != NULL; sp = sp->s_next)
			sp->s_nl = add_nlist(sp->s_str);
	}
	end_vector = np++;
	strncpy(end_vector->n_name, END_NAME, sizeof end_vector->n_name);
	int_nl = np++;
	strncpy(int_nl->n_name, INT_NAME, sizeof int_nl->n_name);
	good_nl = np++;
	strncpy(good_nl->n_name, GOOD_NAME, sizeof good_nl->n_name);
	bad_nl = np++;
	strncpy(bad_nl->n_name, BAD_NAME, sizeof bad_nl->n_name);
	trap_nl = np++;
	strncpy(trap_nl->n_name, TRAP_NAME, sizeof trap_nl->n_name);
	vers_nl = np++;
	strncpy(vers_nl->n_name, VERSION, sizeof vers_nl->n_name);
	sep_nl = np++;
	strncpy(sep_nl->n_name, SEPID_NAME, sizeof sep_nl->n_name);
	if ((unix_fd = open(nlist_name, 0)) < 0) {
		perror(nlist_name);
		exit(AC_SETUP);
	}
	nlist(nlist_name, nl);
	if (debug || bools('P')) {
		for (np = nl; *np->n_name; np++)
			printf("%.8s = %o\n", np->n_name, np->n_value);
	}
	for (np = end_vector; np <= trap_nl; np++) {
		if (np->n_value == 0) {
			fprintf(stderr, "Couldn't find symbols in %s\n",
				nlist_name);
			exit(AC_SETUP);
		}
	}
	if (!debug) {
#define round(x) (ctob(stoc(ctos(btoc(x)))))
		lseek(unix_fd, (off_t) 0, 0);
		read(unix_fd, (char *)&head, sizeof head);
		offst = (off_t)vers_nl->n_value
			+ (off_t) head.a_text + sizeof(head);
		if (head.a_magic == A_MAGIC2 || head.a_magic == A_MAGIC5)
			offst -= (off_t)round(head.a_text);
		if (head.a_magic == A_MAGIC5 || head.a_magic == A_MAGIC6) {
			register i;
			read(unix_fd, (char *)&ovhead, sizeof ovhead);
			offst += sizeof ovhead;
			if (head.a_magic == A_MAGIC5)
				offst -= (off_t)round(ovhead.max_ovl);
			for(i=0; i<NOVL; i++)
				offst += (off_t)ovhead.ov_siz[i];
		}
		lseek(unix_fd, offst, 0);
		read(unix_fd, unix_vers, sizeof(unix_vers));
		lseek(kmem, (off_t)vers_nl->n_value, 0);
		read(kmem, core_vers, sizeof(core_vers));
		unix_vers[99] = core_vers[99] = 0;	/* Just in case! */
		if (strcmp(unix_vers, core_vers)) {
			fprintf(stderr, "%s is not the running version\n",
				nlist_name);
			exit(AC_SETUP);
		}
	}
	close(unix_fd);
}

/*
 * If the passed symbol is in the nlist table, return pointer to it,
 * otherwise add it to the table and return a pointer to new entry.
 */

struct nlist *add_nlist(name)
char *name;
{
	register struct nlist *n;

	for (n = nl; n < np; n++)
		if (strncmp(n->n_name, name, sizeof n->n_name) == 0)
			return n;
	strncpy(np->n_name, name, sizeof n->n_name);
	return np++;
}
