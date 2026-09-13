/* fake ubavar.h to initialize net device drivers */

struct uba_device {
	struct	uba_driver *ui_driver;
	short	ui_unit;	/* unit number on the system */
	caddr_t	ui_addr;	/* address of device in i/o space */
	u_long  ui_flags;       /* parameter from system specification */
	short	ui_alive;	/* device exists */
};

struct uba_driver {
	int	(*ud_probe)();		/* see if a driver is really there */
	int	(*ud_attach)();		/* setup driver */
	u_short	*ud_addr;		/* device csr addresses (or whatever) */
	struct	uba_device **ud_dinfo;	/* backpointers to ubdinit structs */
};

