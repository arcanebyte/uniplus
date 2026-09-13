/*
 * Attach the passed device:
 *	Patch the interrupt vector
 *	Call the device attach routine
 */

#include	<stdio.h>
#include	<a.out.h>
#include	<sys/autoconfig.h>
#include	<sys/psw.h>
#include	"dtab.h"
#include	"ivec.h"

struct	done_s {
	char	*d_name;		/* Device name */
	int	d_unit;			/* Current unit for wildcarding */
	struct	done_s *d_next;
};

struct	done_s	*done = NULL;		/* List of things done */
extern	int	debug;
extern	int	errno;
int	vec_set[NVECTOR], num_vec;

attach(dp)
struct dtab_s *dp;
{
	int unit, addr;
	int ret;
	register struct handler_s *sp;

	if ((unit = find_unit(dp)) == -1) {
		prdev(dp);
		printf(" unit already in use\n");
		return;
	}

	/* First attach the device */
	errno = 0;
	if (debug)
		printf("attach: ucall %o(PS_BR0, %o, %o)\n",
			dp->dt_attach->n_value,
			dp->dt_addr, unit);
	else if ((ret = ucall(PS_BR0, dp->dt_attach->n_value, dp->dt_addr, unit)) == 0
	    || ret == -1) {
		prdev(dp);
		if (ret == -1 && errno) {
			perror("ucall");
			exit(AC_SINGLE);
		}
		printf(" attach failed\n");
		return;
	}

	/* Then fill the interrupt vector */
	addr = dp->dt_vector;
	for (sp = dp->dt_handlers; sp != NULL; sp = sp->s_next) {
		if (sp->s_nl->n_value == 0) {
			prdev(dp);
			printf(" no address found for %s\n", sp->s_str);
			exit(AC_SINGLE);
		}
		write_vec(addr, sp->s_nl->n_value, pry(dp) + unit);
		if (num_vec == NVECTOR-1) {
			printf("Too many vectors to configure\n");
			exit(AC_SINGLE);
		} else
			vec_set[num_vec++] = addr;
		addr += IVSIZE;
	}
	prdev(dp);
	printf(" attached\n");
}

have_set(vec)
{
	int i;

	for (i = 0; i < num_vec; i++)
		if (vec_set[i] == vec)
			return(1);
	return(0);
}
pry(dp)
struct dtab_s *dp;
{
	switch(dp->dt_br) {
		case 4: return PS_BR4;
		case 5: return PS_BR5;
		case 6: return PS_BR6;
		case 7: return PS_BR7;
		default:
			prdev(dp);
			printf(": br%d is not supported.  Assuming 7\n", dp->dt_br);
	}
	return PS_BR7;
}

write_vec(addr, value, pri)
{
	stuff(value, addr);
	stuff(pri, addr + sizeof (int));
}

/*
 * find_unit -- Add this device to the list of devices if it isn't already
 * in the list somewhere.  If it has an explicit unit number then use that
 * else fill in the wildcard with next biggest number
 */

find_unit(dp)
struct dtab_s *dp;
{
	struct done_s *dn = NULL;
	int found, want_unit;

	found = 0;
	if (done == NULL) {
		dn = malloc(sizeof *dn);
		done = dn;
		dn->d_next = NULL;
	} else {
		dn = done;
		while (1) {
			if (strcmp(dn->d_name, dp->dt_name) == 0) {
				found = 1;
				break;
			}
			if (dn->d_next == NULL)
				break;
			dn = dn->d_next;
		}
		if (! found) {
			dn->d_next = malloc(sizeof *dn);
			dn = dn->d_next;
			dn->d_next = NULL;
		}
	}

	if (! found) {
		dn->d_name = dp->dt_name;
		if ((dn->d_unit = dp->dt_unit - 1) < -1)
			dn->d_unit = -1;
	}

	/* Fill in wildcards */
	if ((want_unit = dp->dt_unit) == -1)
		want_unit = dn->d_unit + 1;
	else if (want_unit <= dn->d_unit)
		return -1;
	return dn->d_unit = dp->dt_unit = want_unit;
}

/*
 *  Call the device-attach routine with a 0 addr
 *  to indicate that the device isn't present
 *  (needed for devices that might be root devices
 *  and thus must have addr/vector initialized).
 *  Only done if the unit number was not a wildcard.
 */
detach(dp)
struct dtab_s *dp;
{
	int unit;

	if ((unit = dp->dt_unit) == -1)
		return;
	if (debug)
		printf("detach: ucall %o(PS_BR0, %o, %o)\n", dp->dt_attach->n_value,
			0, unit);
	else
		ucall(PS_BR0, dp->dt_attach->n_value, 0, unit);
}
