# $Id: Makefile.cgi_session_sample.app 108810 2007-08-14 13:23:45Z ivanov $

# Build test CGI application "cgi_sample"
#################################

APP = cgi_session_sample.cgi
SRC = cgi_session_sample

# new_project.sh will copy everything in the following block to any
# Makefile.*_app generated from this sample project.  Do not change
# the lines reading "### BEGIN/END COPIED SETTINGS" in any way.

### BEGIN COPIED SETTINGS
LIB = xgridcgi ncbi_xblobstorage_netcache xcgi xhtml xconnserv \
      xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

### END COPIED SETTINGS