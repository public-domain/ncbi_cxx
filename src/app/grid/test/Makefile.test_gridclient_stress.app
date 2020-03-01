# $Id: Makefile.test_gridclient_stress.app 142501 2008-10-07 16:35:25Z kazimird $

APP = test_gridclient_stress
SRC = test_gridclient_stress
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = test_gridclient_stress.ini
