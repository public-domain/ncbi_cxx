#################################
# $Id: Makefile.test_edit_saver.app 118708 2008-01-31 17:58:33Z ucko $
# Author:  Maxim Didenko (didenko@ncbi.nlm.nih.gov)
#################################

APP = test_edit_saver
SRC = test_edit_saver
LIB = xobjmgr xobjutil ncbi_xloader_patcher $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_edit_saver -gi 45678
CHECK_CMD = test_edit_saver -gi 21225451 
