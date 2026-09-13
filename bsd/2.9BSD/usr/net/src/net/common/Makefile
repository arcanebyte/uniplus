# @(#)Makefile	4.5 10/12/82
OBJS=	gethostnam.o gethostadd.o gethostent.o \
	getnetaddr.o getnetent.o getnetname.o \
	getproto.o getprotent.o getprotnum.o \
	getservent.o getservpor.o getservnam.o \
	raddr.o rcmd.o rexec.o rhost.o ruserpass.o
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
