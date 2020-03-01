# $Id: Makefile.gene_info_reader.app 134408 2008-07-17 22:15:50Z ucko $

REQUIRES = algo

ASN_DEP = seq

APP = gene_info_reader
SRC = gene_info_reader_app

LIB_ = gene_info xobjutil seqdb blastdb $(SOBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

