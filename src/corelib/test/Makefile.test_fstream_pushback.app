# $Id: Makefile.test_fstream_pushback.app 81763 2006-05-02 20:20:03Z lavr $

APP = test_fstream_pushback
SRC = test_fstream_pushback
LIB = xpbacktest xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD  = test_fstream_pushback.sh
CHECK_COPY = test_fstream_pushback.sh
