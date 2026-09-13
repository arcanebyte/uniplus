/*	@(#)bcmp.c	2.1	SCCS id keyword	*/
/*
 * Compare strings (at most n bytes):  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

bcmp(s1, s2, n)
register char *s1, *s2;
register n;
{

	while (--n >= 0 && *s1++ == *s2++)
		continue;
	return(n<0 ? 0 : *--s1 - *--s2);
}
