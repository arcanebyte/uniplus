# @(#)Makefile	4.3 10/14/82
OBJS=	host.o inet.o if.o main.o mbuf.o route.o
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I../../net -I../../../../../src/sys/TEKARCH -I../../../../../include -I../../../../../src/sys -I../../../../../src/ucb/net
LIBC=	../../../../../../lib/libc.a
DESTDIR=/usr/ucb
TYPE=	-n

.c.o:
	${CC} -S ${CFLAGS} $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S

netstat: ${OBJS}
	${CC} ${OBJS} ${TYPE} -o netstat ../../net/netlib ${LIBC}

install: netstat
	install -s netstat ${DESTDIR}/usr/ucb

clean:
	rm -f *.o core a.out errs
