# $Id: Makefile.blast_format_unit_test.app 146491 2008-11-26 13:57:23Z camacho $

APP = blast_format_unit_test
SRC = seqalignfilter_unit_test seq_writer_unit_test \
	showdefline_unit_test blastfmtutil_unit_test blast_test_util \
	showalign_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS)

LIB_ = $(BLAST_FORMATTER_LIBS) ncbi_xloader_blastdb_rmt $(BLAST_LIBS) \
	$(OBJMGR_LIBS)

LIB = $(LIB_:%=%$(STATIC))
LIBS = $(BOOST_LIBPATH) $(BOOST_TEST_UTF_LIBS) $(PCRE_LIBS) \
	$(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = blast_format_unit_test
CHECK_COPY = data

CHECK_AUTHORS = blastsoft