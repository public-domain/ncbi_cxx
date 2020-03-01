# $Id: Makefile.gene_info_writer_unit_test.app 148871 2009-01-05 16:51:12Z camacho $

APP = gene_info_writer_unit_test
SRC = gene_info_writer_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
CXXFLAGS = $(FAST_CXXFLAGS)
LOCAL_LDFLAGS += -L. -L..
LDFLAGS = $(LOCAL_LDFLAGS) $(FAST_LDFLAGS)

LIB_ = gene_info_writer gene_info xobjutil seqdb blastdb $(SOBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

LIBS = $(BOOST_LIBS) \
       $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD  = gene_info_writer_unit_test
CHECK_COPY = data
CHECK_AUTHORS = blastsoft
