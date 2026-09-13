# %W% %G%
ALL=	ruptime rwho rwhod 
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I../../net -I../../../../../include -I../../../../../src/sys
# rwho is always seperate I&D
TYPE=	-n
LIBC=	-lndir ../../../../../../lib/libc.a

all:	${ALL}

.c.o:
	${CC} -S ${CFLAGS} $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S

ruptime.o rwho.o rwhod.o: rwhod.h

ruptime: ruptime.o
	${CC} ruptime.o ${TYPE} -o ruptime -ltermlib ../../net/netlib ${LIBC}

rwho:	rwho.o
	${CC} rwho.o -i -o rwho ../../net/netlib ${LIBC}

rwhod:	rwhod.o
	${CC} rwhod.o ${TYPE} -o rwhod -ljobs ../../net/netlib ${LIBC}

clean:
	rm -f ${ALL} *.o *.s

install: ${ALL}
	install -s rwho ${DESTDIR}/usr/ucb
	install -s rwhod ${DESTDIR}/etc
	install -s ruptime ${DESTDIR}/usr/ucb
