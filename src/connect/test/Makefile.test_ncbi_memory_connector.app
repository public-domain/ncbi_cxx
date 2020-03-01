# $Id: Makefile.test_ncbi_memory_connector.app 16318 2002-12-04 17:00:39Z lavr $

APP = test_ncbi_memory_connector
SRC = test_ncbi_memory_connector
LIB = xconntest connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
