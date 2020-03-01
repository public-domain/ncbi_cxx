# $Id: Makefile.rcgi_sample.app 113128 2007-10-30 14:26:25Z ucko $

# Build test CGI application "cgi_sample"
#################################

APP = rcgi_sample
SRC = rcgi_sample

REQUIRES = MT

# new_project.sh will copy everything in the following block to any
# Makefile.*_app generated from this sample project.  Do not change
# the lines reading "### BEGIN/END COPIED SETTINGS" in any way.

### BEGIN COPIED SETTINGS
## Use these two lines for normal CGI.
LIB = xgridcgi ncbi_xblobstorage_netcache xconnserv xthrserv xcgi xhtml xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

## If you need the C toolkit...
# LIBS     = $(NCBI_C_LIBPATH) -lncbi $(NETWORK_LIBS) $(ORIG_LIBS)
# CPPFLAGS = $(ORIG_CPPFLAGS) $(NCBI_C_INCLUDE)
### END COPIED SETTINGS

#CHECK_CMD  =
#CHECK_COPY = rcgi_sample.html rcgi_sample.ini