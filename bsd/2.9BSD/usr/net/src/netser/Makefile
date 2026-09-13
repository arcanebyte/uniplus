# @(#)Makefile	4.2 10/10/82
ALL=	ftp implog misc netstat rlogin routed rsh rwho telnet tftp 

all:	${ALL}

${ALL}:	/tmp
	cd $@; make ${MFLAGS}

install:
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} install); done

clean:
	rm -f a.out core *.s *.o errs
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} clean); done
