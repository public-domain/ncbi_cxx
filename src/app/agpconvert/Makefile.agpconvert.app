# $Id: Makefile.agpconvert.app 151845 2009-02-09 14:50:16Z ludwigf $
# Author:  Josh Cherry

# Build AGP file converter app

APP = agpconvert
SRC = agpconvert
LIB = xalgoseq taxon1 submit xalnmgr xobjutil xobjread creaders \
      xregexp xconnect $(PCRE_LIB) tables $(SOBJMGR_LIBS)
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

REQUIRES = objects algo