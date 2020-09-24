OBJECTS = xmodem.o getput.o misc.o send.o receive.o batch.o
CFLAGS = -O

xmodem: $(OBJECTS)
	cc $(CFLAGS) $(OBJECTS) -o xmodem

$(OBJECTS): xmodem.h

print: 
	lpr -p -Pvmslp xmodem.h xmodem.c getput.c receive.c send.c batch.c \
	misc.c Makefile update.doc README xmodem.1 getput.sysv.c

shar:
	shar README update.doc Makefile xmodem.1 xmodem.h xmodem.c batch.c > xmodem.shar.1
	shar getput.c getput.sysv.c send.c > xmodem.shar.2
	shar receive.c misc.c tip.diffs > xmodem.shar.3
