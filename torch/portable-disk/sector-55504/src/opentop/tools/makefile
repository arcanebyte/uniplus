################################################################################
# Makefile for Programmer's Pack 1                                             #
#                                                                              #
# Tools                                                                        #
#                                                                              #
# (C) TORCH Computers Ltd., 1986                                               #
################################################################################

# standard definitions

INCLUDE	=/usr/include
CFLAGS	=-DTORCH -D$(DSTATE) $(OOPT)
LFLAGS	=-m $(LIB)
LIB	=-lW -lres
OOPT	=-O
DSTATE	=

# default compilation rule to replace make's normal one

.c:
		$(CC) $(CFLAGS) $< $(LFLAGS) -o $*

all: resshow reslist


