# $Id: Makefile.test_fw.app 101876 2007-04-10 20:03:58Z ucko $

APP = test_fw
SRC = test_fw
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
