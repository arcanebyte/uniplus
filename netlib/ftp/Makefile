# ftp (client), ported from 2.9BSD netser/ftp (4.1c BSD with the 4.1a
# socket conversion)
CFLAGS	= -O -I. -I../include
NETDB	= ../netdb/libnetdb.a
OBJS	= main.o cmds.o cmdtab.o ftp.o compat.o ruserpass.o

all: ftp

ftp: $(OBJS) ../sockcall.o $(NETDB)
	$(CC) $(CFLAGS) -o ftp $(OBJS) ../sockcall.o $(NETDB)

main.o cmds.o ftp.o: ftp.h ftp_var.h compat.h varpat.h ../include/netdb.h
cmdtab.o: ftp_var.h varpat.h

../sockcall.o: ../sockcall.s
	cd ..; make sockcall.o

$(NETDB):
	cd ../netdb; make libnetdb.a

install: ftp
	cp ftp /usr/bin/ftp

clean:
	rm -f ftp $(OBJS)
