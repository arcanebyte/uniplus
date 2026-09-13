adb unix << 'EOF'
$m
0="		Default Vect	Contents	Bdev/Cdev""
rlio="rlio		0160		"o"	8/18"
hkio="hkio		0210		"o"	4/19"
rkio="rkio		0220		"o"	0/9"
tmio="tmio		0224		"o"	3/12"
htio="htio		0224		"o"	7/15"
tsio="tsio		0224		"o"	9/20"
xpio="xpio		0254		"o"	6/14"
rpio="rpio		0260		"o"	1/11"
dzin="dzin		 X  		"o"	-/21"
dzdma="dzdma		X+4 		"o"	-/21"
dhin="dhin		 Y  		"o"	-/4"
dhou="dhou		Y+4 		"o"	-/4"
lpio="lpio		 -  		"o"	-/2"
0="Virtual/physical addresses of CSR pointers in drivers"
xp_controller+4="Address of xp_addr = "o
xp_controller+4?"Value of xp_addr = "o
xp_drive+2="Address of xp_type[0] = "o
xp_drive+2+12="Address of xp_type[1] = "o
xp_drive+2+24="Address of xp_type[2] = "o
xp_drive+2+36="Address of xp_type[3] = "o
HKADDR="Address of HKADDR = "o
HKADDR?"Value of HKADDR = "o
RKADDR="Address of RKADDR = "o
RKADDR?"Value of RKADDR = "o
RLADDR="Address of RLADDR = "o
RLADDR?"Value of RLADDR = "o
RPADDR="Address of RPADDR = "o
RPADDR?"Value of RPADDR = "o
TMADDR="Address of TMADDR = "o
HTADDR="Address of HTADDR = "o
TSADDR="Address of TSADDR = "o
dz_addr="Address of dz_addr = "o
dh_addr="Address of dh_addr = "o
lp_addr="Address of lp_addr = "o
rootdev="Address of rootdev = "o
pipedev="Address of pipedev = "o
swapdev="Address of swapdev = "o
swplo="Address of swplo = "o" (swplo is a long)"
nswap="Address of nswap = "o" (nswap is a short)"
$q
'EOF'
