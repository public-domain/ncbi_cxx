# $Id: Makefile.odbc_sp_who.app 76101 2006-01-24 12:53:25Z ssikorsk $

APP = odbc_sp_who
SRC = odbc_sp_who

REQUIRES = ODBC

LIB  = ncbi_xdbapi_odbc dbapi_driver $(XCONNEXT) xconnect xncbi
LIBS = $(ODBC_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ODBC_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_COPY = odbc_sp_who.ini
CHECK_CMD =
