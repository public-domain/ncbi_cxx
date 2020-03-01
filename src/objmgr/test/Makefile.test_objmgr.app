#################################
# $Id: Makefile.test_objmgr.app 109368 2007-08-23 19:39:51Z ucko $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr"
#################################

APP = test_objmgr
SRC = test_objmgr test_helper
LIB = $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr
