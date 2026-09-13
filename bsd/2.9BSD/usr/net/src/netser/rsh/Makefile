# %W% %G%
ALL=	rsh rshd 
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I../../net -I../../../../../include -I../../../../../src/sys
TYPE=	-n
LIBC=	../../../../../../lib/libc.a

all:	${ALL}

.c.o:
	${CC} -S ${CFLAGS} $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S


rsh:	rsh.o
	${CC} rsh.o ${TYPE} -o rsh -ljobs ../../net/netlib ${LIBC}

rshd:	rshd.o
	${CC} rshd.o ${TYPE} -o rshd -ljobs ../../net/netlib ${LIBC}

clean:
	rm -f ${ALL} *.o *.s

install: ${ALL}
	install -s rsh ${DESTDIR}/usr/ucb
	chown root ${DESTDIR}/usr/ucb/rsh
	chmod 4755 ${DESTDIR}/usr/ucb/rsh
	install -s rshd ${DESTDIR}/etc
