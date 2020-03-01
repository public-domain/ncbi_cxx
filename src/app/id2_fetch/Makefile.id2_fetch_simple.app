# $Id: Makefile.id2_fetch_simple.app 69241 2005-09-13 18:46:13Z ucko $

APP = id2_fetch_simple
SRC = id2_fetch_simple
LIB = id2 seqsplit seqset $(SEQ_LIBS) pub medline biblio general \
      xser xconnect $(COMPRESS_LIBS) xutil xncbi

LIBS = $(NETWORK_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)

REQUIRES = objects

# CHECK_CMD = id2_fetch_simple -gi 3
