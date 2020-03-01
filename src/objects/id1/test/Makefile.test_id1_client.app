# $Id: Makefile.test_id1_client.app 22237 2003-05-21 19:11:18Z ucko $

APP = test_id1_client
SRC = test_id1_client
LIB = id1cli id1 seqset $(SEQ_LIBS) pub medline biblio general \
      xser xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
