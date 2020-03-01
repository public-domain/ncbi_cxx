# $Id: Makefile.test_ic_client.app 151616 2009-02-05 17:54:27Z ivanovp $

APP = test_ic_client
SRC = test_ic_client
LIB = ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


CHECK_CMD = test_ic_client -service NC_test blobs
