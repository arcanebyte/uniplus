/*
 * Internal structure used to store info from the device table
 */

struct handler_s {
	struct handler_s *s_next;
	char *s_str;
	struct nlist *s_nl;
};

struct dtab_s {
	struct dtab_s	*dt_next;	/* Next device in list */
	char	*dt_name;		/* Two character device name */
	int	dt_unit;		/* Unit number, -1 is wildcard */
	int	dt_addr;		/* CSR address */
	int	dt_vector;		/* Interrupt vector */
	struct handlers_s *dt_handlers;	/* Interrupt handlers */
	int	dt_br;			/* Priority */
	int	(*dt_uprobe)();		/* User-level (internal) probe */
	struct nlist *dt_probe;		/* Address of probe function */
	struct nlist *dt_attach;	/* Address of attach function */
};

struct dtab_s *devs;			/* Head of device list */
