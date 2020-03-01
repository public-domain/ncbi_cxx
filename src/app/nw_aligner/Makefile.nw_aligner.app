# $Id: Makefile.nw_aligner.app 110299 2007-09-11 18:11:55Z ucko $
# Author:  Yuri Kapustin

# Generic pairwise global alignment utility

APP = nw_aligner
SRC = nwa

LIB = xalgoalignnw tables $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

REQUIRES = objects algo
