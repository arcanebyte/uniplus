#
#	Standalone Makefile
#
#	Note that there are limitations on how large a program may
#	be loaded along with all device drivers.  This is especially
#	a problem with restor. 
#	Programs should be <= 48K to be safe.

LIBSA = libsa/*.c
SRCDIR	= /usr/src/cmd
INCLUDES=
BOOT = bootstrap/*.[sc]

ADB = ${SRCDIR}/adb/*.c

CFLAGS = -O -DSTANDALONE ${INCLUDES}

ALL=	restor cat icheck mkfs mtboot tsboot maketape boot 

all:	$(ALL) size

libsa.a:${LIBSA}
	(cd libsa; make INCLUDES="${INCLUDES}" ../libsa.a)

restor:	libsa.a srt0.o ${SRCDIR}/restor.c c.o
	cc -c ${CFLAGS} ${SRCDIR}/restor.c
	ld  -o restor srt0.o restor.o c.o libsa.a -lnonfpc
	rm restor.o

cat:	libsa.a srt0.o c.o
	cc -c ${CFLAGS} util/cat.c
	ld -s -o cat srt0.o cat.o c.o libsa.a -lnonfpc
	rm cat.o

mkfs:	libsa.a srt0.o ${SRCDIR}/mkfs.c c.o
	cc -c ${CFLAGS} ${SRCDIR}/mkfs.c 
	ld  -o mkfs srt0.o mkfs.o c.o libsa.a -lnonfpc
	rm mkfs.o

icheck:	libsa.a srt0.o ${SRCDIR}/icheck.c c.o
	cc -c ${CFLAGS} ${SRCDIR}/icheck.c 
	ld  -o icheck srt0.o icheck.o c.o libsa.a -lnonfpc
	rm icheck.o

mtboot:	mtboot.s
	as -o mtboot mtboot.s
	strip mtboot

tsboot:	tsboot.s
	as -o tsboot tsboot.s
	strip tsboot

maketape:	util/maketape.c
	cc -o maketape util/maketape.c

boot:   ${BOOT} libsa.a c.o
	(chdir bootstrap; make INCLUDES="${INCLUDES}" ../boot)

distr:	icheck restor cat mkfs boot mtboot maketape 
	./maketape /usr/dev/nrmt0 tapes/tapedir
	dump 0f /usr/dev/nrmt0 /dev/rrm0b
	chdir /usr; tar cfb /usr/dev/nrmt0 20 \
	70 adm bin contrib dict doc games include lib local man \
	msgs preserve public spool sys tmp ucb

distr2:
	chdir /usr; tar cfb /usr/dev/nrmt0 20 net src -C /mnt/2.8 ingres

distr1600:
	./maketape /dev/nrmt1 tapes/tapedir.1600
	dump 0f /usr/dev/nrmt1 /dev/rrm0b
	chdir /usr; tar cfb /usr/dev/nrmt1 20 \
	70 adm bin dict doc games include lib local man \
	msgs preserve public spool sys tmp ucb net contrib \
	src -C /mnt/2.8 ingres

boottape:	icheck restor cat mkfs boot mtboot maketape 
	./maketape /dev/nrmt0 tapes/tapedir
	dump 0f /dev/rmt0 /dev/rrm0b

tsdistr:icheck restor cat mkfs boot tsboot maketape 
	./maketape /dev/nrmt1 tapes/tstapedir
	dump 0f /usr/dev/nrmt1 /dev/rrm0b
	chdir /usr; tar cfb /usr/dev/nrmt1 20 \
	70 adm bin dict doc games include lib local man \
	msgs preserve public spool sys tmp ucb net contrib \
	src -C /mnt/2.8 ingres

print:
	pr saio.h tsboot.s mtboot.s srt0.s bootstrap/M.s bootstrap/boot.c \
	bootstrap/ubmapset.c libsa/*.c ${SRCDIR}/mkfs.c ${SRCDIR}/restor.c  \
	util/cat.c c.c|lpr

size:
	size ${ALL}
	
clean:
	rm -f $(ALL)
	rm -f *.o
