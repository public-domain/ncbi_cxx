#################################
# $Id: Makefile.test_objmgr_gbloader_mt.app 145149 2008-11-07 15:10:38Z ucko $
#################################

REQUIRES = dbapi FreeTDS

APP = test_objmgr_gbloader_mt
SRC = test_objmgr_gbloader_mt
LIB = test_mt $(OBJMGR_LIBS) ncbi_xdbapi_ftds $(FTDS64_CTLIB_LIB) dbapi_driver$(STATIC)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = run_sybase_app.sh test_objmgr_loaders.sh test_objmgr_gbloader_mt /CHECK_NAME=test_objmgr_gbloader_mt
CHECK_COPY = test_objmgr_loaders.sh
CHECK_TIMEOUT = 400