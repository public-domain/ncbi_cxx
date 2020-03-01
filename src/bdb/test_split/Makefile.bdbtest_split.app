# $Id: Makefile.bdbtest_split.app 104185 2007-05-18 13:40:29Z ivanov $

APP = bdbtest_split
SRC = test_bdb_split
LIB = $(BDB_CACHE_LIB) $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)
