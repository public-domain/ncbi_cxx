# $Id: Makefile.db_copy.app 107233 2007-07-13 22:47:13Z vakatov $

REQUIRES = dbapi

APP = db_copy
SRC = db_copy

### BEGIN COPIED SETTINGS
LIB  = dbapi dbapi_driver xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
### END COPIED SETTINGS
