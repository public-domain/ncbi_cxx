#################################
# $Id: Makefile.streamtest.app 152164 2009-02-11 20:35:06Z ucko $
# Author:  Frank Ludwig
#################################

# Build application "streamtest"
#################################

APP = streamtest
SRC = streamtest
LIB = xobjutil xcleanup prosplign submit xalgoalignutil xalnmgr tables \
      $(COMPRESS_LIBS) $(SOBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects algo
