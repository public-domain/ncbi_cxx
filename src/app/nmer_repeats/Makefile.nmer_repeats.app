# $Id: Makefile.nmer_repeats.app 151845 2009-02-09 14:50:16Z ludwigf $
# Author:  Josh Cherry

# Build n-mer nucleotide repeat finder app

APP = nmer_repeats
SRC = nmer_repeats
LIB = xalgoseq xalnmgr xobjread creaders xobjutil \
      taxon1 xconnect tables xregexp $(PCRE_LIB) $(SOBJMGR_LIBS)
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

REQUIRES = objects algo
