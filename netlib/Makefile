# Build on UniPlus+ V.1.5+ booted from unix.net
CFLAGS	= -O -I./include
AS	= as

tcpconn: tcpconn.o sockcall.o
	$(CC) $(CFLAGS) -o tcpconn tcpconn.o sockcall.o

tcpconn.o: tcpconn.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c tcpconn.c

sockcall.o: sockcall.s
	$(AS) -o sockcall.o sockcall.s

clean:
	rm -f tcpconn *.o
