# $Id: Makefile.test_ncbi_namedpipe_connector.app 79235 2006-03-20 20:52:25Z ivanov $

APP = test_ncbi_namedpipe_connector
SRC = test_ncbi_namedpipe_connector
LIB = xconntest xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS) 

CHECK_REQUIRES = -Cygwin

CHECK_CMD = test_ncbi_namedpipe_connector.sh
CHECK_COPY = test_ncbi_namedpipe_connector.sh
