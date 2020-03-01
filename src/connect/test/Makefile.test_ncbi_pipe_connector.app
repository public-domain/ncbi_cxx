# $Id: Makefile.test_ncbi_pipe_connector.app 105038 2007-06-01 19:17:56Z lavr $

APP = test_ncbi_pipe_connector
SRC = test_ncbi_pipe_connector
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
