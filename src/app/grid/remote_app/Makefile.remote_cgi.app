# $Id: Makefile.remote_cgi.app 83313 2006-05-30 16:45:08Z didenko $
# Author:  Maxim Didneko
#################################

APP = remote_cgi
SRC = remote_cgi_wn exec_helpers
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xcgi xutil xncbi 
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

