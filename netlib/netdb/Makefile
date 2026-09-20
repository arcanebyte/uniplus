# libnetdb.a: network data base and Internet address routines
# (4.1c BSD, from 2.9BSD's back-port), with the 4.3BSD name server
# routines (resolver) for gethostbyname and gethostbyaddr.
CFLAGS	= -O -I../include

# In link order: ld reads the archive once, so a routine comes before
# the routines it calls.  The resolver calls socket, send, receive,
# select and gethostname, so programs link with ../sockcall.o too.
OBJS	= gethnamadr.o sethostent.o res_mkquery.o res_send.o \
	  res_init.o res_comp.o \
	  getnetname.o getnetaddr.o getnetent.o \
	  getproto.o getprotnum.o getprotent.o \
	  getservnam.o getservpor.o getservent.o \
	  inet_ntoa.o inet_maddr.o inet_lnaof.o inet_netof.o \
	  inet_network.o inet_addr.o

all: libnetdb.a hosttest restest

libnetdb.a: $(OBJS)
	rm -f libnetdb.a
	ar r libnetdb.a $(OBJS)

$(OBJS): ../include/netdb.h ../include/bsd.h
gethnamadr.o sethostent.o res_mkquery.o res_send.o res_init.o res_comp.o: \
	../include/arpa/nameser.h ../include/resolv.h

hosttest: hosttest.c libnetdb.a ../sockcall.o
	$(CC) $(CFLAGS) -o hosttest hosttest.c ../sockcall.o libnetdb.a

restest: restest.c libnetdb.a ../sockcall.o
	$(CC) $(CFLAGS) -o restest restest.c ../sockcall.o libnetdb.a

../sockcall.o: ../sockcall.s
	cd ..; make sockcall.o

install: libnetdb.a
	cp ../etc/hosts ../etc/networks ../etc/protocols ../etc/services /etc
	cp ../etc/resolv.conf /etc

clean:
	rm -f $(OBJS) libnetdb.a hosttest restest
