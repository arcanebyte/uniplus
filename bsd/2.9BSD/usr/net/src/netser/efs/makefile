ALL=	efsinit efsd
CFLAGS=-O -DDEBUG

all: ${ALL}

.c.o:
	cc -S ${CFLAGS} $*.c
	sed -f asm.sed $*.s | as -o $*.o
	rm -f $*.s

efsd: efsd.o
	cc efsd.o -o efsd ../raddr.o -ljobs

efsinit: efsinit.o
	cc efsinit.o -o efsinit

clean:
	rm -f ${ALL} *.o *.s

install: ${ALL}
	cp efsd efsinit ${DESTDIR}/etc
	chown root ${DESTDIR}/etc/efsd
	chmod 4755 ${DESTDIR}/etc/efsd
	chown root ${DESTDIR}/etc/efsinit
	chmod 4755 ${DESTDIR}/etc/efsinit
