# @(#)Makefile	4.3 10/14/82
ALL=	telnet telnetd
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I../../net -I../../../../../include -I../../../../../src/sys
TYPE=	-n
LIBC=	../../../../../../lib/libc.a

all:	${ALL}

.c.o:
	${CC} -S ${CFLAGS} $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S

telnet:	telnet.o
	${CC} telnet.o ${TYPE} -o telnet -ljobs ../../net/netlib ${LIBC}

telnetd: telnetd.o
	${CC} telnetd.o ${TYPE} -o telnetd -ljobs ../../net/netlib ${LIBC}

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

install: ${ALL}
	install -s telnet ${DESTDIR}/usr/ucb
	install -s telnetd ${DESTDIR}/etc
