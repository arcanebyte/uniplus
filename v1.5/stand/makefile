# Description file for 68000 Standalone Programs

S	=alm
V1	=v5
V2	=68

SYNC	=
GEN	=/v/sys/GEN/stand
SRC	=/v/src/cmd
INCLUDE	=-I/v/test/dist/$S/usr/include -I/v/sys/alm/sys/h -I/v/test/dist/dist/usr/include -I/v/sys/alm/stand
DISK1	=sony
DISK2	=pro
DISK3	=cv
DISK4	=priam
SERIAL	=console

#Enable/disable the next seven lines for compiling on 68000/VAX
#V1	=
#V2	=
#SYNC	=sync; sync
#GEN	=/usr/src/stand
#SRC	=/usr/src/stand
#INCLUDE=

CC	=cc$(V1)
CC	=cc$(V1)net any
AS	=as$(V2)
LD	=ld$(V2)
SHELL	=/bin/sh
RELOC	=30000
BRELOC	=60000
LIB	=-l$(V1)c
LIBSA	=libsa.a

DEFS	=-DSTANDALONE -Dmc68000
OOPT	=BPS
CFLAGS	=$(INCLUDE) $(DEFS) -O$(OOPT) -v
LFLAGS	=-X -R $(RELOC)
BLFLAGS	=-X -R $(BRELOC)

RESTRICTED=dd.c fsck.c fsck1b.c mkfs.c mkfs1b.c prf.c sys.c 

# NOTE	console.c	keyboard and builtin screen (SERIAL)
#	sony.c		sony driver (DISK1)
#	pro.c		profile/widget driver (DISK2)
#	cv.c		Corvus driver (DISK3)
#	priam.c		Priam driver (DISK4)
#	sunol.c		Sunol streaming tape driver
#	tpriam.c	Priam streaming tape driver
ODEV	=$(DISK1).o $(DISK2).o $(DISK3).o $(DISK4).o $(SERIAL).o
OFILES	=sys.o prf.o gethex.o putchar.o getchar.o wserial.o c.o \
	$(ODEV) rom_mon.o 
CDEV	=$(DISK1).c $(DISK2).c $(DISK3).c $(DISK4).c $(SERIAL).c
CGEN	=$(GEN)/sys.c $(GEN)/prf.c $(GEN)/gethex.c \
		putchar.c $(GEN)/getchar.c wserial.c
CFILES	=$(CGEN) c.c rom_mon.c $(CDEV) sunol.c tpriam.c

ALL =	lisa boot streamer fsck fsck1b mkfs mkfs1b cat ls od

all:	$(ALL)

$(ALL): $(LIBSA)

tags:
	-rm -f tags
	ctags $(CFILES)

clean:
	-rm -f $(ALL)

lint:
	@echo Linting...
	@lint $(DEFS) $(INCLUDE) lisa.c mkser.c mkcode.c cvt.c setup.c $(CFILES)

#restricted:
#	-rm -f $(RESTRICTED)

$(LIBSA):$(OFILES)
	ar$(V2) r $(LIBSA) $?

sys.o:		$(GEN)/sys.c
	$(CC) $(CFLAGS) -c $(GEN)/sys.c
	@$(LD) -r -X $@ ; mv a.out $@
prf.o:		$(GEN)/prf.c
	$(CC) $(CFLAGS) -c $(GEN)/prf.c
	@$(LD) -r -X $@ ; mv a.out $@
gethex.o:	$(GEN)/gethex.c
	$(CC) $(CFLAGS) -c $(GEN)/gethex.c
	@$(LD) -r -X $@ ; mv a.out $@
putchar.o:	putchar.c
	$(CC) $(CFLAGS) -c putchar.c
	@$(LD) -r -X $@ ; mv a.out $@
getchar.o:	$(GEN)/getchar.c
	$(CC) $(CFLAGS) -c $(GEN)/getchar.c
	@$(LD) -r -X $@ ; mv a.out $@
wserial.o:	wserial.c
	$(CC) $(CFLAGS) -c wserial.c
	@$(LD) -r -X $@ ; mv a.out $@
c.o:		c.c
	$(CC) $(CFLAGS) -c c.c
	@$(LD) -r -X $@ ; mv a.out $@
rom_mon.o:	rom_mon.c
	$(CC) $(CFLAGS) -c rom_mon.c
	@$(LD) -r -X $@ ; mv a.out $@
$(DISK1).o:	$(DISK1).c
	$(CC) $(CFLAGS) -c $(DISK1).c
	@$(LD) -r -X $@ ; mv a.out $@
$(DISK2).o:	$(DISK2).c
	$(CC) $(CFLAGS) -c $(DISK2).c
	@$(LD) -r -X $@ ; mv a.out $@
$(DISK3).o:	$(DISK3).c
	$(CC) $(CFLAGS) -c $(DISK3).c
	@$(LD) -r -X $@ ; mv a.out $@
$(DISK4).o:	$(DISK4).c
	$(CC) $(CFLAGS) -c $(DISK4).c
	@$(LD) -r -X $@ ; mv a.out $@
sunol.o:	sunol.c
	$(CC) $(CFLAGS) -c sunol.c
	@$(LD) -r -X $@ ; mv a.out $@
$(SERIAL).o:	$(SERIAL).c
	$(CC) $(CFLAGS) -c $(SERIAL).c
	@$(LD) -r -X $@ ; mv a.out $@

copysboot.o:	copysboot.c
	$(CC) $(CFLAGS) -c copysboot.c
	@$(LD) -r -X $@ ; mv a.out $@
sercheck.o:	sercheck.c
	$(CC) $(CFLAGS) -c sercheck.c
	@$(LD) -r -X $@ ; mv a.out $@
nosercheck.o:	nosercheck.c
	$(CC) $(CFLAGS) -DNOSERCHECK -c nosercheck.c
	@$(LD) -r -X $@ ; mv a.out $@

M.o:	M.s
	$(AS) -o M.o M.s
MB.o:	$(GEN)/MB.s
	$(AS) -o MB.o $(GEN)/MB.s
sbblkM.o:	sbblkM.s
	$(AS) -o sbblkM.o sbblkM.s
wbblkM.o:	wbblkM.s
	$(AS) -o wbblkM.o wbblkM.s

# The start addr (20000 below) is built into the boot rom
BBLK	=sbblk.o wbblk.o pbblk.o

#	Sony boot block
sbblk:	sbblkM.o sbblk.o
	$(LD) -s -R 20000 sbblkM.o sbblk.o -o sbblk.tmp
	fixbin68 sbblk.tmp
	dd if=sbblk.tmp bs=32 skip=1 of=sbblk
	rm sbblk.tmp

#	Widget/profile boot block
wbblk:	wbblkM.o wbblk.o
	$(LD) -s -R 20000 wbblkM.o wbblk.o -o wbblk.tmp
	fixbin68 wbblk.tmp
	dd if=wbblk.tmp bs=32 skip=1 of=wbblk
	rm wbblk.tmp

#	snaddr is an include file for lisa.c.  It shows where the serial
#	number is within the boot program so it can be "serialized".
snaddr:	xboot
	nm68 -n xboot | grep snum | \
		sed -e 's/[ 	].*//' -e 's/^/char *bsadr = (char *)(0x/' -e 's/$$/);/' > snaddr

#	xboot is only used for making snaddr.  It differs from boot only
#	in that the symbol table is not stripped.
xboot:	M.o boot.o sercheck.o cvt.o mkser.o sn.o copysboot.o $(BBLK) $(LIBSA)
	$(LD) $(BLFLAGS) M.o boot.o sercheck.o cvt.o mkser.o sn.o \
		copysboot.o $(BBLK) $(LIBSA) $(LIB) -o xboot

#	Serialization program
lisa:	snaddr lisa.o mkser.o mkcode.o cvt.o setup.o sn.o $(BBLK) $(LIBSA)
	$(LD) -s $(BLFLAGS) M.o lisa.o cvt.o mkser.o mkcode.o setup.o sn.o \
		$(BBLK) $(LIBSA) $(LIB) -o lisa
	fixbin68 lisa

#	Standalone bootstrap
boot:	M.o boot.o sercheck.o cvt.o mkser.o sn.o copysboot.o $(BBLK) $(LIBSA)
	$(LD) -s $(BLFLAGS) M.o boot.o sercheck.o cvt.o mkser.o sn.o \
		copysboot.o $(BBLK) $(LIBSA) $(LIB) -o boot
	fixbin68 boot
#nsboot: M.o boot.o nosercheck.o cvt.o mkser.o sn.o copysboot.o $(BBLK) $(LIBSA)
#	$(LD) -s $(BLFLAGS) M.o boot.o nosercheck.o cvt.o mkser.o sn.o \
#		copysboot.o $(BBLK) $(LIBSA) $(LIB) -o nsboot
#	fixbin68 nsboot
#bootx:	MB.o boot.o sercheck.o cvt.o mkser.o sn.o copysboot.o $(BBLK) $(LIBSA)
#	$(LD) -s $(BLFLAGS) MB.o boot.o sercheck.o cvt.o mkser.o sn.o \
#		copysboot.o $(BBLK) $(LIBSA) $(LIB) -o bootx
#	fixbin68 bootx

#	Standalone streaming tape utility (backup/restore for sunol or priam)
streamer:	M.o streamer.o sunol.o tpriam.o $(BBLK) $(LIBSA)
	$(LD) -s $(BLFLAGS) M.o streamer.o sunol.o tpriam.o $(BBLK) $(LIBSA) $(LIB) \
		-o streamer
	fixbin68 streamer

#	Programs to monitor remote line
#dlmc:	M.o dlmc.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o dlmc.o $(LIBSA) $(LIB) -o dlmc
#mon:	M.o mon.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o mon.o $(LIBSA) $(LIB) -o mon 

#	Standalone versions of utility programs
#dd:	M.o dd.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o dd.o $(LIBSA) $(LIB) -o dd
fsck:	M.o fsck.o $(LIBSA)
	$(LD) $(LFLAGS) M.o fsck.o $(LIBSA) $(LIB) -o fsck
fsck1b:	M.o fsck1b.o $(LIBSA)
	$(LD) $(LFLAGS) M.o fsck1b.o $(LIBSA) $(LIB) -o fsck1b
mkfs:	M.o mkfs.o $(LIBSA)
	$(LD) $(LFLAGS) M.o mkfs.o $(LIBSA) $(LIB) -o mkfs
mkfs1b:	M.o mkfs1b.o $(LIBSA)
	$(LD) $(LFLAGS) M.o mkfs1b.o $(LIBSA) $(LIB) -o mkfs1b

#	Standalone quickies to read the disk
cat:	M.o cat.o $(LIBSA)
	$(LD) $(LFLAGS) M.o cat.o $(LIBSA) $(LIB) -o cat
ls:	M.o ls.o $(LIBSA)
	$(LD) $(LFLAGS) M.o ls.o $(LIBSA) $(LIB) -o ls
od:	M.o od.o $(LIBSA)
	$(LD) $(LFLAGS) M.o od.o $(LIBSA) $(LIB) -o od

#	Disk test programs
#diskcmp: M.o diskcmp.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o diskcmp.o $(LIBSA) $(LIB) -o diskcmp
#diskcopy: M.o diskcopy.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o diskcopy.o $(LIBSA) $(LIB) -o diskcopy
#diskfix: M.o diskfix.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o diskfix.o $(LIBSA) $(LIB) -o diskfix
#diskinq: M.o diskinq.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o diskinq.o $(LIBSA) $(LIB) -o diskinq
#diskrand: M.o diskrand.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o diskrand.o $(LIBSA) $(LIB) -o diskrand
#diskread: M.o diskread.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o diskread.o $(LIBSA) $(LIB) -o diskread
disktest: M.o disktest.o $(BBLK) $(LIBSA)
	$(LD) $(LFLAGS) M.o disktest.o $(BBLK) $(LIBSA) $(LIB) -o disktest
	fixbin68 disktest
#dt:	M.o dt.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o dt.o $(LIBSA) $(LIB) -o dt

#	Memory test programs
#memcmp:	M.o memcmp.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o memcmp.o $(LIBSA) $(LIB) -o memcmp
#memtest: M.o memtest.o $(LIBSA)
#	$(LD) $(LFLAGS) M.o memtest.o $(LIBSA) $(LIB) -o memtest
#staticmem: M.o staticmem.o $(LIBSA)
#	$(LD) -o staticmem -s $(LFLAGS) M.o staticmem.o $(LIBSA) $(LIB)

boot.o:		boot.c
	$(CC) $(CFLAGS) -c boot.c
	@$(LD) -r -X $@ ; mv a.out $@
cat.o:		$(GEN)/cat.c
	$(CC) $(CFLAGS) -c $(GEN)/cat.c
	@$(LD) -r -X $@ ; mv a.out $@
#dd.o:		$(SRC)/dd.c
#	$(CC) $(CFLAGS) -c $(SRC)/dd.c
#	@$(LD) -r -X $@ ; mv a.out $@
#diskcmp.o:	$(GEN)/diskcmp.c
#	$(CC) $(CFLAGS) -c $(GEN)/diskcmp.c
#	@$(LD) -r -X $@ ; mv a.out $@
#diskcopy.o:	$(GEN)/diskcopy.c
#	$(CC) $(CFLAGS) -c $(GEN)/diskcopy.c
#	@$(LD) -r -X $@ ; mv a.out $@
#diskfix.o:	$(GEN)/diskfix.c
#	$(CC) $(CFLAGS) -c $(GEN)/diskfix.c
#	@$(LD) -r -X $@ ; mv a.out $@
#diskinq.o:	$(GEN)/diskinq.c
#	$(CC) $(CFLAGS) -c $(GEN)/diskinq.c
#	@$(LD) -r -X $@ ; mv a.out $@
#diskrand.o:	$(GEN)/diskrand.c
#	$(CC) $(CFLAGS) -c $(GEN)/diskrand.c
#	@$(LD) -r -X $@ ; mv a.out $@
#diskread.o:	$(GEN)/diskread.c
#	$(CC) $(CFLAGS) -c $(GEN)/diskread.c
#	@$(LD) -r -X $@ ; mv a.out $@
disktest.o:	$(GEN)/disktest.c
	$(CC) $(CFLAGS) -c $(GEN)/disktest.c
	@$(LD) -r -X $@ ; mv a.out $@
#dlmc.o:		$(GEN)/dlmc.c
#	$(CC) $(CFLAGS) -c $(GEN)/dlmc.c
#	@$(LD) -r -X $@ ; mv a.out $@
#dt.o:		$(GEN)/dt.c
#	$(CC) $(CFLAGS) -c $(GEN)/dt.c
#	@$(LD) -r -X $@ ; mv a.out $@
fsck.o:		$(SRC)/fsck/fsck.c
	$(CC) $(CFLAGS) -c -DFsTYPE=2 $(SRC)/fsck/fsck.c
	@$(LD) -r -X $@ ; mv a.out $@
fsck1b.o:	$(SRC)/fsck/fsck1b.c
	$(CC) $(CFLAGS) -c -DFsTYPE=1 $(SRC)/fsck/fsck1b.c
	@$(LD) -r -X $@ ; mv a.out $@
lisa.o:		snaddr lisa.c
	$(CC) $(CFLAGS) -c lisa.c
	@$(LD) -r -X $@ ; mv a.out $@
ls.o:		$(GEN)/ls.c
	$(CC) $(CFLAGS) -c $(GEN)/ls.c
	@$(LD) -r -X $@ ; mv a.out $@
#memcmp.o:	$(GEN)/memcmp.c
#	$(CC) $(CFLAGS) -c $(GEN)/memcmp.c
#	@$(LD) -r -X $@ ; mv a.out $@
#memtest.o:	$(GEN)/memtest.c
#	$(CC) $(CFLAGS) -c $(GEN)/memtest.c
#	@$(LD) -r -X $@ ; mv a.out $@
mkfs.o:		$(SRC)/mkfs/mkfs.c
	$(CC) $(CFLAGS) -c $(SRC)/mkfs/mkfs.c
	@$(LD) -r -X $@ ; mv a.out $@
mkfs1b.o:	$(SRC)/mkfs/mkfs1b.c
	$(CC) $(CFLAGS) -c -DFsTYPE=1 $(SRC)/mkfs/mkfs1b.c
	@$(LD) -r -X $@ ; mv a.out $@
#mon.o:		$(GEN)/mon.c
#	$(CC) $(CFLAGS) -c $(GEN)/mon.c
#	@$(LD) -r -X $@ ; mv a.out $@
od.o:		$(GEN)/od.c
	$(CC) $(CFLAGS) -c $(GEN)/od.c
	@$(LD) -r -X $@ ; mv a.out $@
#staticmem.o:	$(GEN)/staticmem.c
#	$(CC) $(CFLAGS) -c $(GEN)/staticmem.c
#	@$(LD) -r -X $@ ; mv a.out $@
streamer.o:	streamer.c
	$(CC) $(CFLAGS) -c streamer.c
	@$(LD) -r -X $@ ; mv a.out $@
