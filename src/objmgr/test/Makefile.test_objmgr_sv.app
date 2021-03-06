#################################
# $Id: Makefile.test_objmgr_sv.app 149718 2009-01-14 19:43:57Z grichenk $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr"
#################################

APP = test_objmgr_sv
SRC = test_objmgr_sv
LIB = $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr_sv -seed 1 -checksum c78cb2fb4d1b2926fede0945d9ae88b9 /CHECK_NAME=test_objmgr_sv
