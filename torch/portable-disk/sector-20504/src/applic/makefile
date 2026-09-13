WLIB	= /lib/libw.a
LIB	=  -lw -lm 
INCLUDE =/usr/include
DEBUG   =
CFLAGS	=-v -O -D$(DEBUG) -DTORCH
LFLAGS	= -m -n -s
HFILES	= $(INCLUDE)/mmi.h $(INCLUDE)/event.h $(INCLUDE)/edit.h \
	$(INCLUDE)/menu.h $(INCLUDE)/window.h $(INCLUDE)/zoom.h \
	$(INCLUDE)/eioctl.h $(INCLUDE)/icon.h

DISTRIB	= chuffer calc yawn

all:	$(DISTRIB)

version:
	version	$(DISTRIB)

# DISTRIBUTION PROGRAMS

yawn:	yawn.o $(WLIB)
	$(CC) $(LFLAGS) yawn.o $(LIB) -o yawn 
	version yawn

calc:	calc.o $(WLIB)
	$(CC) $(LFLAGS) calc.o $(LIB) -o calc
	version calc

chuffer:	chuffer.o $(WLIB)
	$(CC) $(LFLAGS) chuffer.o $(LIB) -o chuffer
	version chuffer
