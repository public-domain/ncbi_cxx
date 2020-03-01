# $Id: Makefile.test_ncbi_core.app 21867 2003-05-14 04:01:19Z lavr $

APP = test_ncbi_core
SRC = test_ncbi_core
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
