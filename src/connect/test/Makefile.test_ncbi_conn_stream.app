# $Id: Makefile.test_ncbi_conn_stream.app 27628 2003-09-22 20:33:02Z ucko $

APP = test_ncbi_conn_stream
SRC = test_ncbi_conn_stream
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
