/*
 *  FILE
 *
 *	rmt.h    redefine the system calls for the remote mag tape library
 *
 *  SCCS
 *
 *	@(#)rmt.h	1.3	9/20/85
 *
 *  SYNOPSIS
 *
 *	#ifdef RMT
 *	#include <rmt.h>
 *	#endif
 *
 *  DESCRIPTION
 *
 *	This file makes the use of remote tape drives transparent
 *	to the program that includes it.  A remote tape drive has
 *	the form system[.user]:/dev/???.
 *
 *	Note that the standard system calls are simply remapped to
 *	the remote mag tape library support routines.
 *
 *	Also, note that if <sys/stat.h> is included, it must be AFTER
 *	this file to get the stat structure name remapped.
 *
 */

#ifndef access		/* avoid multiple redefinition */

#define access rmtaccess
#define close rmtclose
#define creat rmtcreat
#define dup rmtdup
#define fcntl rmtfcntl
#define fstat rmtfstat
#define ioctl rmtioctl
#define isatty rmtisatty
#define lseek rmtlseek
#define lstat rmtlstat
#define open rmtopen
#define read rmtread
#define stat rmtstat
#define write rmtwrite

extern int rmtaccess ();
extern int rmtclose ();
extern int rmtcreat ();
extern int rmtdup ();
extern int rmtfcntl ();
extern int rmtfstat ();
extern int rmtioctl ();
extern int rmtisatty ();
extern long rmtlseek ();
extern int rmtlstat ();
extern int rmtopen ();
extern int rmtread ();
extern int rmtstat ();
extern int rmtwrite ();

#endif
