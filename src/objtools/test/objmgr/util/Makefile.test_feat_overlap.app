#################################
# $Id: Makefile.test_feat_overlap.app 118708 2008-01-31 17:58:33Z ucko $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)
#################################

# Build title-computation test application "test_title"
#################################


APP = test_feat_overlap
SRC = test_feat_overlap
LIB = xobjutil $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

