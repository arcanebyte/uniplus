# tftp, ported from 2.9BSD netser/tftp (4.1c BSD)
CFLAGS	= -O -I../include
NETDB	= ../netdb/libnetdb.a

all: tftp

tftp: main.o tftp.o ../sockcall.o $(NETDB)
	$(CC) $(CFLAGS) -o tftp main.o tftp.o ../sockcall.o $(NETDB)

main.o tftp.o: tftp.h ../include/netdb.h ../include/bsd.h

../sockcall.o: ../sockcall.s
	cd ..; make sockcall.o

$(NETDB):
	cd ../netdb; make libnetdb.a

install: tftp
	cp tftp /usr/bin/tftp

clean:
	rm -f tftp main.o tftp.o
