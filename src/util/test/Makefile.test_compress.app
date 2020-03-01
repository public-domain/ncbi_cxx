#################################
# $Id: Makefile.test_compress.app 128102 2008-05-20 16:14:25Z ivanov $

APP = test_compress
SRC = test_compress
LIB = xutil xcompress $(CMPRS_LIB) xncbi
LIBS = $(CMPRS_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

CHECK_CMD = test_compress z
CHECK_CMD = test_compress bz2
CHECK_CMD = test_compress lzo
