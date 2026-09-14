# netstat, ported from 2.9BSD netser/netstat (4.1c BSD)
#
# Built against the kernel's own headers so that the structures read from
# /dev/kmem match the running kernel: KINC is where the kernel was built
# from (lisa-build.md puts v1.5/include at /usr/src/include).
KINC	= /usr/src/include
CFLAGS	= -O -I$(KINC) -I../include
NETDB	= ../netdb/libnetdb.a
OBJS	= main.o inet.o if.o route.o mbuf.o

all: netstat

netstat: $(OBJS) $(NETDB)
	$(CC) $(CFLAGS) -o netstat $(OBJS) $(NETDB)

$(NETDB):
	cd ../netdb; make libnetdb.a

install: netstat
	cp netstat /usr/bin/netstat

clean:
	rm -f netstat $(OBJS)
