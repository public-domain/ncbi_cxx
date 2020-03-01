# $Id: Makefile.netcache_client_sample1.app 86178 2006-07-17 14:42:03Z ucko $

APP = netcache_client_sample1
SRC = netcache_client_sample1
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

