# Build on UniPlus+ V.1.5+ booted from unix.net
CFLAGS	= -O -I./include
AS	= as

all: tcpconn tcpecho looptest udptest netinfo

tcpconn: tcpconn.o sockcall.o
	$(CC) $(CFLAGS) -o tcpconn tcpconn.o sockcall.o

tcpconn.o: tcpconn.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c tcpconn.c

tcpecho: tcpecho.o sockcall.o
	$(CC) $(CFLAGS) -o tcpecho tcpecho.o sockcall.o

tcpecho.o: tcpecho.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c tcpecho.c

looptest: looptest.o sockcall.o
	$(CC) $(CFLAGS) -o looptest looptest.o sockcall.o

looptest.o: looptest.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c looptest.c

udptest: udptest.o sockcall.o
	$(CC) $(CFLAGS) -o udptest udptest.o sockcall.o

udptest.o: udptest.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c udptest.c

netinfo: netinfo.o sockcall.o
	$(CC) $(CFLAGS) -o netinfo netinfo.o sockcall.o

netinfo.o: netinfo.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c netinfo.c

sockcall.o: sockcall.s
	$(AS) -o sockcall.o sockcall.s

clean:
	rm -f tcpconn tcpecho looptest udptest netinfo *.o
