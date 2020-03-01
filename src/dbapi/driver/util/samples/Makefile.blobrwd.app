# $Id: Makefile.blobrwd.app 49300 2004-10-26 19:42:34Z ivanovsk $

APP = blobrwd
SRC = blobrwd

CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

LIB  = dbapi_util_blobstore dbapi_driver xcompress $(CMPRS_LIB) xutil xncbi
LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

