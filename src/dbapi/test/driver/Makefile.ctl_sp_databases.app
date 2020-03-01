# $Id: Makefile.ctl_sp_databases.app 138116 2008-08-21 19:06:18Z ivanovp $

APP = ctl_sp_databases
SRC = ctl_sp_databases dbapi_driver_sample_base_driver

LIB  = dbapi$(STATIC) ncbi_xdbapi_ctlib$(STATIC) dbapi_driver$(STATIC) $(XCONNEXT) xconnect xncbi
LIBS = $(SYBASE_LIBS) $(SYBASE_DLLS) $(NETWORK_LIBS) $(ORIG_LIBS) $(DL_LIBS)

CPPFLAGS = $(SYBASE_INCLUDE) $(ORIG_CPPFLAGS)

REQUIRES = Sybase

CHECK_CMD = run_sybase_app.sh ctl_sp_databases /CHECK_NAME=ctl_sp_databases