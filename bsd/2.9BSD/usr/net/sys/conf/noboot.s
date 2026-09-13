#include "whoami.h"

#ifdef	UCB_AUTOBOOT

/  This file is a dummy to keep the Makefile the same with AUTOBOOT disabled.
/  It should be replaced by the correct boot file from sys/conf
/  to actually use it.

/  Intentional error if AUTOBOOT is enabled with this file is installed:
	jmp	How2Boot	/ WRONG!!!

#endif	UCB_AUTOBOOT
