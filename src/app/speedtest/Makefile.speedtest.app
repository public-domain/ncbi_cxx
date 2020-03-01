#################################
# $Id: Makefile.speedtest.app 138355 2008-08-25 15:26:16Z ucko $
# Author:  Frank Ludwig
#################################

# Build application "speedtest"
#################################

APP = speedtest
SRC = speedtest
LIB = prosplign xalgoalignutil xalnmgr xcleanup xobjutil submit tables \
      $(OBJMGR_LIBS:%=%$(STATIC))

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects algo -Cygwin

