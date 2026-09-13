#include <sys/types.h>
/*
 * copy count bytes from from to to.
 */
bcopy(from, to, count)
register caddr_t from, to;
register count;
{
	if (count == 0)
		return;
	if((from|to|count)&1)
		do	/* copy by bytes */
			*to++ = *from++;
		while(--count);
	else {		/* copy by words */
		count >>= 1;
		do
			*((short *)to)++ = *((short *)from)++;
		while(--count);
	}
}
