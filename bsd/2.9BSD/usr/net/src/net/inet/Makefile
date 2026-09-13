# @(#)Makefile	4.4 10/12/82
OBJS=	inet_addr.o inet_network.o \
	inet_netof.o inet_lnaof.o \
	inet_maddr.o
CFLAGS=	-DUCB_NET -DMENLO_JCL -O -I.. -I../../../../../include -I../../../../../src/sys

.c.o:
	${CC} -S ${CFLAGS} -c $*.c
	sed -f ../asm.sed $*.s > $*.S
	as - -o $*.o $*.S
	rm -f $*.s $*.S
	-ld -x -r $*.o
	mv a.out $*.o

all:	${OBJS}
	ar cru ../netlib ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o errs a.out core
