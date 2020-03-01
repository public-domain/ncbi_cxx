# $Id: Makefile.ns_submit_remote_job.app 86178 2006-07-17 14:42:03Z ucko $

APP = ns_submit_remote_job
SRC = ns_submit_remote_job
LIB = ncbi_xblobstorage_netcache xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

