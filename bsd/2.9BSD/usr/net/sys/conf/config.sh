:	script to set up a new kernel configuration directory.
:	Usage:	MAKE machineid
:	to make a directory for "machineid"


case $# in
    1) ;;

    *) echo "Usage: MAKE machinename"
	echo " to make a configuration directory ../machinename"
	echo " from the description file ./machinename"
	echo " and copy the necessary files into it."
	exit 1
	;;
esac

:  set defaults
pdp11="GENERIC"
ident="UNKNOWN"
maxusers=4
bootdev="none"
timezone=8
dst=1
rootdev="UNSPECIFIED"
swapdev="UNSPECIFIED"
dumpdev="UNSPECIFIED"
pipedev="UNSPECIFIED"
swplo="UNSPECIFIED"
nswap="UNSPECIFIED"
dumplo="UNSPECIFIED"
dumproutine="UNSPECIFIED"

NBK=0 NDH=0 NDM=0 LOWDM=0 NDN=0 NDZ=0 NHK=0 NHP=0
NHS=0 NHT=0 NKL=1 NLP=0 NRF=0 NRK=0 NRL=0
NRM=0 NRP=0 NTM=0 NTS=0 NVP=0 NXP=0 NXP_CONTROLLER=0

machine=$1

: read in specification file
eval `awk '/^#/ {next} $1 != "" { printf "%s=\\"%s\\"\\n", $1, $2 }' $machine`

case $pdp11 in
	GENERIC|40|60)
		splfix=:splfix.movb
		makefile=Ovmakefile
		;;
	34|23|24)
		splfix=:splfix.mtps
		makefile=Ovmakefile
		;;
	44|45|70)
		splfix=:splfix.spl
		makefile=Makefile
		;;
	*)
		echo "Unrecognized PDP11 type"
		exit 1
		;;
esac
if [ $NKL -lt 1 ]
then
	echo "NKL must be at least one (for the console)"
	exit 1
fi

if [ -d ../$machine ]
then
	echo "../$machine: directory exists"
	exit 1
fi
echo "Making ../$machine."
mkdir ../$machine

: copy in the standard configuration files, etc.

echo "Copying in configuration files."
cp c.c genassym.c ioconf.c l.s param.c ../$machine
cp whoami.h localopts.h param.h ../$machine
cp bk.h dh.h dn.h dz.h hk.h hp.h hs.h ht.h kl.h lp.h ../$machine
cp rk.h rl.h rm.h rp.h tm.h ts.h vp.h xp.h ../$machine
cp :comm-to-bss $splfix ../$machine
cp Depend newvers.sh checksys.c ../$machine
chmod 664 ../$machine/param.h

: copy in the cpu-dependent files and modify as necessary

echo "Setting up boot.s, Makefile, whoami.h, param.c, ioconf.c."
cp $makefile ../$machine/Makefile
if [ $bootdev = "none" ]
then
	bootdev="no"
fi
cp ${bootdev}boot.s ../$machine/boot.s

chmod 664 ../$machine/Makefile 
ed - ../$machine/Makefile << EOF
/%SPLFIX%/s//$splfix/
/%MAXUSERS%/s//$maxusers/
w
q
EOF

chmod 664 ../$machine/whoami.h 
ex - ../$machine/whoami.h << EOF
/%PDP%/s//$pdp11/
/%IDENT%/s//$ident/
/%ident%/s//\L$ident/
w
q
EOF

chmod 664 ../$machine/param.c
ed - ../$machine/param.c << EOF
/%TIMEZONE%/s//$timezone/
/%DST%/s//$dst/
w
q
EOF

chmod 664 ../$machine/ioconf.c
ed - ../$machine/ioconf.c << EOF
/%ROOTDEV%/s//$rootdev/
/%SWAPDEV%/s//$swapdev/
/%PIPEDEV%/s//$pipedev/
/%DUMPDEV%/s//$dumpdev/
/%NSWAP%/s//$nswap/
/%SWPLO%/s//$swplo/
/%DUMPLO%/s//$dumplo/
g/%DUMPROUTINE%/s//$dumproutine/
w
q
EOF

:  Now edit all of the device header files.

echo "Setting up device header files."
for hd in bk.h dh.h dn.h dz.h hk.h hp.h hs.h ht.h kl.h lp.h rk.h rl.h rm.h rp.h tm.h ts.h vp.h xp.h 
do
	chmod 664 ../$machine/$hd
done

ed - ../$machine/bk.h << EOF
/%NBK%/s//$NBK/
w
q
EOF
ed - ../$machine/dh.h << EOF
/%NDH%/s//$NDH/
/%NDM%/s//$NDM/
/%LOWDM%/s//$LOWDM/
w
q
EOF
ed - ../$machine/dn.h << EOF
/%NDN%/s//$NDN/
w
q
EOF
ed - ../$machine/dz.h << EOF
/%NDZ%/s//$NDZ/
w
q
EOF
ed - ../$machine/hk.h << EOF
/%NHK%/s//$NHK/
w
q
EOF
ed - ../$machine/hp.h << EOF
/%NHP%/s//$NHP/
w
q
EOF
ed - ../$machine/hs.h << EOF
/%NHS%/s//$NHS/
w
q
EOF
ed - ../$machine/ht.h << EOF
/%NHT%/s//$NHT/
w
q
EOF
ed - ../$machine/kl.h << EOF
/%NKL%/s//$NKL/
w
q
EOF
ed - ../$machine/lp.h << EOF
/%NLP%/s//$NLP/
w
q
EOF
ed - ../$machine/rk.h << EOF
/%NRK%/s//$NRK/
w
q
EOF
ed - ../$machine/rl.h << EOF
/%NRL%/s//$NRL/
w
q
EOF
ed - ../$machine/rm.h << EOF
/%NRM%/s//$NRM/
w
q
EOF
ed - ../$machine/rp.h << EOF
/%NRP%/s//$NRP/
w
q
EOF
ed - ../$machine/tm.h << EOF
/%NTM%/s//$NTM/
w
q
EOF
ed - ../$machine/ts.h << EOF
/%NTS%/s//$NTS/
w
q
EOF
ed - ../$machine/vp.h << EOF
/%NVP%/s//$NVP/
w
q
EOF
ed - ../$machine/xp.h << EOF
/%NXP%/s//$NXP/
/%NXP_CONTROLLER%/s//$NXP_CONTROLLER/
w
q
EOF

echo ""
echo "Now edit the header files in ../$machine to change any local options"
echo "or parameters in localopts.h, param.c and the device headers."
echo "Remember to edit Depend if you need to add optional device drivers,"
echo "then \"make depend\".  You will also have to add any optional files"
echo "to the load rules in Makefile."
case $makefile in
	Makefile)
		;;
	Ovmakefile)
		echo "You will probably also have to edit the load rules"
		echo "in the Makefile to tune the overlay sizes as well."
		;;
esac
