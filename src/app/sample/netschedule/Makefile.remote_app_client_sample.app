# $Id: Makefile.remote_app_client_sample.app 88482 2006-08-23 16:36:13Z didenko $

APP = remote_app_client_sample
SRC = remote_app_client_sample

### BEGIN COPIED SETTINGS

LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

### END COPIED SETTINGS
