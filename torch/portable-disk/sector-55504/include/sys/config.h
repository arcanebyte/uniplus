/*
 * Copyright 1982 UniSoft Corporation
 */

/*
 * Configuration constants and definitions for the 451 MMU
 */

#define BOOTADDR	0x1000		/* PROM reboot address */

/*
 * Configuration constants for the CASE board
 */
#define	VERSAMEM	0x200000	/* Start of Versabus memory */
#define	isversa(a)	((long)(a)>=VERSAMEM)
