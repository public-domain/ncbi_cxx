# $Id: Makefile.bdb_env_keeper.app 107010 2007-07-10 19:42:50Z kuznets $

APP = bdb_env_keeper
SRC = bdb_env_keeper
LIB = xthrserv xconnect xutil xncbi


LIB =  $(BDB_LIB) $(COMPRESS_LIBS) xconnserv xthrserv xconnect xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


REQUIRES = MT bdb

