# $Id: Makefile.writedb_unit_test.app 151845 2009-02-09 14:50:16Z ludwigf $

APP = writedb_unit_test
SRC = writedb_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS)

LIB_ = writedb seqdb xobjread xobjutil creaders blastdb \
       $(SOBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))
LIBS = $(BOOST_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = writedb_unit_test
CHECK_AUTHORS = blastsoft
CHECK_COPY = data
