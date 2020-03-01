# $Id: Makefile.dbapi_bcp.app 77473 2006-02-16 17:35:19Z ssikorsk $

APP = dbapi_bcp
SRC = dbapi_bcp

LIB  = dbapi_sample_base dbapi_driver $(XCONNEXT) xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS) $(DL_LIBS)

CHECK_COPY = dbapi_bcp.ini
