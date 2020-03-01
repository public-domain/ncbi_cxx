#################################
# $Id: Makefile.netbvstore.app 103396 2007-05-03 00:44:38Z ucko $
#################################

APP = netbvstore
SRC = netbvstore

REQUIRES = MT bdb


LIB = $(BDB_LIB) $(COMPRESS_LIBS) xthrserv xconnserv \
      xconnect xutil xncbi 
LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
