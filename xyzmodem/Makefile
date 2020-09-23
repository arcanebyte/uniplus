# Makefile for Unix/Xenix rz and sz programs
# the makefile is not too well tested yet

nothing:
	@echo
	@echo "Please study the #ifdef's in rbsb.c, rz.c and sz.c,"
	@echo "then type 'make system' where system is one of:"
	@echo "	sysvr3	SYSTEM 5.3 Unix with mkdir(2)"
	@echo "	sysv	SYSTEM 3/5 Unix"
	@echo "	xenix	Xenix"
	@echo "	x386	386 Xenix"
	@echo "	bsd	Berkeley 4.x BSD, Ultrix, V7"
	@echo

usenet:
	shar -f /tmp/rzsz README Makefile zmodem.h zm.c sz.c rz.c rbsb.c \
	  minirb.c *.1 gz ptest.sh zupl.t

shar:
	 shar -f /tmp/rzsz -m 1000000 README Makefile zmodem.h zm.c \
	    sz.c rz.c rbsb.c minirb.c *.1 gz ptest.sh zupl.t

unixforum: shar
	compress -b12 /tmp/rzsz.sh

arc:
	rm -f /tmp/rzsz.arc
	arc a /tmp/rzsz README Makefile zmodem.h zm.c sz.c rz.c \
	    rbsb.c *.1 gz ptest.sh zupl.t minirb.c
	chmod og-w /tmp/rzsz.arc
	mv /tmp/rzsz.arc /t/yam

zoo:
	rm -f /tmp/rzsz.zoo
	zoo a /tmp/rzsz README Makefile zmodem.h zm.c sz.c rz.c \
	    rbsb.c *.1 gz ptest.sh zupl.t minirb.c
	chmod og-w /tmp/rzsz.zoo
	mv /tmp/rzsz.zoo /t/yam

.PRECIOUS:rz sz

xenix:
	cc -M0 -Ox -K -i -DNFGVMIN -DREADCHECK sz.c -lx -o sz
	size sz
	-ln sz sb
	-ln sz sx
	cc -M0 -Ox -K -i -DMD rz.c -o rz
	size rz
	-ln rz rb
	-ln rz rx

x386:
	cc -Ox -DMD rz.c -o rz
	size rz
	-ln rz rb
	-ln rz rx
	cc -Ox -DNFGVMIN -DREADCHECK sz.c -lx -o sz
	size sz
	-ln sz sb
	-ln sz sx

sysv:
	cc -O -DMD rz.c -o rz
	size rz
	-ln rz rb
	-ln rz rx
	cc -DSV -O -DNFGVMIN sz.c -o sz
	size sz
	-ln sz sb
	-ln sz sx

sysvlisa:
	cc -O -DMD rz.c -o rz
	size rz
	-ln rz rb
	-ln rz rx
	cc -O -DNFGVMIN sz.c -o sz
	size sz
	-ln sz sb
	-ln sz sx

sysvr3:
	cc -O -DMD=2 rz.c -o rz
	size rz
	-ln rz rb
	-ln rz rx
	cc -DSV -O -DNFGVMIN sz.c -o sz
	size sz
	-ln sz sb
	-ln sz sx

bsd:
	cc -DMD=2 -Dstrchr=index -DV7 -O rz.c -o rz
	size rz
	-ln rz rb
	-ln rz rx
	cc -DV7 -O -DNFGVMIN sz.c -o sz
	size sz
	-ln sz sb
	-ln sz sx

sz: nothing
sb: nothing
rz: nothing
rb: nothing