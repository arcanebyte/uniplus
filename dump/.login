setenv TERM vtl
tset -s '-e^H' '-k' vtl >/tmp/$$
# next line shows how to query the user for a terminal type
# at startup. Hitting CR defaults to vtl
#tset -s '-e^H' '-k' -m:\?vtl >/tmp/$$
source /tmp/$$
/bin/rm /tmp/$$
stty -tabs
setenv EXINIT "set redraw showmatch"
