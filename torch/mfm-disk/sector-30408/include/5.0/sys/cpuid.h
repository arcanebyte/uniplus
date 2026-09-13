/*	@(#)cpuid.h UniPlus+ v.0.1.1 */
/*
 * Global CPU definitions for multiple processors and
 *	multiple configurations.
 */

/*
 * Specific CPU identifiers
 */
#define CPU_MC68000	1
#define CPU_NA16000	2
#define CPU_VAX11780	3
#define CPU_VAX11750	4
#define CPU_VAX11730	5
#define CPU_PDP1170	6

/*
 * CPU version identifiers
 */
	/* CPU_MC68000 */
#define VER_MC68000	1
#define VER_MC68010	2
#define VER_MC68020	3

/*
 * MMU identifiers
 */
#define MMU_NONE	1
#define MMU_SUN		2
#define MMU_68451	3
#define MMU_PIX		4
#define MMU_APPLE	5
