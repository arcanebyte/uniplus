CFLAGS = -O -s

all:	rinstall installd

rinstall: rinstall.o /usr/src/cmd/netser/rcmd.o
	cc $(CFLAGS) rinstall.o /usr/src/cmd/netser/rcmd.o -o rinstall

installd: installd.o
	cc $(CFLAGS) installd.o -o installd

cp:	rinstall installd
	cp rinstall /usr/local/rinstall
	cp installd /etc/installd

clean:
	rm -f *.o rinstall installd

cmp:	cmp rinstall /usr/local/rinstall
	cmp installd /usr/local/installd
