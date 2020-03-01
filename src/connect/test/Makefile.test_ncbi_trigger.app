# $Id: Makefile.test_ncbi_trigger.app 145005 2008-11-05 22:15:12Z lavr $

APP = test_ncbi_trigger
SRC = test_ncbi_trigger
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

REQUIRES = MT

CHECK_CMD = test_ncbi_trigger.sh
CHECK_COPY = test_ncbi_trigger.sh
