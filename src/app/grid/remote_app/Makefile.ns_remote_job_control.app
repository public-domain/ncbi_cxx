# $Id: Makefile.ns_remote_job_control.app 86178 2006-07-17 14:42:03Z ucko $

APP = ns_remote_job_control
SRC = ns_remote_job_control info_collector renderer
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

