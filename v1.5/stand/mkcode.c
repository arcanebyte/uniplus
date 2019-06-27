/* These routines implement a simple software protection scheme.
 *
 * mkcode (pw, iv)
 *	-- gets passed a 13 character string like those kept in the
 *	password file.  This string is considered the serial number.
 *	It encrypts the string combining a keyword with it.  This is so
 *	the authorization program has a little work to do in taking the serial
 *	number apart.
 *	-- iv is a pointer to three long ints where the results are left.
 *
 * chkser (pw, rv)
 *	-- where pw is the original string passed to mkcode.
 *	-- where rv is the returned authorization code (an integer)
 * 	Returns zero if the decoded rv is correct, 1 otherwise.
 *
 */
#include <stdio.h>

#define ROTORSZ 256
#define MASK 0377
extern char	t1[ROTORSZ];
extern char	t2[ROTORSZ];
extern char	t3[ROTORSZ];

static char w[] = "0xFCD001";

chkser(pw, rv)
	char *pw;
	unsigned long rv;
{
	int i, n1;
	char sa[4];
	char c, *cp, *crypt();

	sa[0] = pw[3];
	sa[1] = pw[6];
	sa[2] = 0;
/* printf("W=`%s' SALT=`%s', PW=`%s'\n",w,sa,pw+2); */
	cp = crypt(w, sa);
/* printf("NEW KEY = `%s'\n",cp); */
	_setup(cp);
	cp = (char *)&rv;
	n1 = 0;
	for (i=0; i<4; i++) {
		c = t2[t3[t1[(*cp+n1)&MASK]&MASK]&MASK]-n1;
/* printf("cp[%d]=%x -> %x",i,*cp&0xFF,c&0xFF); */
		if (c != pw[i+2]){
/* printf("FAILED (on char %d)\n",i); */
			return i+1;
		}
		n1++;
		cp++;
	}
/* printf("OK\n"); */
	return 0;
}

mkcode(pw, iv)
	char *pw;
	unsigned long *iv;
{
	char sa[4];
	static char out[30];
	static char tbuf[30];
	register int n1, i, j;
	register char *cp;
	unsigned long v;

		/* Now use the encrypted serial number as the salt and
		 * `0xFCD001' as the password to setup the crypt algorithm
		 * for encoding the magic word and serial number.
		 */
	sa[0] = (pw[5] & 0xF) + 'a';
	sa[1] = pw[1];
	sa[2] = 0;
	pw += 2;

		/* build the string to transmit, wwwwsssssssssssx, where
		 * wwww is the first 4 characters of the magic word,
		 * sssssssssss is the 11 relevant characters of the serial #,
		 * x is the low order salt character used in priming the
		 * crypt routine.
		 */
	cp = out;
	for (i=0; i<4; i++)
		*cp++ = w[i];
	for (i=0; i<11; i++)
		*cp++ = *pw++;
	*cp++ = 'a';
	cvtbin(tbuf, out, 16);

		/* Now tbuf contains 11 bytes of data to encode.
		 * it is then converted into 3 longs for transmission to
		 * to the host as dates.
		 */
	_setup(crypt(w, sa));
	n1 = 0;
	cp = tbuf;
	for (j=0; j<3; j++) {
	    v = 0;
	    for (i=0; i<4; i++) {
		v <<= 8;
		v |=(t2[t3[t1[(*cp++ + n1)&MASK]&MASK]&MASK]-n1)&0xFF;
		n1++;
	    }
	    iv[j] = v;
	}
	iv[2] &= ~0xFF;
	iv[2] |= sa[1];
}
