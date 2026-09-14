# ucb: Berkeley versions of commands, installed beside the System V ones
# (ls7 and rm7, as the 1984 root .cshrc expects for ls).
CFLAGS	= -O

all: ls7 rm7

ls7: ls7.o ndir.o
	$(CC) $(CFLAGS) -o ls7 ls7.o ndir.o

ls7.o: ls7.c ndir.h

rm7: rm7.o
	$(CC) $(CFLAGS) -o rm7 rm7.o

install: ls7 rm7
	cp ls7 rm7 /usr/bin

clean:
	rm -f ls7 rm7 *.o
