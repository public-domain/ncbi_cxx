# $Id: Makefile.test_bvs_client.app 86178 2006-07-17 14:42:03Z ucko $

APP = test_bvs_client
SRC = test_bvs_client
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

