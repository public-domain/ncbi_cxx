# $Id: Makefile.localfinder.app 151845 2009-02-09 14:50:16Z ludwigf $
# Makefile for 'localfinder' app

SRC = local_finder
APP = localfinder

LIB  = xalgognomon xobjread xobjutil $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

# These are necessary to avoid build errors in some configurations
# (notably 32-bit SPARC WorkShop Release).
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)