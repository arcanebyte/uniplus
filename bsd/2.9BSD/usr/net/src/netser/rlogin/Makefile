# %W% %G%
ALL=	rlogin rlogind
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I../../net -I../../../../../include -I../../../../../src/sys
LIBC=	../../../../../../lib/libc.a
TYPE=	-n

all:	${ALL}

.c.o:
	${CC} -S ${CFLAGS} $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S

rlogin:	rlogin.o
	${CC} rlogin.o ${TYPE} -o rlogin ../../net/netlib ${LIBC}

rlogind: rlogind.o
	${CC} rlogind.o ${TYPE} -o rlogind -ljobs ../../net/netlib ${LIBC}

clean:
	rm -f ${ALL} *.o *.s

install: ${ALL}
	install -s rlogin ${DESTDIR}/usr/ucb
	chown root ${DESTDIR}/usr/ucb/rlogin
	chmod 4755 ${DESTDIR}/usr/ucb/rlogin
	install -s rlogind ${DESTDIR}/etc
