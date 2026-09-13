INCLUDES=	-I.. -I/usr/include
CFLAGS=		-O ${INCLUDES}

DRIVERS=	prf.o\
		xp.o\
		ht.o\
		rk.o\
		rm.o\
		rl.o\
		rp.o\
		hk.o\
		ts.o\
		tm.o

../libsa.a:	SYS.o ${DRIVERS} 
		ar rv ../libsa.a $?
		ranlib ../libsa.a
