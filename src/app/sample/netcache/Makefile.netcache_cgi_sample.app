# $Id: Makefile.netcache_cgi_sample.app 109054 2007-08-17 15:30:44Z didenko $

APP = netcache_cgi_sample.cgi
SRC = netcache_cgi_sample
LIB = ncbi_xblobstorage_netcache xconnserv xconnect xcgi xhtml xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

