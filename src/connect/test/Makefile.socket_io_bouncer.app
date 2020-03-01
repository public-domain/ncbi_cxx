# $Id: Makefile.socket_io_bouncer.app 101876 2007-04-10 20:03:58Z ucko $

APP = socket_io_bouncer
SRC = socket_io_bouncer
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)
