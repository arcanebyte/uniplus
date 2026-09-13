/*	@(#)symbol.h	2.1	 */
/*	3.0 SID #	1.1	*/
/*
 * Structure of a symbol table entry
 */

struct	symbol {
	char	sy_name[8];
	char	sy_type;
	int	sy_value;
};
