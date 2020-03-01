# $Id: Makefile.dbapi_cache_test.app 94723 2006-12-05 02:10:52Z ucko $

APP = dbapi_cache_test
SRC = dbapi_cache_test

LIB  = ncbi_xcache_dbapi dbapi dbapi_driver xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)
