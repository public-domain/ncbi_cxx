# $Id: Makefile.demo1.app 104185 2007-05-18 13:40:29Z ivanov $

APP = bdb_demo1
SRC = demo1
LIB = bdb xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)
