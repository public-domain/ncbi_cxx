# $Id: Makefile.blobreader.app 49054 2004-10-22 12:14:59Z ivanov $

APP = blobreader
SRC = blobreader

CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

LIB  = dbapi_util_blobstore dbapi_driver xcompress $(CMPRS_LIB) xutil xncbi
LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)
