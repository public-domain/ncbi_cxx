# $Id: Makefile.netcache_client_sample3.app 86178 2006-07-17 14:42:03Z ucko $

APP = netcache_client_sample3
SRC = netcache_client_sample3
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil \
      xcompress $(CMPRS_LIB) xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

