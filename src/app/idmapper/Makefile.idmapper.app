#################################
# $Id: Makefile.idmapper.app 151845 2009-02-09 14:50:16Z ludwigf $
# Author:  Frank Ludwig
#################################

# Build application "idmapper"
#################################

APP = idmapper
SRC = idmapper
LIB = xformat xobjutil xobjread submit gbseq xalnmgr xcleanup \
	entrez2cli entrez2 tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin

