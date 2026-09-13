# Makefile 10/14/82
ALL=	tftp tftpd
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I../../net -I../../../../../include -I../../../../../src/sys
TYPE=	-n
LIBC=	../../../../../../lib/libc.a

all: ${ALL}

.c.o:
	${CC} -S ${CFLAGS} $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S

tftp:	main.o tftp.o
	${CC} main.o tftp.o ${TYPE} -o tftp -ljobs ../../net/netlib ${LIBC}

tftpd:	tftpd.o
	${CC} tftpd.o ${TYPE} -o tftpd -ljobs ../../net/netlib ${LIBC}

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

install: tftp tftpd
	install -s tftp ${DESTDIR}/usr/ucb
	install -s tftpd ${DESTDIR}/etc
