# $Id: Makefile.convert_seq.app 151845 2009-02-09 14:50:16Z ludwigf $

APP = convert_seq
SRC = convert_seq
LIB = xformat xalnmgr gbseq xobjutil xobjread creaders tables submit \
      xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(PCRE_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin
