# $Id: Makefile.winmasker.app 151845 2009-02-09 14:50:16Z ludwigf $

REQUIRES = objects algo

ASN_DEP = seq

APP = windowmasker
SRC = main win_mask_app win_mask_config win_mask_dup_table \
      win_mask_gen_counts win_mask_util win_mask_sdust_masker \
      win_mask_counts_converter

LIB = xalgowinmask xalgodustmask blast composition_adjustment seqdb blastdb \
	seqmasks_io tables xobjread creaders xobjutil \
	$(OBJMGR_LIBS:%=%$(STATIC))

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)
