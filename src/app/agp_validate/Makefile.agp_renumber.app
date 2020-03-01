#################################
# $Id: Makefile.agp_renumber.app 151845 2009-02-09 14:50:16Z ludwigf $
#################################

REQUIRES = objects

APP = agp_renumber
SRC = agp_renumber

LIB = xobjread seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi
