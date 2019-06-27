/*
 * Copyright 1983 UniSoft Corporation
 * Use of this material is subject to your disclosure agreement
 * with UniSoft Systems of Berkeley.
 *
 * 	mkser (buf)
 * Given a pointer to a buffer to stick the results in this routine
 * gets the serial number in raw format and converts to a 13 byte ascii
 * passwd sequence.
 * NOTE that this is the only machine dependant routine in all of the serial
 * number code.
 */
#include <stdio.h>
mkser (buf)
	char *buf;
{
	unsigned long  v, v1, v2;
	register char *lp, *cp;
	int i;
	char *cptr, *sn();
	char b[20];
	char *crypt();

	cptr = sn();			/* get the REAL serial number */
	if (!cptr) {
		printf("Couldn't get the serial #\n");
		while(1);
	}
	lp = buf;			/* resultant string buffer */
	cp = cptr;			/* nibble format serial # */

	for (i=5; i>0; i--) {
		*lp = (*cp++ << 4) & 0xF0;
		*lp++ |= (*cp++ & 0xF);
	}
	*lp = (*cp++ << 4) & 0xF0;
	cp += 3;
	for (i=5; i>0; i--) {
		*lp++ |= (*cp++ & 0xF);
		*lp = (*cp++ << 4) & 0xF0;
	}
	*lp++ |= (*cp++ & 0xF);

		/* Now convert BCD serial # into binary.
		 * Two 32 bit words + one 12 bit word
		 */
	v = 0;
	for (cp=buf; cp<buf+4; cp++) {
		v = v * 10 + ((*cp >> 4) & 0xF);  
		v = v * 10 + (*cp & 0xF);  
	}
	v = v * 10 + ((*cp >> 4) & 0xF);  
	v1 = v;

	v = *cp++ & 0xF;
	while (cp<buf+9) {
		v = v * 10 + ((*cp >> 4) & 0xF);  
		v = v * 10 + (*cp & 0xF);  
		cp++;
	}
	v2 = v;

	v = 0;
	while (cp<buf+11) {
		v = v * 10 + ((*cp >> 4) & 0xF);  
		v = v * 10 + (*cp & 0xF);  
		cp++;
	}

	v1 <<= 2;
	v1 |= v & 3;
	v >>= 2;
	v2 <<= 2;
	v2 |= v & 3;
	v >>= 2;
	v &= 0xFFF;

	cp = buf;
	for (i=0; i<4; i++) {
		*cp++ = v1 & 0xFF;
		v1 >>= 8;
	}
	for (i=0; i<4; i++) {
		*cp++ = v2 & 0xFF;
		v2 >>= 8;
	}

		/* use the 12 bit word as the salt to encrypt the two
		 * 32 bit words treated as 8 characters.
		 * Return the resultant 13 byte sting.
		 */
	cvtcy((long)v, b, 2);			/* convert to two ascii chars */
	cp = crypt(buf, b);		/* encrypt serial number */
	lp = buf;
	for (i=0; i<13; i++)
		lp[i] = *cp++;
	lp[i] = 0;
}
