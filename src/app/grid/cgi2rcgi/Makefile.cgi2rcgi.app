# $Id: Makefile.cgi2rcgi.app 86178 2006-07-17 14:42:03Z ucko $

# Build test CGI application "cgi_tunnel2grid"
#################################

APP = cgi2rcgi
SRC = cgi2rcgi

LIB = xgridcgi ncbi_xblobstorage_netcache xconnserv xthrserv \
      xcgi xhtml xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
