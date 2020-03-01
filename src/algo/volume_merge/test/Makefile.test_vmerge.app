#################################
# $Id: Makefile.test_vmerge.app 93819 2006-11-17 07:30:47Z kuznets $
# Author:  Anatoliy Kuznetsov
#################################



APP = test_vmerge
SRC = test_vmerge
LIB = xalgovmerge xutil xncbi

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS)

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


