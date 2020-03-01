#################################
# $Id: Makefile.test_reader_pubseq.app 118683 2008-01-31 14:45:44Z ucko $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build serialization test application "serialtest"
#################################

REQUIRES = PubSeqOS

APP = test_reader_pubseq
SRC = test_reader_pubseq

LIB = $(GENBANK_READER_PUBSEQOS_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
