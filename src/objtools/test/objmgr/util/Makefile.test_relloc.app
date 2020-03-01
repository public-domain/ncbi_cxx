#################################
# $Id: Makefile.test_relloc.app 118708 2008-01-31 17:58:33Z ucko $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)
#################################

# Build relative-location-computation test application "test_relloc"
#################################


APP = test_relloc
SRC = test_relloc
LIB = xobjutil $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
