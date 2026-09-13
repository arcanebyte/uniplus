INCLUDES=	-I/usr/include -I..
CFLAGS =	-O ${INCLUDES}

../boot:	boot.o M.o ubmapset.o ../libsa.a ../c.o
		ld -o ../boot M.o ubmapset.o boot.o ../c.o ../libsa.a -lnonfpc
