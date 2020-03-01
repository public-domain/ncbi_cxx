# $Id: Makefile.bdbtestthr.app 104185 2007-05-18 13:40:29Z ivanov $

APP = bdbtestthr
SRC = test_bdb_thr
LIB = $(BDB_CACHE_LIB) $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = MT bdb


CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)