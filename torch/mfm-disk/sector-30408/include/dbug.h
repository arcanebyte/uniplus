/************************************************************************
 *									*
 *			Copyright (c) 1984, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released into the		*
 *	public domain for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	dbug.h    user include file for programs using the dbug package
 *
 *  SYNOPSIS
 *
 *	#include <dbug.h>
 *
 *  SCCS ID
 *
 *	@(#)dbug.h	1.3 12/31/84
 *
 *  DESCRIPTION
 *
 *	Programs which use the dbug package must include this file.
 *	It contains the appropriate macros to call support routines
 *	in the dbug runtime library.
 *
 *	To disable compilation of the macro expansions define the
 *	preprocessor symbol "DBUG_OFF".  This will result in null
 *	macros expansions so that the resulting code will be smaller
 *	and faster.  (The difference may be smaller than you think
 *	so this step is recommended only when absolutely necessary).
 *	In general, tradeoffs between space and efficiency are
 *	decided in favor of efficiency since space is seldom a
 *	problem on the new machines).
 *
 *	All externally visible symbol names follow the pattern
 *	"_db_xxx..xx_" to minimize the possibility of a dbug package
 *	symbol colliding with a user defined symbol.
 *	
 *	Because the C preprocessor will not accept macros with a variable
 *	number of arguments, there are separate DBUG_<N> macros for
 *	cases N = {0,1,...NMAX}.  NMAX is currently 5.
 *
 *  AUTHOR
 *
 *	Fred Fish
 *	(Currently employed by UniSoft Systems, Berkeley, Ca.)
 *
 */


/*
 *	Internally used dbug variables which must be global.
 */

#ifndef DBUG_OFF
    extern int _db_on_;			/* TRUE if debug currently enabled */
    extern FILE *_db_fp_;		/* Current debug output stream */
    extern char *_db_process_;		/* Name of current process */
    extern int _db_keyword_ ();		/* Accept/reject keyword */
    extern void _db_push_ ();		/* Push state, set up new state */
    extern void _db_pop_ ();		/* Pop previous debug state */
    extern void _db_enter_ ();		/* New user function entered */
    extern void _db_return_ ();		/* User function return */
    extern void _db_printf_ ();		/* Print debug output */
# endif


/*
 *	These macros provide a user interface into functions in the
 *	dbug runtime support library.  They isolate users from changes
 *	in the MACROS and/or runtime support.
 *
 *	The symbols "__LINE__" and "__FILE__" are expanded by the
 *	preprocessor to the current source file line number and file
 *	name respectively.
 *
 *	WARNING ---  Because the DBUG_ENTER macro allocates space on
 *	the user function's stack, it must precede any executable
 *	statements in the user function.
 *
 */

# ifdef DBUG_OFF
#    define DBUG_ENTER(a1)
#    define DBUG_RETURN(a1) return(a1)
#    define DBUG_VOID_RETURN return
#    define DBUG_EXECUTE(keyword,a1)
#    define DBUG_2(keyword,format)
#    define DBUG_3(keyword,format,a1)
#    define DBUG_4(keyword,format,a1,a2)
#    define DBUG_5(keyword,format,a1,a2,a3)
#    define DBUG_PUSH(a1)
#    define DBUG_POP()
#    define DBUG_PROCESS(a1)
#    define DBUG_FILE (stderr)
# else
#    define DBUG_ENTER(a) \
	auto char *_db_func_, *_db_file_; \
	int _db_level_; \
	_db_enter_ (a,__FILE__,__LINE__,&_db_func_,&_db_file_,&_db_level_)
#    define DBUG_LEAVE \
	(_db_return_ (__LINE__, &_db_func_, &_db_file_, &_db_level_))
#    define DBUG_RETURN(a1) return (DBUG_LEAVE, (a1))
/*   define DBUG_RETURN(a1) {DBUG_LEAVE; return(a1);}  Alternate form */
#    define DBUG_VOID_RETURN {DBUG_LEAVE; return;}
#    define DBUG_EXECUTE(keyword,a1) \
	if (_db_on_) {if (_db_keyword_ (keyword)) { a1 }}
#    define DBUG_2(keyword,format) \
	if (_db_on_) {_db_printf_ (__LINE__, keyword, format);}
#    define DBUG_3(keyword,format,a1) \
	if (_db_on_) {_db_printf_ (__LINE__, keyword, format, a1);}
#    define DBUG_4(keyword,format,a1,a2) \
	if (_db_on_) {_db_printf_ (__LINE__, keyword, format, a1, a2);}
#    define DBUG_5(keyword,format,a1,a2,a3) \
	if (_db_on_) {_db_printf_ (__LINE__, keyword, format, a1, a2, a3);};
#    define DBUG_PUSH(a1) _db_push_ (a1)
#    define DBUG_POP() _db_pop_ ()
#    define DBUG_PROCESS(a1) (_db_process_ = a1)
#    define DBUG_FILE (_db_fp_)
# endif
