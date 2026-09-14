# telnet, ported from 2.9BSD netser/telnet (4.1c BSD)
CFLAGS	= -O -I../include
NETDB	= ../netdb/libnetdb.a

all: telnet

telnet: telnet.o ../sockcall.o $(NETDB)
	$(CC) $(CFLAGS) -o telnet telnet.o ../sockcall.o $(NETDB)

telnet.o: telnet.c telnet.h ../include/netdb.h

../sockcall.o: ../sockcall.s
	cd ..; make sockcall.o

$(NETDB):
	cd ../netdb; make libnetdb.a

install: telnet
	cp telnet /usr/bin/telnet

clean:
	rm -f telnet telnet.o
