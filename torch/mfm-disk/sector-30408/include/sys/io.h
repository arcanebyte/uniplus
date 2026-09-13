/* @(#)io.h	6.1 */

struct size {
	daddr_t nblocks;
	int	cyloff;
};

#ifdef RP03_0
struct size rp_sizes[8] = {
	10000,	0,		/* cyl 0 thru 49 */
	71200,	50,		/* cyl 50 thru 405 */
	40600,	203,		/* cyl 203 thru 405 */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	81200,	0,		/* cyl 0 thru 405 */
};
#endif
#ifdef RP05_0
#define RP04_0
#endif
#ifdef RP04_0
struct size hp_sizes[8] = {
	18392,	0,		/* cyl 0 thru 43 */
	153406,	44,		/* cyl 44 thru 410 */
	87780,	201,		/* cyl 201 thru 410 */
	22154,	358,		/* cyl 358 thru 410 */
	0,	0,
	0,	0,
	0,	0,
	171798,	0,		/* cyl 0 thru 410 */
};
#endif
#ifdef RP06_0
struct size hp_sizes[8] = {
	18392,	0,		/* cyl 0 thru 43 */
	322278,	44,		/* cyl 44 thru 814 */
	256652,	201,		/* cyl 201 thru 814 */
	191026,	358,		/* cyl 358 thru 814 */
	125400,	515,		/* cyl 515 thru 814 */
	59774,	672,		/* cyl 672 thru 814 */
	0,	0,
	340670,	0,		/* cyl 0 thru 814 */
};
#endif
#ifdef RM80_0
struct size he_sizes[8] = {
	18228,	0,		/* cyl 0 thru 41 */
	224378,	42,		/* cyl 42 thru 558 */
	158410,	194,		/* cyl 194 thru 558 */
	92442,	346,		/* cyl 346 thru 558 */
	26474,	498,		/* cyl 498 thru 558 */
	0,	0,
	0,	0,
	242606,	0,		/* cyl 0 thru 558 */
};
#endif
#ifdef RM05_0
struct size hm_sizes[8] = {
	24320,	0,		/* cyl 0 thru 39 */
	476064,	40,		/* cyl 40 thru 822 */
	403104,	160,		/* cyl 160 thru 822 */
	330144,	280,		/* cyl 280 thru 822 */
	257184,	400,		/* cyl 400 thru 822 */
	184224,	520,		/* cyl 520 thru 822 */
	111264,	640,		/* cyl 640 thru 822 */
	500384,	0,		/* cyl 0 thru 822 */
};
#endif
#ifdef RP07_0
struct size hP_sizes[8] = {
	64000,	0,		/* cyl 0 thru 39 */
	944000,	40,		/* cyl 40 thru 630 */
	840000,	105,		/* cyl 105 thru 630 */
	672000,	210,		/* cyl 210 thru 630 */
	504000,	315,		/* cyl 315 thru 630 */
	336000,	420,		/* cyl 420 thru 630 */
	168000,	525,		/* cyl 525 thru 630 */
	1008000,	0,		/* cyl 0 thru 630 */
};
#endif
