/* NFSSRC @(#)conf.h	2.1 86/04/11 */
/*	@(#)conf.h	UniPlus VVV.2.1.1	*/
/*
 * Declaration of block device switch. Each entry (row) is
 * the only link between the main unix code and the driver.
 * The initialization of the device switches is in the file conf.c.
 */
struct bdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_print)();
};
extern struct bdevsw bdevsw[];

/*
 * Character device switch.
 */
struct cdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	struct tty *d_ttys;
	int	(*d_select)();
	struct streamtab *d_str;
};
extern struct cdevsw cdevsw[];

extern int	bdevcnt;
extern int	cdevcnt;

/*
 *	Streams modules 
 */
 
#define FMNAMESZ	8

struct fmodsw {
	char f_name[FMNAMESZ+1];
	struct streamtab *f_str;
};

extern struct fmodsw fmodsw[];

extern int fmodcnt;

/*
 * Line discipline switch.
 */
struct linesw {
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_input)();
	int	(*l_output)();
	int	(*l_mdmint)();
};
extern struct linesw linesw[];

extern int	linecnt;
/*
 * Terminal switch
 */
struct termsw {
	int	(*t_input)();
	int	(*t_output)();
	int	(*t_ioctl)();
};
extern struct termsw termsw[];

extern int	termcnt;

/*
 * Initialization function table used in main()
 */
struct init_tbl {
	int	(*init_func)();
	char	*init_msg;
};

extern struct init_tbl init0_tbl[];
extern struct init_tbl init7_tbl[];
