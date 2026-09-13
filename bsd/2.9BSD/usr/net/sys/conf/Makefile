#	Makefile for Berkeley UNIX kernel, separate I/D version
#
#	Makefile Configuration:
#		First, edit the Depend file to include any optional
#		device drivers you need to include.
#		Then check the global definitions, MAXUSERS, and include file
#		search paths below.
#		INCLUDE is the directory with the normal complement of
#		include files, normally /usr/include.
#		Finally, run "make depend" and stand back.
#

# global definitions
DEFS =	-DKERNEL

# maxusers is used to scale the size of kernel data structures.
MAXUSERS=%MAXUSERS%

# SPLFIX should be one of :splfix.movb, :splfix.mtps or :splfix.spl
SPLFIX=	%SPLFIX%

# include file search path
INCLUDE= /usr/include


CPPFLAGS= ${DEFS} -I. -I${INCLUDE}
CFLAGS=	-O ${CPPFLAGS}
CC =	cc
CPP =	/lib/cpp -P
AS =	as
LD =	ld

#  shorthand to avoid running make out of space
C =	${CC} -S ${CFLAGS}
E =	ed - < ${SPLFIX}
A =	${AS} - -o
I =	${INCLUDE}

OFILE =	unix

OBJS =	acct.o alloc.o clock.o fio.o iget.o ioctl.o machdep.o main.o \
	malloc.o nami.o pipe.o prf.o prim.o rdwri.o sig.o \
	slp.o subr.o sys1.o sys2.o sys3.o sys4.o sysent.o \
	syslocal.o text.o trap.o ureg.o \
	bio.o dh.o dkbad.o dkleave.o dn.o dsort.o dz.o hk.o \
	hp.o ht.o kl.o lp.o mem.o partab.o rk.o rl.o rm.o \
	rp.o sys.o tm.o ts.o tty.o ttynew.o ttyold.o xp.o vp.o

#	boot.o must be loaded in the base well above 0 data
CONFOBJ = l.o mch.o c.o ioconf.o boot.o 


#	l.o MUST be loaded first, param.o MUST be loaded last
${OFILE}: ${OBJS} ${CONFOBJ} param.o Makefile checksys
	sh newvers.sh
	${CC} -c vers.c
	${LD} -X -i -o ${OFILE} ${CONFOBJ} ${OBJS} vers.o param.o
	size ${OFILE}
	./checksys ${OFILE}

.PRECIOUS: whoami.h param.h Makefile ../sys/sig.c

whoami.h:  localopts.h
	touch whoami.h

param.h: whoami.h ${I}/signal.h ${I}/sys/psw.h \
	${I}/sys/types.h 
	touch param.h

mch.o:	assym.s ../sys/mch.s whoami.h dz.h ht.h tm.h \
	${I}/a.out.h ${I}/sys/cpu.m ${I}/sys/trap.h \
	${I}/sys/reboot.h ${I}/sys/iopage.m \
	${I}/sys/koverlay.h
	${CPP} ${CPPFLAGS} ../sys/mch.s >mch.i
	${AS} -o mch.o assym.s mch.i
	-rm mch.i

l.o:	l.s whoami.h ${I}/a.out.h \
	${I}/sys/trap.h ${I}/sys/koverlay.h \
	dh.h dn.h dz.h hk.h hp.h hs.h ht.h kl.h lp.h \
	rk.h rl.h rm.h rp.h tm.h ts.h vp.h xp.h
	${CPP} ${CPPFLAGS} l.s >l.i
	${AS} -o l.o l.i
	-rm l.i

boot.o:	boot.s whoami.h
	${CPP} ${CPPFLAGS} boot.s >boot.i
	${AS} -o boot.o boot.i
	-rm boot.i

assym.s: genassym.c param.h dz.h \
	${I}/sys/user.h  ${I}/sys/vm.h ${I}/sys/proc.h \
	${I}/sys/dir.h ${I}/sys/inode.h ${I}/sys/seg.h
	cc ${CFLAGS} genassym.c; ./a.out >assym.s; rm -f a.out

param.o: param.c param.h Makefile ${I}/sys/proc.h \
	${I}/sys/text.h ${I}/sys/file.h ${I}/sys/inode.h \
	${I}/sys/dir.h ${I}/sys/buf.h ${I}/sys/tty.h \
	${I}/sys/conf.h ${I}/sys/user.h ${I}/sys/filsys.h \
	${I}/sys/mount.h ${I}/sys/acct.h ${I}/sys/systm.h \
	${I}/sys/map.h
	${CC} -S -DMAXUSERS=${MAXUSERS} ${CFLAGS} param.c
	ed - param.s <:comm-to-bss
	${AS} -o param.o param.s
	-rm param.s

checksys: checksys.c
	${CC} -I. -I${INCLUDE} -O -o checksys checksys.c

../sys/sig.c: ../sys/sigjcl.c ../sys/signojcl.c
	touch ../sys/sig.c

sig.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h \
	${I}/sys/user.h ${I}/sys/proc.h ${I}/sys/inode.h \
	${I}/sys/reg.h ${I}/sys/text.h ${I}/sys/seg.h \
	${I}/sys/vm.h

depend:
	@make -f Depend depend

clean:
	rm -f *.o *.i

# DO NOT DELETE THIS LINE

c.o: c.c
c.o: param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/tty.h ${I}/sys/conf.h
c.o: ${I}/sys/proc.h ${I}/sys/text.h ${I}/sys/dir.h ${I}/sys/user.h
c.o: ${I}/sys/file.h ${I}/sys/inode.h ${I}/sys/acct.h ${I}/sys/map.h
c.o: ${I}/sys/filsys.h ${I}/sys/mount.h bk.h dh.h dn.h dz.h hk.h hp.h hs.h ht.h
c.o: lp.h rk.h rl.h rm.h rp.h tm.h ts.h vp.h xp.h
	${C} c.c
	${E} c.s
	${A} c.o c.s
	-rm c.s

ioconf.o: ioconf.c
ioconf.o: param.h ${I}/sys/systm.h hk.h hp.h hs.h rk.h rl.h rm.h rp.h xp.h
ioconf.o: ${I}/sys/hpreg.h
	${C} ioconf.c
	${E} ioconf.s
	${A} ioconf.o ioconf.s
	-rm ioconf.s

acct.o: ../sys/acct.c
acct.o: param.h ${I}/sys/systm.h ${I}/sys/acct.h ${I}/sys/dir.h
acct.o: ${I}/sys/user.h ${I}/sys/inode.h ${I}/sys/proc.h ${I}/sys/seg.h
	${C} ../sys/acct.c
	${E} acct.s
	${A} acct.o acct.s
	-rm acct.s

alloc.o: ../sys/alloc.c
alloc.o: param.h ${I}/sys/systm.h ${I}/sys/filsys.h ${I}/sys/mount.h
alloc.o: ${I}/sys/fblk.h ${I}/sys/conf.h ${I}/sys/buf.h ${I}/sys/inode.h
alloc.o: ${I}/sys/ino.h ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/quota.h
	${C} ../sys/alloc.c
	${E} alloc.s
	${A} alloc.o alloc.s
	-rm alloc.s

clock.o: ../sys/clock.c
clock.o: param.h ${I}/sys/systm.h ${I}/sys/callout.h ${I}/sys/seg.h
clock.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/proc.h ${I}/sys/reg.h
clock.o: ${I}/sys/text.h ${I}/sys/vm.h
	${C} ../sys/clock.c
	${E} clock.s
	${A} clock.o clock.s
	-rm clock.s

fio.o: ../sys/fio.c
fio.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
fio.o: ${I}/sys/filsys.h ${I}/sys/file.h ${I}/sys/conf.h ${I}/sys/inode.h
fio.o: ${I}/sys/reg.h ${I}/sys/acct.h
	${C} ../sys/fio.c
	${E} fio.s
	${A} fio.o fio.s
	-rm fio.s

iget.o: ../sys/iget.c
iget.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
iget.o: ${I}/sys/inode.h ${I}/sys/ino.h ${I}/sys/filsys.h
iget.o: ${I}/sys/mount.h ${I}/sys/conf.h ${I}/sys/buf.h ${I}/sys/quota.h
iget.o: ${I}/sys/inline.h
	${C} ../sys/iget.c
	${E} iget.s
	${A} iget.o iget.s
	-rm iget.s

ioctl.o: ../sys/ioctl.c
ioctl.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
ioctl.o: ${I}/sys/tty.h ${I}/sys/proc.h ${I}/sys/mx.h ${I}/sys/inode.h
ioctl.o: ${I}/sys/file.h ${I}/sys/reg.h ${I}/sys/conf.h
	${C} ../sys/ioctl.c
	${E} ioctl.s
	${A} ioctl.o ioctl.s
	-rm ioctl.s

machdep.o: ../sys/machdep.c
machdep.o: param.h ${I}/sys/systm.h ${I}/sys/acct.h ${I}/sys/dir.h
machdep.o: ${I}/sys/user.h ${I}/sys/inode.h ${I}/sys/proc.h
machdep.o: ${I}/sys/seg.h ${I}/sys/map.h ${I}/sys/reg.h ${I}/sys/buf.h
machdep.o: ${I}/sys/tty.h ${I}/sys/reboot.h ${I}/sys/uba.h
machdep.o: ${I}/sys/iopage.h
	${C} ../sys/machdep.c
	${E} machdep.s
	${A} machdep.o machdep.s
	-rm machdep.s

main.o: ../sys/main.c
main.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
main.o: ${I}/sys/filsys.h ${I}/sys/mount.h ${I}/sys/map.h ${I}/sys/proc.h
main.o: ${I}/sys/inode.h ${I}/sys/seg.h ${I}/sys/conf.h ${I}/sys/buf.h
	${C} ../sys/main.c
	${E} main.s
	${A} main.o main.s
	-rm main.s

malloc.o: ../sys/malloc.c
malloc.o: param.h ${I}/sys/systm.h ${I}/sys/map.h ${I}/sys/vm.h
	${C} ../sys/malloc.c
	${E} malloc.s
	${A} malloc.o malloc.s
	-rm malloc.s

nami.o: ../sys/nami.c
nami.o: param.h ${I}/sys/systm.h ${I}/sys/inode.h ${I}/sys/filsys.h
nami.o: ${I}/sys/mount.h ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/buf.h
nami.o: ${I}/sys/quota.h ${I}/sys/seg.h
	${C} ../sys/nami.c
	${E} nami.s
	${A} nami.o nami.s
	-rm nami.s

pipe.o: ../sys/pipe.c
pipe.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
pipe.o: ${I}/sys/inode.h ${I}/sys/file.h ${I}/sys/reg.h ${I}/sys/inline.h
	${C} ../sys/pipe.c
	${E} pipe.s
	${A} pipe.o pipe.s
	-rm pipe.s

prf.o: ../sys/prf.c
prf.o: param.h ${I}/sys/systm.h ${I}/sys/filsys.h ${I}/sys/mount.h
prf.o: ${I}/sys/seg.h ${I}/sys/buf.h ${I}/sys/conf.h ${I}/sys/inline.h
prf.o: ${I}/sys/reboot.h
	${C} ../sys/prf.c
	${E} prf.s
	${A} prf.o prf.s
	-rm prf.s

prim.o: ../sys/prim.c
prim.o: param.h dh.h dz.h ${I}/sys/tty.h ${I}/sys/systm.h ${I}/sys/conf.h
prim.o: ${I}/sys/buf.h ${I}/sys/seg.h ${I}/sys/map.h ${I}/sys/uba.h
	${C} ../sys/prim.c
	${E} prim.s
	${A} prim.o prim.s
	-rm prim.s

rdwri.o: ../sys/rdwri.c
rdwri.o: param.h ${I}/sys/systm.h ${I}/sys/inode.h ${I}/sys/dir.h
rdwri.o: ${I}/sys/user.h ${I}/sys/buf.h ${I}/sys/conf.h
	${C} ../sys/rdwri.c
	${E} rdwri.s
	${A} rdwri.o rdwri.s
	-rm rdwri.s

sig.o: ../sys/sig.c
sig.o: whoami.h
	${C} ../sys/sig.c
	${E} sig.s
	${A} sig.o sig.s
	-rm sig.s

slp.o: ../sys/slp.c
slp.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
slp.o: ${I}/sys/proc.h ${I}/sys/text.h ${I}/sys/map.h ${I}/sys/file.h
slp.o: ${I}/sys/inode.h ${I}/sys/buf.h ${I}/sys/seg.h ${I}/sys/vm.h
slp.o: ${I}/sys/inline.h
	${C} ../sys/slp.c
	${E} slp.s
	${A} slp.o slp.s
	-rm slp.s

subr.o: ../sys/subr.c
subr.o: param.h ${I}/sys/systm.h ${I}/sys/conf.h ${I}/sys/inode.h
subr.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/buf.h ${I}/sys/quota.h
	${C} ../sys/subr.c
	${E} subr.s
	${A} subr.o subr.s
	-rm subr.s

sys1.o: ../sys/sys1.c
sys1.o: param.h ${I}/sys/systm.h ${I}/sys/map.h ${I}/sys/dir.h
sys1.o: ${I}/sys/user.h ${I}/sys/proc.h ${I}/sys/buf.h ${I}/sys/reg.h
sys1.o: ${I}/sys/inode.h ${I}/sys/seg.h ${I}/sys/acct.h ${I}/sys/file.h
sys1.o: ${I}/wait.h
	${C} ../sys/sys1.c
	${E} sys1.s
	${A} sys1.o sys1.s
	-rm sys1.s

sys2.o: ../sys/sys2.c
sys2.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
sys2.o: ${I}/sys/reg.h ${I}/sys/file.h ${I}/sys/inode.h ${I}/sys/quota.h
sys2.o: ${I}/sys/proc.h ${I}/sys/inline.h
	${C} ../sys/sys2.c
	${E} sys2.s
	${A} sys2.o sys2.s
	-rm sys2.s

sys3.o: ../sys/sys3.c
sys3.o: param.h ${I}/sys/systm.h ${I}/sys/ino.h ${I}/sys/reg.h
sys3.o: ${I}/sys/buf.h ${I}/sys/filsys.h ${I}/sys/mount.h ${I}/sys/dir.h
sys3.o: ${I}/sys/user.h ${I}/sys/inode.h ${I}/sys/file.h ${I}/sys/conf.h
sys3.o: ${I}/sys/stat.h ${I}/sys/inline.h
	${C} ../sys/sys3.c
	${E} sys3.s
	${A} sys3.o sys3.s
	-rm sys3.s

sys4.o: ../sys/sys4.c
sys4.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
sys4.o: ${I}/sys/reg.h ${I}/sys/inode.h ${I}/sys/proc.h ${I}/sys/timeb.h
sys4.o: ${I}/sys/quota.h ${I}/sys/reboot.h ${I}/sys/filsys.h
	${C} ../sys/sys4.c
	${E} sys4.s
	${A} sys4.o sys4.s
	-rm sys4.s

sysent.o: ../sys/sysent.c
sysent.o: param.h ${I}/sys/systm.h ${I}/sys/quota.h
	${C} ../sys/sysent.c
	${E} sysent.s
	${A} sysent.o sysent.s
	-rm sysent.s

syslocal.o: ../sys/syslocal.c
syslocal.o: param.h ${I}/sys/dir.h ${I}/sys/proc.h ${I}/sys/seg.h
syslocal.o: ${I}/sys/user.h ${I}/sys/systm.h ${I}/sys/inode.h
syslocal.o: ${I}/sys/ino.h ${I}/sys/file.h ${I}/sys/conf.h
syslocal.o: ${I}/sys/quota.h ${I}/sys/qstat.h ${I}/sys/buf.h
syslocal.o: ${I}/sys/tty.h ${I}/sys/autoconfig.h
	${C} ../sys/syslocal.c
	${E} syslocal.s
	${A} syslocal.o syslocal.s
	-rm syslocal.s

text.o: ../sys/text.c
text.o: param.h ${I}/sys/systm.h ${I}/sys/map.h ${I}/sys/dir.h
text.o: ${I}/sys/user.h ${I}/sys/proc.h ${I}/sys/text.h ${I}/sys/inode.h
text.o: ${I}/sys/buf.h ${I}/sys/seg.h
	${C} ../sys/text.c
	${E} text.s
	${A} text.o text.s
	-rm text.s

trap.o: ../sys/trap.c
trap.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
trap.o: ${I}/sys/proc.h ${I}/sys/reg.h ${I}/sys/seg.h ${I}/sys/vm.h
trap.o: ${I}/sys/trap.h ${I}/sys/inline.h ${I}/sys/iopage.h
	${C} ../sys/trap.c
	${E} trap.s
	${A} trap.o trap.s
	-rm trap.s

ureg.o: ../sys/ureg.c
ureg.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
ureg.o: ${I}/sys/proc.h ${I}/sys/text.h ${I}/sys/seg.h
	${C} ../sys/ureg.c
	${E} ureg.s
	${A} ureg.o ureg.s
	-rm ureg.s

bio.o: ../dev/bio.c
bio.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
bio.o: ${I}/sys/buf.h ${I}/sys/conf.h ${I}/sys/proc.h ${I}/sys/seg.h
bio.o: ${I}/sys/vm.h ${I}/sys/uba.h
	${C} ../dev/bio.c
	${E} bio.s
	${A} bio.o bio.s
	-rm bio.s

dh.o: ../dev/dh.c
dh.o: dh.h param.h ${I}/sys/conf.h ${I}/sys/systm.h ${I}/sys/dir.h
dh.o: ${I}/sys/user.h ${I}/sys/file.h ${I}/sys/tty.h ${I}/sys/dhreg.h
dh.o: ${I}/sys/uba.h
	${C} ../dev/dh.c
	${E} dh.s
	${A} dh.o dh.s
	-rm dh.s

dkbad.o: ../dev/dkbad.c
dkbad.o: param.h ${I}/sys/buf.h ${I}/sys/dkbad.h
	${C} ../dev/dkbad.c
	${E} dkbad.s
	${A} dkbad.o dkbad.s
	-rm dkbad.s

dkleave.o: ../dev/dkleave.c
dkleave.o: param.h ${I}/sys/buf.h
	${C} ../dev/dkleave.c
	${E} dkleave.s
	${A} dkleave.o dkleave.s
	-rm dkleave.s

dn.o: ../dev/dn.c
dn.o: dn.h param.h ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/dnreg.h
	${C} ../dev/dn.c
	${E} dn.s
	${A} dn.o dn.s
	-rm dn.s

dsort.o: ../dev/dsort.c
dsort.o: param.h ${I}/sys/systm.h ${I}/sys/buf.h
	${C} ../dev/dsort.c
	${E} dsort.s
	${A} dsort.o dsort.s
	-rm dsort.s

dz.o: ../dev/dz.c
dz.o: dz.h param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
dz.o: ${I}/sys/tty.h ${I}/sys/file.h ${I}/sys/conf.h ${I}/sys/dzreg.h
dz.o: ${I}/sys/pdma.h
	${C} ../dev/dz.c
	${E} dz.s
	${A} dz.o dz.s
	-rm dz.s

hk.o: ../dev/hk.c
hk.o: hk.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
hk.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/uba.h ${I}/sys/inline.h
hk.o: ${I}/sys/hkreg.h ${I}/sys/dkbad.h
	${C} ../dev/hk.c
	${E} hk.s
	${A} hk.o hk.s
	-rm hk.s

hp.o: ../dev/hp.c
hp.o: hp.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
hp.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/seg.h ${I}/sys/hpreg.h
hp.o: ${I}/sys/inline.h ${I}/sys/uba.h
	${C} ../dev/hp.c
	${E} hp.s
	${A} hp.o hp.s
	-rm hp.s

hs.o: ../dev/hs.c
hs.o: hs.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
hs.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/seg.h ${I}/sys/hsreg.h
	${C} ../dev/hs.c
	${E} hs.s
	${A} hs.o hs.s
	-rm hs.s

ht.o: ../dev/ht.c
ht.o: ht.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
ht.o: ${I}/sys/dir.h ${I}/sys/file.h ${I}/sys/user.h ${I}/sys/htreg.h
ht.o: ${I}/sys/mtio.h
	${C} ../dev/ht.c
	${E} ht.s
	${A} ht.o ht.s
	-rm ht.s

kl.o: ../dev/kl.c
kl.o: kl.h param.h ${I}/sys/conf.h ${I}/sys/dir.h ${I}/sys/user.h
kl.o: ${I}/sys/tty.h ${I}/sys/systm.h ${I}/sys/klreg.h
	${C} ../dev/kl.c
	${E} kl.s
	${A} kl.o kl.s
	-rm kl.s

lp.o: ../dev/lp.c
lp.o: lp.h param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
lp.o: ${I}/sys/tty.h ${I}/sys/lpreg.h
	${C} ../dev/lp.c
	${E} lp.s
	${A} lp.o lp.s
	-rm lp.s

mem.o: ../dev/mem.c
mem.o: param.h ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/conf.h
mem.o: ${I}/sys/seg.h
	${C} ../dev/mem.c
	${E} mem.s
	${A} mem.o mem.s
	-rm mem.s

partab.o: ../dev/partab.c
partab.o: param.h
	${C} ../dev/partab.c
	${E} partab.s
	${A} partab.o partab.s
	-rm partab.s

rk.o: ../dev/rk.c
rk.o: rk.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
rk.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/rkreg.h
	${C} ../dev/rk.c
	${E} rk.s
	${A} rk.o rk.s
	-rm rk.s

rl.o: ../dev/rl.c
rl.o: rl.h param.h ${I}/sys/buf.h ${I}/sys/dir.h ${I}/sys/user.h
rl.o: ${I}/sys/systm.h ${I}/sys/conf.h ${I}/sys/rlreg.h
	${C} ../dev/rl.c
	${E} rl.s
	${A} rl.o rl.s
	-rm rl.s

rm.o: ../dev/rm.c
rm.o: rm.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
rm.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/hpreg.h ${I}/sys/inline.h
rm.o: ${I}/sys/seg.h ${I}/sys/uba.h
	${C} ../dev/rm.c
	${E} rm.s
	${A} rm.o rm.s
	-rm rm.s

rp.o: ../dev/rp.c
rp.o: rp.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/dir.h
rp.o: ${I}/sys/conf.h ${I}/sys/user.h ${I}/sys/rpreg.h
	${C} ../dev/rp.c
	${E} rp.s
	${A} rp.o rp.s
	-rm rp.s

sys.o: ../dev/sys.c
sys.o: param.h ${I}/sys/conf.h ${I}/sys/dir.h ${I}/sys/user.h
sys.o: ${I}/sys/tty.h ${I}/sys/proc.h
	${C} ../dev/sys.c
	${E} sys.s
	${A} sys.o sys.s
	-rm sys.s

tm.o: ../dev/tm.c
tm.o: tm.h param.h ${I}/sys/buf.h ${I}/sys/dir.h ${I}/sys/conf.h
tm.o: ${I}/sys/file.h ${I}/sys/user.h ${I}/sys/systm.h ${I}/sys/tmreg.h
tm.o: ${I}/sys/mtio.h
	${C} ../dev/tm.c
	${E} tm.s
	${A} tm.o tm.s
	-rm tm.s

ts.o: ../dev/ts.c
ts.o: ts.h param.h ${I}/sys/buf.h ${I}/sys/dir.h ${I}/sys/conf.h
ts.o: ${I}/sys/file.h ${I}/sys/user.h ${I}/sys/tsreg.h ${I}/sys/mtio.h
	${C} ../dev/ts.c
	${E} ts.s
	${A} ts.o ts.s
	-rm ts.s

tty.o: ../dev/tty.c
tty.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
tty.o: ${I}/sys/tty.h ${I}/sys/proc.h ${I}/sys/mx.h ${I}/sys/inode.h
tty.o: ${I}/sys/file.h ${I}/sys/reg.h ${I}/sys/conf.h ${I}/sys/buf.h bk.h
	${C} ../dev/tty.c
	${E} tty.s
	${A} tty.o tty.s
	-rm tty.s

ttynew.o: ../dev/ttynew.c
ttynew.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
ttynew.o: ${I}/sys/tty.h ${I}/sys/proc.h ${I}/sys/mx.h ${I}/sys/inode.h
ttynew.o: ${I}/sys/file.h ${I}/sys/reg.h ${I}/sys/conf.h ${I}/sys/buf.h
	${C} ../dev/ttynew.c
	${E} ttynew.s
	${A} ttynew.o ttynew.s
	-rm ttynew.s

ttyold.o: ../dev/ttyold.c
ttyold.o: param.h ${I}/sys/systm.h ${I}/sys/dir.h ${I}/sys/user.h
ttyold.o: ${I}/sys/tty.h ${I}/sys/proc.h ${I}/sys/mx.h ${I}/sys/inode.h
ttyold.o: ${I}/sys/file.h ${I}/sys/reg.h ${I}/sys/conf.h ${I}/sys/buf.h
	${C} ../dev/ttyold.c
	${E} ttyold.s
	${A} ttyold.o ttyold.s
	-rm ttyold.s

xp.o: ../dev/xp.c
xp.o: xp.h param.h ${I}/sys/systm.h ${I}/sys/buf.h ${I}/sys/conf.h
xp.o: ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/seg.h ${I}/sys/hpreg.h
xp.o: ${I}/sys/inline.h ${I}/sys/uba.h
	${C} ../dev/xp.c
	${E} xp.s
	${A} xp.o xp.s
	-rm xp.s

vp.o: ../dev/vp.c
vp.o: vp.h param.h ${I}/sys/dir.h ${I}/sys/user.h ${I}/sys/buf.h
	${C} ../dev/vp.c
	${E} vp.s
	${A} vp.o vp.s
	-rm vp.s

# DEPENDENCIES MUST END HERE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
