# Description file for 68000 UNIX
# Modified by Arcanebyte for compilation on Apple Lisa
# Avoid bringing in resources for other platforms or devices that
# we know we won't need.

# mch and ureg have code that must have bit 14 of the address
# set because they access the mmu registers yet are referenced via them.
# This is why the starting address is C000 and why they MUST REMAIN FIRST
# in the `KERNEL' define below.
# This value is hard-coded into the RESTART ioctl used by reboot.
# It's bit 14 because there are 14 bits worth of ROM (ie 8K words).

S       =alm
M       =SUN
V1      =v5
V2      =68
HOST    =happy

FIX     =fixbin68
SYNC    =
#MMU     =/v/sys/$M/sys
#GEN     =/v/sys/GEN/sys
GENnet  =/v/sys/GENnet/sys
#NET     =/v/sys/NET/sys
#SPECIFIC=/v/sys/$S/sys
#INCLUDE =-I/v/test/dist/$S/usr/include -I/v/test/dist/dist/usr/include -I$(SPECIFIC)/h
TTY     =0
TTY     =1

CC      =cc$(V1)
CC      =cc$(V1)net $(HOST)
AS      =as$(V2)
LD      =ld$(V2)
SHELL   =/bin/sh
RELOC   =C000
LIB     =-l$(V1)c

#Enable/disable the next five lines for compiling on target 68000/(VAX or pippin)
MMU    =/usr/src/sys
GEN    =/usr/src/sys
NET    =/usr/src/sys
SPECIFIC=/usr/src/sys
INCLUDE=

#Enable/disable the next five lines for compiling on any 68000/VAX
FIX     =size
SYNC    =sync; sync
V1      =
V2      =
CC      =cc$(V1)

SYS     =unix
NODE    =alm
REL     =5.1
VER     =rl
MACH    =m68000
DEFS    =-DNONSCATLOAD -DKERNEL -Uvax -Dmc68000 -DUCB_NET
OOPT    =BPS
V       =-v
CFLAGS  =$(DEFS) $(INCLUDE) -O$(OOPT) $(V)
LFLAGS  =-R $(RELOC) -d -x

SFILES= mch.s ivec.s

CCONFIG=conf.c config.c linesw.c machdep.c name.c
CDEV=   bm.c co.c cv.c kb.c l1.c l2.c lp.c ms.c rtc.c \
	priam.c pro.c rom_mon.c scc.c sk.c  sony.c tecmar.c vt100.c
CMMU=   $(MMU)/bio.c ./cxureg.c $(MMU)/main.c \
	$(MMU)/malloc.c mem.c ./shm.c $(MMU)/sig.c \
	$(MMU)/slp.c $(MMU)/sys1.c $(MMU)/sys4.c $(MMU)/text.c
CGEN=   $(GEN)/acct.c $(GEN)/alloc.c $(GEN)/clist.c \
	$(GEN)/clock.c $(GEN)/dsort.c $(GEN)/err.c $(GEN)/errlog.c \
	$(GEN)/fio.c $(GEN)/iget.c $(GEN)/ipc.c \
	$(GEN)/lock.c $(GEN)/locking.c $(GEN)/msg.c \
	$(GEN)/nami.c $(GEN)/partab.c $(GEN)/pipe.c \
	$(GEN)/prf.c $(GEN)/rdwri.c $(GEN)/sem.c \
	$(GEN)/subr.c $(GEN)/sys.c $(GEN)/sys2.c \
	$(GEN)/sys3.c $(GEN)/sysent.c $(GEN)/trap.c \
	$(GEN)/tt$(TTY).c $(GEN)/tty.c $(GEN)/utssys.c
CGENnet=$(GENnet)/syslocal.c $(GENnet)/pty.c \
	$(GENnet)/mbuf.c $(GENnet)/netipc.c \
	$(GENnet)/proto.c $(GENnet)/socket.c $(GENnet)/socketsubr.c
CNONET= $(NET)/fnet.c fnetlocal.c
CNETFILES= $(NET)/af.c $(NET)/if.c ./if_eb.c $(NET)/if_loop.c \
	$(NET)/if_ether.c $(NET)/in.c \
	$(NET)/in_cksum.c $(NET)/in_pcb.c $(NET)/ip_icmp.c $(NET)/ip_input.c \
	$(NET)/ip_output.c $(NET)/raw_cb.c \
	$(NET)/raw_ip.c $(NET)/raw_usrreq.c \
	$(NET)/route.c $(NET)/tcp_debug.c $(NET)/tcp_input.c \
	$(NET)/tcp_output.c $(NET)/tcp_subr.c $(NET)/tcp_timer.c \
	$(NET)/tcp_usrreq.c $(NET)/udp_usrreq.c
CFILES= $(CCONFIG) $(CDEV) $(CMMU) $(CGEN) $(CGENnet) $(CNETFILES)
CNONETFILES=    $(CCONFIG) $(CDEV) $(CMMU) $(CGEN) $(CNONET)

NETSYS= syslocal.o pty.o mbuf.o
IPC=    netipc.o socket.o socketsubr.o proto.o
TCP=    tcp_output.o tcp_input.o tcp_timer.o
IP=     af.o ip_icmp.o ip_input.o ip_output.o in.o in_pcb.o in_cksum.o
IF=     if.o if_loop.o if_eb.o if_ether.o
USR=    udp_usrreq.o raw_cb.o raw_ip.o raw_usrreq.o tcp_usrreq.o tcp_subr.o \
	tcp_debug.o route.o
NETOBJS=$(NETSYS) $(IPC) $(TCP) $(IP) $(IF) $(USR)
FNETOBJS=fnet.o fnetlocal.o

KERNEL= mch.o cxureg.o \
	acct.o alloc.o bio.o clist.o clock.o dsort.o \
	err.o errlog.o fio.o iget.o ipc.o lock.o locking.o \
	machdep.o main.o malloc.o mem.o msg.o nami.o partab.o \
	pipe.o prf.o rdwri.o sem.o shm.o sig.o slp.o subr.o \
	sys.o sys1.o sys2.o sys3.o sys4.o sysent.o \
	text.o trap.o tt$(TTY).o tty.o utssys.o

SKERNEL=mch.o cxureg.o \
	acct.o alloc.o bio.o clist.o clock.o dsort.o \
	err.o errlog.o fio.o iget.o ipc.o lock.o locking.o \
	machdep.o main.o malloc.o mem.o msg.o nami.o partab.o \
	pipe.o prf.o rdwri.o sem.o shm.o sig.o s_slp.o subr.o \
	sys.o sys1.o sys2.o sys3.o sys4.o sysent.o \
	text.o trap.o tt$(TTY).o tty.o utssys.o

# Normal configuration files.
CONFIG= conf.o config.o linesw.o ivec.o \
	bm.o co.o cv.o kb.o l1.o l2.o lp.o ms.o rtc.o \
	priam.o pro.o rom_mon.o scc.o sk.o sony.o tecmar.o vt100.o name.o

# Configuration files for sunix (startup unix which uses
# Sony root device and asks for the swap device).
CONFIGS=reinit.o s_conf.o s_config.o linesw.o ivec.o \
	bm.o s_co.o cv.o s_kb.o l1.o l2.o lp.o ms.o rtc.o \
	priam.o pro.o rom_mon.o scc.o sk.o sony.o tecmar.o vt100.o name.o

# Configuration files for unix.su (single-user unix).
CONFIGSU= conf.o config.o linesw.o ivec.o \
	bm.o co.o cv.o kb.o l1.o l2.o lp.o ms.o rtc.o \
	priam.o pro.o rom_mon.o scc.su.o sk.o sony.o tecmar.su.o vt100.o name.o

VAX=    vax:/v/sys/alm

.c.o:
	$(CC) $(CFLAGS) -c $<
	@$(LD) -r -x $@ ; mv a.out $@

all:    unix.net

# Remove name.o before doing a make distrib
# Be sure to distribute reboot with sunix
distrib:reboot unix.net unix.nonet unix.su

upload:
	rcp unix.net $(VAX)/unix.net
	rcp unix.nonet $(VAX)/unix.nonet
	rcp unix.su $(VAX)/unix.su
	rcp sunix $(VAX)/sunix
	rcp reboot $(VAX)/reboot

tags:   $(CFILES)
	ctags $(CFILES)

xtags:
	ctags -x $(CFILES) > xref

unix.net:       unix.o net.o $(CONFIG)
	$(LD) -o unix.net $(LFLAGS) unix.o net.o $(CONFIG) $(LIB)
	$(FIX) unix.net
	$(SYNC)
	echo "^G^G^G^G"

xunix:  $(KERNEL) $(NETOBJS) $(CONFIG)
	$(LD) -o xunix $(LFLAGS) $(KERNEL) $(NETOBJS) $(CONFIG) $(LIB)
	$(FIX) xunix
	$(SYNC)
	echo "^G^G^G^G"

unix.nonet:     unix.o $(FNETOBJS) $(CONFIG)
	$(LD) -o unix.nonet $(LFLAGS) unix.o $(FNETOBJS) $(CONFIG) $(LIB)
	$(FIX) unix.nonet
	$(SYNC)
	echo "^G^G^G^G"

# Startup unix.
sunix:  sunix.o $(FNETOBJS) $(CONFIGS)
	$(LD) -o sunix $(LFLAGS) sunix.o $(FNETOBJS) $(CONFIGS) $(LIB)
	$(SYNC)
	echo "^G^G^G^G"

nmaddrs.h:      sunix
	nm -n sunix | egrep 'rootdev|pipedev|dumpdev|swapdev|nswap' | \
		sed -e '/rootdev/s/^/#define ROOTDEV 0x/' \
			-e '/rootdev/s/ D _rootdev//' \
			-e '/pipedev/s/^/#define PIPEDEV 0x/' \
			-e '/pipedev/s/ D _pipedev//' \
			-e '/dumpdev/s/^/#define DUMPDEV 0x/' \
			-e '/dumpdev/s/ D _dumpdev//' \
			-e '/swapdev/s/^/#define SWAPDEV 0x/' \
			-e '/swapdev/s/ D _swapdev//' \
			-e '/nswap/s/^/#define NSWAP 0x/' \
			-e '/nswap/s/ D _nswap//' > nmaddrs.h

reboot: nmaddrs.h reboot.c
	$(CC) $(INCLUDE) -O -v reboot.c -o reboot

mkfspm: mkfspm.c
	$(CC) $(INCLUDE) -O -v mkfspm.c -o mkfspm

# Single-user unix.
unix.su:        unix.o $(FNETOBJS) $(CONFIGSU)
	$(LD) -o unix.su $(LFLAGS) unix.o $(FNETOBJS) $(CONFIGSU) $(LIB)
	$(FIX) unix.su
	$(SYNC)
	echo "^G^G^G^G"

sunix.o:        $(SKERNEL)
	$(LD) -o sunix.o -r $(SKERNEL)

unix.o: $(KERNEL)
	$(LD) -o unix.o -r $(KERNEL)

net.o:  $(NETOBJS)
	$(LD) -o net.o -r $(NETOBJS)

mch.o:  mch.s
	$(AS) -o mch.o mch.s
	@$(LD) -r -x $@ ; mv a.out $@

ivec.o: ivec.s
	$(AS) -o ivec.o ivec.s
	@$(LD) -r -x $@ ; mv a.out $@

#name.o:
name.o: Makefile /tmp
	$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		-DTIMESTAMP="\"`date`\"" \
		name.c
install:
	mv /unix /ounix
	cp unix.net /unix
	chown check /unix
	chgrp check /unix

lint:
	@echo Linting...
	@lint $(DEFS) $(INCLUDE) Locore.c $(CFILES)

lintnonet:
	@echo Linting...
	@lint $(DEFS) $(INCLUDE) Locore.c $(CNONETFILES)

s_co.o:         s_co.c
	$(CC) $(CFLAGS) -DSUNIX -c s_co.c
	@$(LD) -r -x $@ ; mv a.out $@
s_conf.o:       s_conf.c
	$(CC) $(CFLAGS) -DSUNIX -c s_conf.c
	@$(LD) -r -x $@ ; mv a.out $@
s_config.o:     s_config.c
	$(CC) $(CFLAGS) -DSUNIX -c s_config.c
	@$(LD) -r -x $@ ; mv a.out $@
s_kb.o:         s_kb.c
	$(CC) $(CFLAGS) -DSUNIX -c s_kb.c
	@$(LD) -r -x $@ ; mv a.out $@
scc.su.o:       scc.su.c
	$(CC) $(CFLAGS) -DSINGLEUSER -c scc.su.c
	@$(LD) -r -x $@ ; mv a.out $@
tecmar.su.o:    tecmar.su.c
	$(CC) $(CFLAGS) -DSINGLEUSER -c tecmar.su.c
	@$(LD) -r -x $@ ; mv a.out $@
bio.o:          $(MMU)/bio.c
	$(CC) $(CFLAGS) -c $(MMU)/bio.c
	@$(LD) -r -x $@ ; mv a.out $@
cxureg.o:       ./cxureg.c
	$(CC) $(CFLAGS) -c ./cxureg.c
	@$(LD) -r -x $@ ; mv a.out $@
main.o:         $(MMU)/main.c
	$(CC) $(CFLAGS) -c $(MMU)/main.c
	@$(LD) -r -x $@ ; mv a.out $@
malloc.o:       $(MMU)/malloc.c
	$(CC) $(CFLAGS) -c $(MMU)/malloc.c
	@$(LD) -r -x $@ ; mv a.out $@
mem.o:          mem.c
	$(CC) $(CFLAGS) -c mem.c
	@$(LD) -r -x $@ ; mv a.out $@
shm.o:          ./shm.c
	$(CC) $(CFLAGS) -c ./shm.c
	@$(LD) -r -x $@ ; mv a.out $@
sig.o:          $(MMU)/sig.c
	$(CC) $(CFLAGS) -c $(MMU)/sig.c
	@$(LD) -r -x $@ ; mv a.out $@
s_slp.o:        s_slp.c
	$(CC) $(CFLAGS) -c s_slp.c
	@$(LD) -r -x $@ ; mv a.out $@
slp.o:          $(MMU)/slp.c
	$(CC) $(CFLAGS) -c $(MMU)/slp.c
	@$(LD) -r -x $@ ; mv a.out $@
sys1.o:         $(MMU)/sys1.c
	$(CC) $(CFLAGS) -c $(MMU)/sys1.c
	@$(LD) -r -x $@ ; mv a.out $@
sys4.o:         $(MMU)/sys4.c
	$(CC) $(CFLAGS) -c $(MMU)/sys4.c
	@$(LD) -r -x $@ ; mv a.out $@
text.o:         $(MMU)/text.c
	$(CC) $(CFLAGS) -c $(MMU)/text.c
	@$(LD) -r -x $@ ; mv a.out $@
acct.o:         $(GEN)/acct.c
	$(CC) $(CFLAGS) -c $(GEN)/acct.c
	@$(LD) -r -x $@ ; mv a.out $@
alloc.o:        $(GEN)/alloc.c
	$(CC) $(CFLAGS) -c $(GEN)/alloc.c
	@$(LD) -r -x $@ ; mv a.out $@
clist.o:        $(GEN)/clist.c
	$(CC) $(CFLAGS) -c $(GEN)/clist.c
	@$(LD) -r -x $@ ; mv a.out $@
clock.o:        $(GEN)/clock.c
	$(CC) $(CFLAGS) -c $(GEN)/clock.c
	@$(LD) -r -x $@ ; mv a.out $@
dsort.o:        $(GEN)/dsort.c
	$(CC) $(CFLAGS) -c $(GEN)/dsort.c
	@$(LD) -r -x $@ ; mv a.out $@
err.o:          $(GEN)/err.c
	$(CC) $(CFLAGS) -c $(GEN)/err.c
	@$(LD) -r -x $@ ; mv a.out $@
errlog.o:       $(GEN)/errlog.c
	$(CC) $(CFLAGS) -c $(GEN)/errlog.c
	@$(LD) -r -x $@ ; mv a.out $@
fio.o:          $(GEN)/fio.c
	$(CC) $(CFLAGS) -c $(GEN)/fio.c
	@$(LD) -r -x $@ ; mv a.out $@
iget.o:         $(GEN)/iget.c
	$(CC) $(CFLAGS) -c $(GEN)/iget.c
	@$(LD) -r -x $@ ; mv a.out $@
ipc.o:          $(GEN)/ipc.c
	$(CC) $(CFLAGS) -c $(GEN)/ipc.c
	@$(LD) -r -x $@ ; mv a.out $@
lock.o:         $(GEN)/lock.c
	$(CC) $(CFLAGS) -c $(GEN)/lock.c
	@$(LD) -r -x $@ ; mv a.out $@
locking.o:      $(GEN)/locking.c
	$(CC) $(CFLAGS) -c $(GEN)/locking.c
	@$(LD) -r -x $@ ; mv a.out $@
msg.o:          $(GEN)/msg.c
	$(CC) $(CFLAGS) -c $(GEN)/msg.c
	@$(LD) -r -x $@ ; mv a.out $@
nami.o:         $(GEN)/nami.c
	$(CC) $(CFLAGS) -c $(GEN)/nami.c
	@$(LD) -r -x $@ ; mv a.out $@
partab.o:       $(GEN)/partab.c
	$(CC) $(CFLAGS) -c $(GEN)/partab.c
	@$(LD) -r -x $@ ; mv a.out $@
pipe.o:         $(GEN)/pipe.c
	$(CC) $(CFLAGS) -c $(GEN)/pipe.c
	@$(LD) -r -x $@ ; mv a.out $@
prf.o:          $(GEN)/prf.c
#       $(CC) $(CFLAGS) -c -DPRINTFSTALL=30000 $(GEN)/prf.c
	$(CC) $(CFLAGS) -c $(GEN)/prf.c
	@$(LD) -r -x $@ ; mv a.out $@
rdwri.o:        $(GEN)/rdwri.c
	$(CC) $(CFLAGS) -c $(GEN)/rdwri.c
	@$(LD) -r -x $@ ; mv a.out $@
sem.o:          $(GEN)/sem.c
	$(CC) $(CFLAGS) -c $(GEN)/sem.c
	@$(LD) -r -x $@ ; mv a.out $@
subr.o:         $(GEN)/subr.c
	$(CC) $(CFLAGS) -c $(GEN)/subr.c
	@$(LD) -r -x $@ ; mv a.out $@
sys.o:          $(GEN)/sys.c
	$(CC) $(CFLAGS) -c $(GEN)/sys.c
	@$(LD) -r -x $@ ; mv a.out $@
sys2.o:         $(GEN)/sys2.c
	$(CC) $(CFLAGS) -c $(GEN)/sys2.c
	@$(LD) -r -x $@ ; mv a.out $@
sys3.o:         $(GEN)/sys3.c
	$(CC) $(CFLAGS) -c $(GEN)/sys3.c
	@$(LD) -r -x $@ ; mv a.out $@
sysent.o:       $(GEN)/sysent.c
	$(CC) $(CFLAGS) -c $(GEN)/sysent.c
	@$(LD) -r -x $@ ; mv a.out $@
trap.o:         $(GEN)/trap.c
	$(CC) $(CFLAGS) -c $(GEN)/trap.c
	@$(LD) -r -x $@ ; mv a.out $@
tt$(TTY).o:     $(GEN)/tt$(TTY).c
	$(CC) $(CFLAGS) -c $(GEN)/tt$(TTY).c
	@$(LD) -r -x $@ ; mv a.out $@
tty.o:          $(GEN)/tty.c
	$(CC) $(CFLAGS) -c $(GEN)/tty.c
	@$(LD) -r -x $@ ; mv a.out $@
utssys.o:       $(GEN)/utssys.c
	$(CC) $(CFLAGS) -c $(GEN)/utssys.c
	@$(LD) -r -x $@ ; mv a.out $@
mbuf.o:         $(GENnet)/mbuf.c
	$(CC) $(CFLAGS) -c $(GENnet)/mbuf.c
	@$(LD) -r -x $@ ; mv a.out $@
netipc.o:       $(GENnet)/netipc.c
	$(CC) $(CFLAGS) -c $(GENnet)/netipc.c
	@$(LD) -r -x $@ ; mv a.out $@
proto.o:        $(GENnet)/proto.c
	$(CC) $(CFLAGS) -c $(GENnet)/proto.c
	@$(LD) -r -x $@ ; mv a.out $@
pty.o:          $(GENnet)/pty.c
	$(CC) $(CFLAGS) -c $(GENnet)/pty.c
	@$(LD) -r -x $@ ; mv a.out $@
socket.o:       $(GENnet)/socket.c
	$(CC) $(CFLAGS) -c $(GENnet)/socket.c
	@$(LD) -r -x $@ ; mv a.out $@
socketsubr.o:   $(GENnet)/socketsubr.c
	$(CC) $(CFLAGS) -c $(GENnet)/socketsubr.c
	@$(LD) -r -x $@ ; mv a.out $@
syslocal.o:     $(GENnet)/syslocal.c
	$(CC) $(CFLAGS) -c $(GENnet)/syslocal.c
	@$(LD) -r -x $@ ; mv a.out $@
af.o:           $(NET)/af.c
	$(CC) $(CFLAGS) -c $(NET)/af.c
	@$(LD) -r -x $@ ; mv a.out $@
fnet.o:         $(NET)/fnet.c
	$(CC) $(CFLAGS) -c $(NET)/fnet.c
	@$(LD) -r -x $@ ; mv a.out $@
fnetlocal.o:    fnetlocal.c
	$(CC) $(CFLAGS) -c fnetlocal.c
	@$(LD) -r -x $@ ; mv a.out $@
if_loop.o:      $(NET)/if_loop.c
	$(CC) $(CFLAGS) -c $(NET)/if_loop.c
	@$(LD) -r -x $@ ; mv a.out $@
if_eb.o:        ./if_eb.c
	$(CC) $(CFLAGS) -c ./if_eb.c
	@$(LD) -r -x $@ ; mv a.out $@
if.o:           $(NET)/if.c
	$(CC) $(CFLAGS) -c $(NET)/if.c
	@$(LD) -r -x $@ ; mv a.out $@
if_ether.o:     $(NET)/if_ether.c
	$(CC) $(CFLAGS) -c $(NET)/if_ether.c
	@$(LD) -r -x $@ ; mv a.out $@
in.o:           $(NET)/in.c
	$(CC) $(CFLAGS) -c $(NET)/in.c
	@$(LD) -r -x $@ ; mv a.out $@
in_cksum.o:     $(NET)/in_cksum.c
	$(CC) $(CFLAGS) -c $(NET)/in_cksum.c
	@$(LD) -r -x $@ ; mv a.out $@
in_pcb.o:       $(NET)/in_pcb.c
	$(CC) $(CFLAGS) -c $(NET)/in_pcb.c
	@$(LD) -r -x $@ ; mv a.out $@
ip_icmp.o:      $(NET)/ip_icmp.c
	$(CC) $(CFLAGS) -c $(NET)/ip_icmp.c
	@$(LD) -r -x $@ ; mv a.out $@
ip_input.o:     $(NET)/ip_input.c
	$(CC) $(CFLAGS) -c $(NET)/ip_input.c
	@$(LD) -r -x $@ ; mv a.out $@
ip_output.o:    $(NET)/ip_output.c
	$(CC) $(CFLAGS) -c $(NET)/ip_output.c
	@$(LD) -r -x $@ ; mv a.out $@
raw_cb.o:       $(NET)/raw_cb.c
	$(CC) $(CFLAGS) -c $(NET)/raw_cb.c
	@$(LD) -r -x $@ ; mv a.out $@
raw_ip.o:       $(NET)/raw_ip.c
	$(CC) $(CFLAGS) -c $(NET)/raw_ip.c
	@$(LD) -r -x $@ ; mv a.out $@
raw_usrreq.o:   $(NET)/raw_usrreq.c
	$(CC) $(CFLAGS) -c $(NET)/raw_usrreq.c
	@$(LD) -r -x $@ ; mv a.out $@
route.o:        $(NET)/route.c
	$(CC) $(CFLAGS) -c $(NET)/route.c
	@$(LD) -r -x $@ ; mv a.out $@
tcp_debug.o:    $(NET)/tcp_debug.c
	$(CC) $(CFLAGS) -c $(NET)/tcp_debug.c
	@$(LD) -r -x $@ ; mv a.out $@
tcp_input.o:    $(NET)/tcp_input.c
	$(CC) $(CFLAGS) -c $(NET)/tcp_input.c
	@$(LD) -r -x $@ ; mv a.out $@
tcp_output.o:   $(NET)/tcp_output.c
	$(CC) $(CFLAGS) -c $(NET)/tcp_output.c
	@$(LD) -r -x $@ ; mv a.out $@
tcp_subr.o:     $(NET)/tcp_subr.c
	$(CC) $(CFLAGS) -c $(NET)/tcp_subr.c
	@$(LD) -r -x $@ ; mv a.out $@
tcp_timer.o:    $(NET)/tcp_timer.c
	$(CC) $(CFLAGS) -c $(NET)/tcp_timer.c
	@$(LD) -r -x $@ ; mv a.out $@
tcp_usrreq.o:   $(NET)/tcp_usrreq.c
	$(CC) $(CFLAGS) -c $(NET)/tcp_usrreq.c
	@$(LD) -r -x $@ ; mv a.out $@
udp_usrreq.o:   $(NET)/udp_usrreq.c
	$(CC) $(CFLAGS) -c $(NET)/udp_usrreq.c
	@$(LD) -r -x $@ ; mv a.out $@
