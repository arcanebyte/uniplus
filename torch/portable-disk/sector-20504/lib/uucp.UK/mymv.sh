: 'move a file for mvq.sh, since its called in find it cannot use {} twice'
: 'to refer to the same file since the second time is ignored.'
: 'copy then remove the file, mv isnt used since they may be on different'
: 'filestores'
: 'should only rm if the cp completed OK'
OWNER=5
GROUP=5
CHOWN=/etc/chown
CHGRP=/bin/chgrp
cp $1 $2 && rm $1 && $CHOWN $OWNER $2 && $CHGRP $GROUP $2
