# $Id: Makefile.readresult.app 72283 2005-11-07 21:30:36Z lewisg $
# Author:  Lewis Y. Geer

# Build application "omssacl"
#################################

APP = readresult

SRC = readresult

LIB = omssa seqset seq seqcode sequtil pub medline biblio general xser xregexp $(PCRE_LIB) xutil xncbi
LIBS = $(PCRE_LIBS) $(ORIG_LIBS)
