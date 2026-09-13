/*
 * information needed by sysfail processing
 */

#define MAXMMUCPU	1
#define MAXSYSCON	1
#define MAXDRAM		16
#define MAXSASI		8
#define MAXFDC		4
#define MAXSIO		4

struct motherboard {
	caddr_t mmucpu[MAXMMUCPU];
	caddr_t syscon[MAXSYSCON];
	caddr_t dram[MAXDRAM];
	caddr_t sasi[MAXSASI];
	caddr_t fdc[MAXFDC];
	caddr_t sio[MAXSIO];
};

extern struct motherboard motherboard;
