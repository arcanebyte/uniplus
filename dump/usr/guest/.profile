# default terminal setups follow for lisa terminal: vtl
TERM=vtl
export TERM
EXINIT="set ai sm"
export EXINIT
tset -s '-e^H' '-k' vtl > $$
rm $$
stty ixon echoe echok -tabs
#
PATH=:/bin/:/usr/bin:/etc:.:
export PATH
