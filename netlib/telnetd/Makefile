# telnetd, ported from 2.9BSD netser/telnet (4.1c BSD)
CFLAGS	= -O -I../include
NETDB	= ../netdb/libnetdb.a

all: telnetd

telnetd: telnetd.o ../sockcall.o $(NETDB)
	$(CC) $(CFLAGS) -o telnetd telnetd.o ../sockcall.o $(NETDB)

telnetd.o: telnetd.c telnet.h ../include/netdb.h

../sockcall.o: ../sockcall.s
	cd ..; make sockcall.o

$(NETDB):
	cd ../netdb; make libnetdb.a

install: telnetd
	cp telnetd /etc/telnetd
	sh mkptys

clean:
	rm -f telnetd telnetd.o
