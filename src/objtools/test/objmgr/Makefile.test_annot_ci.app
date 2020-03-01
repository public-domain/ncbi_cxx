#################################
# $Id: Makefile.test_annot_ci.app 118708 2008-01-31 17:58:33Z ucko $
# Author:  Aleksey Grichenko (grichenk@ncbi.nlm.nih.gov)
#################################

# Build seq-annot iterators test application "test_annot_ci"
#################################


APP = test_annot_ci
SRC = test_annot_ci
LIB = $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_annot_ci
CHECK_COPY = test_annot_ci.ini test_annot_entries.asn test_annot_res.asn
