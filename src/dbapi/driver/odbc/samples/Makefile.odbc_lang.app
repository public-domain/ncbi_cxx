# $Id: Makefile.odbc_lang.app 76101 2006-01-24 12:53:25Z ssikorsk $

APP = odbc_lang
SRC = odbc_lang

REQUIRES = ODBC

LIB  = ncbi_xdbapi_odbc dbapi_driver $(XCONNEXT) xconnect xncbi
LIBS = $(ODBC_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ODBC_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_COPY = odbc_lang.ini
CHECK_CMD =
