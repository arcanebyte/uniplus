################################################################################
#                                                                              #
# Make file for clock program                                                  #
#                                                                              #
# (C) TORCH Computers Ltd., 1986.                                              #
#                                                                              #
################################################################################

# standard make definitions

INCLUDE	=/usr/include
CFLAGS	=-DTORCH -D$(DSTATE) $(OOPT)
LFLAGS	=-m $(LIB)
LIB	=-lW -lres -lm
OOPT	=-O
DSTATE	=

# intermediate objects for the clock

OFILES	=	cl.main.o cl.window.o cl.antime.o cl.anface.o cl.digtime.o\
		cl.digface.o cl.mevent.o cl.imenu.o cl.date.o cl.close.o\
		cl.update.o cl.res.o

clock:		$(OFILES)
		$(CC) $(OFILES) $(LFLAGS) -o clock
		echo "\07\07\07\07"

$OFILES:	clock.h
