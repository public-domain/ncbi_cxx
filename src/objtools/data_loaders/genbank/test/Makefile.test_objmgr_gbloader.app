#################################
# $Id: Makefile.test_objmgr_gbloader.app 145149 2008-11-07 15:10:38Z ucko $
#################################

REQUIRES = bdb dbapi FreeTDS

APP = test_objmgr_gbloader
SRC = test_objmgr_gbloader
LIB = $(BDB_CACHE_LIB) $(BDB_LIB) $(OBJMGR_LIBS) ncbi_xdbapi_ftds $(FTDS64_CTLIB_LIB) dbapi_driver$(STATIC)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BERKELEYDB_LIBS) $(ORIG_LIBS)

CHECK_CMD = run_sybase_app.sh test_objmgr_loaders.sh test_objmgr_gbloader /CHECK_NAME=test_objmgr_gbloader
CHECK_COPY = test_objmgr_loaders.sh