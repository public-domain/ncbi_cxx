# $Id: Makefile.test_ncbidll.app 50054 2004-11-09 16:20:22Z ivanov $

APP = test_ncbidll
SRC = test_ncbidll
LIB = xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = DLL
CHECK_CMD =
