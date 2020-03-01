# $Id: Makefile.aln_build.app 97816 2007-01-31 00:22:38Z todorov $

APP = aln_build
SRC = aln_build_app
LIB = xalnmgr submit tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

# CPPFLAGS = $(ORIG_CPPFLAGS) -pg
# LDFLAGS  = $(ORIG_LDFLAGS) -pg
# CPPFLAGS = $(ORIG_CPPFLAGS) $(NCBI_C_INCLUDE)
# CFLAGS   = $(ORIG_CFLAGS)
# CXXFLAGS = $(ORIG_CXXFLAGS)
