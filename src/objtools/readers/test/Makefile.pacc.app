#################################
# $Id: Makefile.pacc.app 151845 2009-02-09 14:50:16Z ludwigf $
#################################

APP = pacc
SRC = pacc

LIB = xobjread seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi
