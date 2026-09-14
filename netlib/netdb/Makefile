# libnetdb.a: network data base and Internet address routines
# (4.1c BSD, from 2.9BSD's back-port), for the ported network programs.
CFLAGS	= -O -I../include

# In link order: ld reads the archive once, so a routine comes before
# the routines it calls.
OBJS	= gethostnam.o gethostadd.o gethostent.o \
	  getnetname.o getnetaddr.o getnetent.o \
	  getproto.o getprotnum.o getprotent.o \
	  getservnam.o getservpor.o getservent.o \
	  inet_ntoa.o inet_maddr.o inet_lnaof.o inet_netof.o \
	  inet_network.o inet_addr.o

all: libnetdb.a hosttest

libnetdb.a: $(OBJS)
	rm -f libnetdb.a
	ar r libnetdb.a $(OBJS)

$(OBJS): ../include/netdb.h ../include/bsd.h

hosttest: hosttest.c libnetdb.a
	$(CC) $(CFLAGS) -o hosttest hosttest.c libnetdb.a

install: libnetdb.a
	cp ../etc/hosts ../etc/networks ../etc/protocols ../etc/services /etc

clean:
	rm -f $(OBJS) libnetdb.a hosttest
