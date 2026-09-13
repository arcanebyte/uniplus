WLIB	= /lib/libw.a
GLIB	= /lib/libg.a
LIB	= -lg -lw /lib/libc.a
OBJ	= xd.o solve.o actions.o
DEBUG	=
CFLAGS	= -O -v -D$(DEBUG) 
LFLAGS	= -n -m -s

all:	/lib/libg.a xd xplot f colours cset

xd:	$(OBJ) $(WLIB) $(GLIB)
	$(CC) -m -o xd -n $(OBJ) -lg -lm -lw
	version xd

xddebug: $(OBJ) $(WLIB) $(GLIB)
	$(CC) -o xddebug -n $(OBJ) -lg -lm -lw
	version xddebug

pensize:	pensize.o
	$(CC) -o pensize pensize.o -lg
	version pensize

testplot:	testplot.o $(WLIB) $(GLIB)
	$(CC) -o testplot testplot.o -lg -lm -lw
	version testplot

arcplot:	arcplot.o $(WLIB) $(GLIB)
	$(CC) -o arcplot arcplot.o -lg -lm -lw
	version arcplot

xdtest:	solve.o xdtest.o
	$(CC) -o xdtest xdtest.o solve.o -lm

xplot:	xplot.o $(WLIB) $(GLIB)
	$(CC) -m -o xplot xplot.o -lg -lm -lw
	version xplot

cset:	cset.o $(WLIB) $(GLIB)
	$(CC) -m -o cset cset.o -lm -lw
	version cset

colours:	colours.o $(WLIB) $(GLIB)
	$(CC) -m -o colours colours.o -lg -lm -lw
	version colours

f:	f.o $(WLIB) $(GLIB)
	$(CC) -m -o f f.o -lg -lm -lw
	version f

/lib/libg.a: libg.a
	cp libg.a /lib

libg.a:	libg1.o libg2.o libg3.o libg4.o
	ar rv libg.a `lorder libg[1234].o | tsort`

libg1.o: libg.h
libg2.o: libg.h
libg3.o: libg.h
libg4.o: libg.h
xd.c:  drawdefs.h gsl.h gpl.h

