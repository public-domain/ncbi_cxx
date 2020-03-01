#################################
# $Id: Makefile.objmgr_demo.app 152210 2009-02-12 16:17:56Z vasilche $
# Author:  Aleksey Grichenko (grichenk@ncbi.nlm.nih.gov)
#################################

# Build object manager demo application "objmgr_demo"
#################################

REQUIRES = objects bdb algo dbapi FreeTDS -Cygwin

APP = objmgr_demo
SRC = objmgr_demo
LIB = ncbi_xloader_blastdb ncbi_xloader_blastdb_rmt $(BLAST_LIBS) ncbi_xloader_lds lds bdb \
      xobjread xobjutil ncbi_xdbapi_ftds $(FTDS64_CTLIB_LIB) \
      dbapi_driver$(STATIC) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BERKELEYDB_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS)
