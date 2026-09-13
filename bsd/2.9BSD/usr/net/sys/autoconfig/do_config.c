/*
 * Now with all our information, make a configuration
 * Devices without both attach and probe routines will
 * not be configured into the system
 */

#include	<stdio.h>
#include	<a.out.h>
#include	<sys/autoconfig.h>
#include	<sys/trap.h>
#include	<errno.h>
#include	<sys/psw.h>
#include	"dtab.h"
#include	"ivec.h"

extern int kmem, verbose, debug, errno, complain;
extern struct nlist *bad_nl, *good_nl, *int_nl, *end_vector, *trap_nl, *sep_nl;

grab(where)
unsigned int where;
{
	int var;

	if (debug) {
		char line[80];

		printf("Grab %o =", where);
		gets(line);
		return otoi(line);
	}
	lseek(kmem, ((long) where) & 0xffffL, 0);
	read(kmem, &var, sizeof var);
	return var;
}

stuff(var, where)
unsigned int where;
{
	char s[20];

	if (debug) {
		printf("Stuff %o @ %o\n", var, where);
		return;
	}
	lseek(kmem, ((long) where) & 0xffffL, 0);
	if (write(kmem, &var, sizeof var) < sizeof var) {
		sprintf(s, "stuff 0%o", where);
		perror(s);
	}
}

prdev(dp)
struct dtab_s *dp;
{
	printf("%s ", dp->dt_name);
	if (dp->dt_unit == -1)
		printf("?");
	else
		printf("%d", dp->dt_unit);
	printf(" csr %o vector %o", dp->dt_addr, dp->dt_vector);
}

/*
 * Go through the configuration table and probe all the devices.  Call
 * attach for ones which exist.  Probe routines should return
 *	ACP_NXDEV	No such device
 *	ACP_IFINTR	Device exists if interrupts ok
 *	ACP_EXISTS	Device exists
 * All interrupt vectors are poked to either point to conf_goodvec or
 * conf_badvec which change the value of conf_int.  Values of this
 * variable are:
 *	ACI_BADINTR	Device interrupted through wrong vector
 *	ACI_NOINTR	Device didn't interrupt
 *	ACI_GOODINTR	Interrupt ok
 */

auto_config()
{
	register struct dtab_s *dp;
	int ret;

	if (intval() != CONF_MAGIC) {
		fprintf(stderr, "Namelist doesn't match running kernel\n");
		exit(AC_SETUP);
	}

	init_lowcore(bad_nl->n_value);

	for (dp = devs; dp != NULL; dp = dp->dt_next) {

		/* Make sure we have both a probe and attach routine */
		if (!((dp->dt_uprobe || (dp->dt_probe && dp->dt_probe->n_value))
		    && (dp->dt_attach && dp->dt_attach->n_value))) {
			if (debug || verbose) {
				prdev(dp);
				printf(" skipped:  No autoconfig routines\n");
			}
			continue;
		}

		/* Make sure the CSR is there */
		errno = 0;
		grab(dp->dt_addr);
		if (errno) {
			if (errno != EFAULT && errno != ENXIO)
				perror("Reading CSR");
			if (debug || verbose) {
				prdev(dp);
				printf(" skipped:  No CSR\n");
			}
			detach(dp);
			continue;
		}

		/* Ok, try a probe now */
		if (expect_intr(dp)) {
			if (complain) {
				prdev(dp);
				printf(" interrupt vector already in use\n");
			}
			detach(dp);
			continue;
		}
		ret = do_probe(dp, dp->dt_addr);
		clear_vec(dp);
		switch (ret) {
			case ACP_NXDEV:
				if (debug || verbose) {
					prdev(dp);
					printf(" does not exist\n");
				}
				detach(dp);
				break;
			case ACP_IFINTR:
				switch (intval()) {
					case ACI_BADINTR:
						if (debug || verbose || complain) {
							prdev(dp);
							printf(" interrupt vector wrong\n");
						}
						detach(dp);
						break;
					case ACI_NOINTR:
						if (complain) {
							prdev(dp);
							printf(" didn't interrupt\n");
						}
						detach(dp);
						break;
					case ACI_GOODINTR:
						attach(dp);
						break;
					default:
						prdev(dp);
						printf(" bad interrupt value %d\n", intval());
						break;
				}
				break;

			case ACP_EXISTS:
				attach(dp);
				break;
			
			default:
				prdev(dp);
				printf(" bad probe value %d\n", ret);
				break;
		}
	}
	set_unused();
}

/*
 * Return the current value of the interrupt return flag.
 * Initial value is the magic number
 */

static int conf_int = CONF_MAGIC;

intval()
{
	if (debug)
		return conf_int;
	else
		return grab(int_nl->n_value);
}

static int save_vec[9][2], save_p;

/*
 * Fill all interrupt vectors of this device with pointers to
 * the good interrupt vector routine.  Also save values to
 * later restore them.  Since we init_lowcore() everything to
 * conf_badint, anything not equalling that indicates a vector
 * which is already in use; unless the vector was initialized in l.s,
 * this device should be skipped.
 */

expect_intr(dp)
struct dtab_s *dp;
{
	struct handler_s *hp;
	int addr;

	addr = dp->dt_vector;
	for (save_p = 0, hp = dp->dt_handlers; hp != NULL; hp = hp->s_next) {
		save_vec[save_p][1] = grab(addr + sizeof(int));
		if (((save_vec[save_p][0] = grab(addr)) != bad_nl->n_value)
		    && ((save_vec[save_p][0] != hp->s_nl->n_value)
		    || have_set(addr))) {
			clear_vec(dp);
			return 1;
		}
		save_p ++;
		write_vector(addr, good_nl->n_value, PS_BR7);
		addr += IVSIZE;
	}
	return 0;
}

clear_vec(dp)
register struct dtab_s *dp;
{
	register int addr = dp->dt_vector, n;

	for (n = 0; n < save_p; n++) {
		write_vector(addr, save_vec[n][0], save_vec[n][1]);
		addr += IVSIZE;
	}
}

init_lowcore(val)
{
	int addr;

	if (debug)
		return;
	for (addr = 0; addr < end_vector->n_value; addr += IVSIZE) {
		if (grab(addr) || grab(addr + 2))
			continue;
		write_vector(addr, val, PS_BR7);
	}
}

do_probe(dp, a1)
register struct dtab_s *dp;
int a1;
{
	int func;
	int ret;

	func = dp->dt_probe->n_value;
	if (debug) {
		char line[80];

		if (func)
			printf("ucall %o(PS_BR0, %o, 0):", func, a1);
		else
			printf("probe %s:", dp->dt_name);
		printf(" return conf_int:");
		gets(line);
		sscanf(line, "%o%o", &ret, &conf_int);
		return ret;
	}
	stuff(0, int_nl->n_value);	/* Clear conf_int */
	/*
	 * use the kernel's probe routine if it exists,
	 * otherwise use our internal probe.
	 */
	if (func) {
		errno = 0;
		ret = ucall(PS_BR0, func, a1, 0);
		if (errno)
			perror("ucall");
		return(ret);
	}
	return((*(dp->dt_uprobe))(a1));
}

set_unused()
{
	int addr;

	if (debug)
		return;
	if (sep_nl->n_value) {
		/*
		 * On non-separate I/D kernel, attempt to set up catcher
		 * at 0 for both jumps and traps to 0.
		 * On traps, set PS for randomtrap, with the catcher at 0444.
		 * On jumps, branch to 0112, then to 050 (where there's space).
		 * The handler at 50 is already in place.
		 * The funny numbers are due to attempts to find vectors
		 * that are unlikely to be used, and the need for the value
		 * at 0 to serve as both a vector and an instruction
		 * (br 112 is octal 444).
		 */
		if ((grab(0110) == bad_nl->n_value) && (grab(0112) == PS_BR7)
		    && (grab(0444) == bad_nl->n_value) && (grab(0446) == PS_BR7)) {
			stuff(0444, 0);			/* br 0112 */
			stuff(PS_BR7 + ZEROTRAP, 2);
			stuff(trap_nl->n_value, 0110);	/* trap; 756 (br7+14.)*/
			stuff(0756, 0112);		/* br 050 */
			stuff(0137, 0444);		/* jmp $*trap */
			stuff(trap_nl->n_value, 0446);
		}
	}
	for (addr = 0; addr < end_vector->n_value; addr += IVSIZE) {
		if (grab(addr) != bad_nl->n_value)
			continue;
		write_vector(addr, trap_nl->n_value, PS_BR7+RANDOMTRAP);
	}
}
