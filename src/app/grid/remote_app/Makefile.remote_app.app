# $Id: Makefile.remote_app.app 83312 2006-05-30 16:43:36Z didenko $
# Author:  Maxim Didneko
#################################

APP = remote_app
SRC = remote_app_wn exec_helpers
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil xncbi 
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
