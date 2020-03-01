# $Id: Makefile.cgi_tunnel2grid.app 86178 2006-07-17 14:42:03Z ucko $

# Build test CGI application "cgi_tunnel2grid"
#################################

APP = cgi_tunnel2grid.cgi
SRC = cgi_tunnel2grid

LIB = xgridcgi ncbi_xblobstorage_netcache xconnserv xthrserv \
      xcgi xhtml xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

#CHECK_CMD  =
CHECK_COPY = cgi_tunnel2grid_sample.html cgi_tunnel2grid.ini
