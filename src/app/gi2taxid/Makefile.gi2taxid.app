# $Id: Makefile.gi2taxid.app 33966 2004-02-06 15:42:51Z ucko $
# Makefile for 'gi2taxid' demo app
#

APP = gi2taxid
SRC = gi2taxid

LIB = id1cli id1 taxon1 seqset $(SEQ_LIBS) pub medline biblio general xser xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects
