#	@(#)Makefile	4.2	1/15/83
#
ALL=	ftp ftpd
DESTDIR=
INCLUDE = -I../../net -I../../../../../include
INC	= ../../../../../include
LIBC	= ../../net/netlib ../../../../../../lib/libc.a
CFLAGS=	-O -I. ${INCLUDE} -I../../../../../src/sys -DTCP4_1a -DMENLO_JCL -DUCB_NET
TYPE=	-n
SETIDS= fakedir.o setreuid.o setregid.o
LD= ld

all:	${ALL}

ftp:	cmds.o cmdtab.o ftp.o main.o compat.o
	${CC} ${TYPE} cmds.o cmdtab.o ftp.o main.o compat.o -o ftp -ljobs ${LIBC}

ftp.o:	ftp.h ftp_var.h
cmds.o:	ftp.h ftp_var.h
cmdtab.o: ftp_var.h
ftp.o:	ftp_var.h
main.o: ftp_var.h

ftp.h: compat.h
	touch ftp.h
ftpvar.h: varpat.h
	touch ftpvar.h

ftpd:	ftpd.o ftpcmd.o vers.o glob.o compat.o ${SETIDS}
	${CC} ${TYPE} ftpd.o ftpcmd.o glob.o compat.o ${SETIDS}\
	 -o ftpd vers.o -ljobs -lndir ${LIBC}

vers.o:	ftpd.c ftpcmd.y
	sh newvers.sh
	${CC} -c vers.c

setreuid.o: setreuid.s
	${AS} -o setreuid.o ${INC}/sys.s setreuid.s
	-${LD} -r -x setreuid.o
	mv a.out setreuid.o

setregid.o: setregid.s
	${AS} -o setregid.o ${INC}/sys.s setregid.s
	-${LD} -r -x setregid.o
	mv a.out setregid.o

install:
	install -s ftp ${DESTDIR}/usr/ucb/ftp
	install -s ftpd ${DESTDIR}/etc/ftpd

clean:
	rm -f ${ALL} *.o errs core a.out t.? yacc.* y.tab.c
