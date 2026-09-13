/*
 * Header file for argument parser
 */

extern char _bf[],*_sf[],**_argspace,**_ap, *newstr();
extern int nargs;

#define bools(x) _bf[x >= 'a' ? x - 'a' : x - 'A']
#define strings(x) _sf[x >= 'a' ? x - 'a' : x - 'A']
#define next_arg() (*_ap++)
#define rew_arg() _ap = _argspace
