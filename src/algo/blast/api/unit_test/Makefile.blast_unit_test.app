# $Id: Makefile.blast_unit_test.app 148871 2009-01-05 16:51:12Z camacho $

APP = blast_unit_test
# N.B.: if you remove sources, don't remove blast_unit_test lest you define
# BOOST_AUTO_TEST_MAIN in another source file
SRC = test_objmgr blast_test_util blast_unit_test bl2seq_unit_test \
	gencode_singleton_unit_test blastoptions_unit_test blastfilter_unit_test \
	uniform_search_unit_test remote_blast_unit_test aascan_unit_test \
	ntscan_unit_test version_reference_unit_test aalookup_unit_test \
	subj_ranges_unit_test blastengine_unit_test linkhsp_unit_test \
	blasthits_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) -I.. -I$(srcdir)/..

LIB_ = $(BLAST_INPUT_LIBS) ncbi_xloader_blastdb_rmt $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS) \
xalgowinmask
LIB = $(LIB_:%=%$(STATIC))

LIBS = $(BOOST_TEST_LIBS) $(NETWORK_LIBS) \
		$(PCRE_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS)

REQUIRES = objects

CHECK_CMD = blast_unit_test
CHECK_COPY = data
CHECK_AUTHORS = blastsoft
CHECK_TIMEOUT = 750