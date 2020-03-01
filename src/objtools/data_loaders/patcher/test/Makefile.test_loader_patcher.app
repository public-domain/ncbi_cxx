#################################
# $Id: Makefile.test_loader_patcher.app 118708 2008-01-31 17:58:33Z ucko $
#################################

APP = test_loader_patcher
SRC = test_loader_patcher

LIB = ncbi_xloader_patcher $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

