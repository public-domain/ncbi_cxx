# $Id: Makefile.remote_app_dispatcher.app 86012 2006-07-13 15:20:50Z didenko $

APP = remote_app_dispatcher.cgi
SRC = remote_app_dispatcher
LIB = ncbi_xblobstorage_netcache xconnserv xconnect xcgi xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

