# Build on UniPlus+ V.1.5+ booted from unix.net
CFLAGS	= -O -I./include
AS	= as

all: tcpconn tcpecho

tcpconn: tcpconn.o sockcall.o
	$(CC) $(CFLAGS) -o tcpconn tcpconn.o sockcall.o

tcpconn.o: tcpconn.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c tcpconn.c

tcpecho: tcpecho.o sockcall.o
	$(CC) $(CFLAGS) -o tcpecho tcpecho.o sockcall.o

tcpecho.o: tcpecho.c include/net/socket.h include/net/in.h
	$(CC) $(CFLAGS) -c tcpecho.c

sockcall.o: sockcall.s
	$(AS) -o sockcall.o sockcall.s

clean:
	rm -f tcpconn tcpecho *.o
