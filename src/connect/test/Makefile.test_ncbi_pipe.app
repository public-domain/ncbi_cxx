# $Id: Makefile.test_ncbi_pipe.app 78977 2006-03-16 12:43:54Z ivanov $

APP = test_ncbi_pipe
SRC = test_ncbi_pipe
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
