# $Id: Makefile.test_ncbi_buffer.app 21867 2003-05-14 04:01:19Z lavr $

APP = test_ncbi_buffer
SRC = test_ncbi_buffer
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_buffer.sh
CHECK_COPY = test_ncbi_buffer.sh

