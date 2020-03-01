# $Id: Makefile.bdbtest_vmerge.app 104185 2007-05-18 13:40:29Z ivanov $

APP = bdbtest_vmerge
SRC = test_bdb_vmerge
LIB = xalgovmerge $(BDB_CACHE_LIB) $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)
