#################################
# $Id: Makefile.ace2asn.app 151845 2009-02-09 14:50:16Z ludwigf $
#################################

APP = ace2asn
SRC = ace2asn

LIB = xobjread xobjutil creaders $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

