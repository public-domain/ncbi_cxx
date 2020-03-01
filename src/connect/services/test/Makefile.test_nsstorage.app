# $Id: Makefile.test_nsstorage.app 109054 2007-08-17 15:30:44Z didenko $

APP = test_nsstorage
SRC = test_nsstorage
LIB = ncbi_xblobstorage_netcache xconnserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

