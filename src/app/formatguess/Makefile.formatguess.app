#################################
# $Id: Makefile.formatguess.app 151845 2009-02-09 14:50:16Z ludwigf $
# Author:  Frank Ludwig
#################################

# Build application "formatguess"
#################################

APP = formatguess
SRC = formatguess
LIB = xobjread creaders seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

REQUIRES = objects -Cygwin

