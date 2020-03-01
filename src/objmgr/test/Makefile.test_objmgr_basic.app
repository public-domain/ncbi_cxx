#################################
# $Id: Makefile.test_objmgr_basic.app 109368 2007-08-23 19:39:51Z ucko $
#################################

APP = test_objmgr_basic
SRC = test_objmgr_basic
LIB = $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr_basic
