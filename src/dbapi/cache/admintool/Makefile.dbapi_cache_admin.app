# $Id: Makefile.dbapi_cache_admin.app 66353 2005-07-21 18:54:46Z kuznets $


APP = dbapi_cache_admin
SRC = dbapi_cache_admin


LIB  = ncbi_xcache_dbapi dbapi ncbi_xdbapi_ftds $(FTDS_LIB) dbapi_driver \
       xutil xncbi

LIBS = $(FTDS_LIBS) $(ICONV_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


REQUIRES = FreeTDS



