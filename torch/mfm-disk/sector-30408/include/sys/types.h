/*
 * @(#)ROOT	types.h	1.2	87/04/10	09:23:04
 * @(#)UniSoft	types.h	NFSSRC	2.2
 */

#ifndef _TYPES_
#define _TYPES_
typedef struct { int r[1]; } *	physadr;
typedef	long		daddr_t;
typedef	char *		caddr_t;
typedef	unsigned int	uint;
typedef	unsigned short	ushort;
typedef	unsigned long	ulong;
typedef	unsigned char	uchar_t;
typedef	ushort		ino_t;
typedef short		cnt_t;
typedef	long		time_t;
typedef	int		label_t[13];
typedef	short		dev_t;
typedef	long		off_t;
typedef	long		paddr_t;
typedef	long		key_t;
typedef	unsigned char	u_char;
typedef	unsigned char	unchar;
typedef	unsigned short	u_short;
typedef	unsigned long	u_long;
typedef unsigned int	u_int;
typedef	long		ubadr_t;	/* physical unibus address */
typedef struct  fd_set { long fds_bits[1]; } fd_set;
#endif	_TYPES_
