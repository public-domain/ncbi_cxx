# $Id: Makefile.netcache_client_sample2.app 86178 2006-07-17 14:42:03Z ucko $

APP = netcache_client_sample2
SRC = netcache_client_sample2
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

