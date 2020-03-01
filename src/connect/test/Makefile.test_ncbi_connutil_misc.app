# $Id: Makefile.test_ncbi_connutil_misc.app 149827 2009-01-15 18:44:53Z ucko $

APP = test_ncbi_connutil_misc
SRC = test_ncbi_connutil_misc
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(C_LIBS)
LINK = $(C_LINK)
#LINK = purify $(C_LINK)

CHECK_CMD =
