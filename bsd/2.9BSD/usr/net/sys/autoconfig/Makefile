#
# Makefile for the system autoconfiguration program
#

CFILES=		args.c attach.c do_config.c main.c read_dtab.c read_nlist.c \
		uprobe1.c uprobe2.c
OBJS=		args.o attach.o do_config.o main.o read_dtab.o read_nlist.o \
		uprobe1.o uprobe2.o
CFLAGS=		-O
LDFLAGS=
INCLUDES=	/usr/include

all:		autoconfig

autoconfig:	$(OBJS)
		cc $(LDFLAGS) $(OBJS) -o autoconfig

cp:		all
		cp autoconfig ${DESTDIR}/etc
		-strip ${DESTDIR}/etc/autoconfig

clean:
		rm -f autoconfig *.o

args.o:		${INCLUDES}/stdio.h
args.o:		args.h
attach.o:	${INCLUDES}/a.out.h
attach.o:	${INCLUDES}/stdio.h
attach.o:	${INCLUDES}/sys/autoconfig.h
attach.o:	${INCLUDES}/sys/psw.h
attach.o:	dtab.h
attach.o:	ivec.h
do_config.o:	${INCLUDES}/a.out.h
do_config.o:	${INCLUDES}/errno.h
do_config.o:	${INCLUDES}/stdio.h
do_config.o:	${INCLUDES}/sys/autoconfig.h
do_config.o:	${INCLUDES}/sys/psw.h
do_config.o:	${INCLUDES}/sys/trap.h
do_config.o:	dtab.h
do_config.o:	ivec.h
main.o:		${INCLUDES}/stdio.h
main.o:		${INCLUDES}/sys/autoconfig.h
main.o:		args.h
read_dtab.o:	${INCLUDES}/ctype.h
read_dtab.o:	${INCLUDES}/stdio.h
read_dtab.o:	${INCLUDES}/sys/autoconfig.h
read_dtab.o:	dtab.h
read_dtab.o:	uprobe.h
read_nlist.o:	${INCLUDES}/a.out.h
read_nlist.o:	${INCLUDES}/stdio.h
read_nlist.o:	${INCLUDES}/sys/autoconfig.h
read_nlist.o:	${INCLUDES}/sys/param.h
read_nlist.o:	args.h
read_nlist.o:	dtab.h
uprobe1.o:	${INCLUDES}/sys/autoconfig.h
uprobe1.o:	${INCLUDES}/sys/hkreg.h
uprobe1.o:	${INCLUDES}/sys/hpreg.h
uprobe1.o:	${INCLUDES}/sys/hsreg.h
uprobe1.o:	${INCLUDES}/sys/htreg.h
uprobe1.o:	${INCLUDES}/sys/param.h
uprobe1.o:	${INCLUDES}/sys/rkreg.h
uprobe1.o:	${INCLUDES}/sys/rlreg.h
uprobe1.o:	${INCLUDES}/sys/rpreg.h
uprobe1.o:	${INCLUDES}/sys/tmreg.h
uprobe1.o:	${INCLUDES}/sys/tsreg.h
uprobe1.o:	uprobe.h
uprobe2.o:	${INCLUDES}/sgtty.h
uprobe2.o:	${INCLUDES}/sys/autoconfig.h
uprobe2.o:	${INCLUDES}/sys/dhreg.h
uprobe2.o:	${INCLUDES}/sys/dnreg.h
uprobe2.o:	${INCLUDES}/sys/dzreg.h
uprobe2.o:	${INCLUDES}/sys/klreg.h
uprobe2.o:	${INCLUDES}/sys/lpreg.h
uprobe2.o:	${INCLUDES}/sys/param.h
uprobe2.o:	${INCLUDES}/sys/vpreg.h
uprobe2.o:	uprobe.h
