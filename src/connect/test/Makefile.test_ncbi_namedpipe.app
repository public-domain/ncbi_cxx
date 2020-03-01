# $Id: Makefile.test_ncbi_namedpipe.app 79235 2006-03-20 20:52:25Z ivanov $

APP = test_ncbi_namedpipe
SRC = test_ncbi_namedpipe
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = -Cygwin

CHECK_CMD = test_ncbi_namedpipe.sh
CHECK_COPY = test_ncbi_namedpipe.sh

