#################################
# $Id: Makefile.test_objmgr_title.app 118708 2008-01-31 17:58:33Z ucko $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)
#################################

# Build title-computation test application "test_title"
#################################


APP = test_objmgr_title
SRC = test_objmgr_title
LIB = $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

