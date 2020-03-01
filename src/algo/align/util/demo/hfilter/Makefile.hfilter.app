# $Id: Makefile.hfilter.app 86582 2006-07-24 14:59:47Z ucko $
#################################
# Build demo "hfilter"

APP = hfilter
SRC = hitfilter_app

LIB = xalgoalignutil xalnmgr xobjutil tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)
