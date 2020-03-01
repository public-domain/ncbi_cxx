#################################
# $Id: Makefile.test_compress_mt.app 128102 2008-05-20 16:14:25Z ivanov $

APP = test_compress_mt
SRC = test_compress_mt
LIB = xutil xcompress $(CMPRS_LIB) test_mt xncbi
LIBS = $(CMPRS_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

CHECK_CMD = test_compress_mt z
CHECK_CMD = test_compress_mt bz2
CHECK_CMD = test_compress_mt lzo
CHECK_TIMEOUT = 250

