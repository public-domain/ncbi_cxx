# $Id: Makefile.test_ncbi_disp.app 101876 2007-04-10 20:03:58Z ucko $

APP = test_ncbi_disp
SRC = test_ncbi_disp
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
