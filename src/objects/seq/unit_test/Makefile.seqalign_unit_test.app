# $Id: Makefile.seqalign_unit_test.app 159017 2009-04-30 18:28:43Z ucko $

APP = seqalign_unit_test
SRC = seqalign_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS)

REQUIRES = objects

CHECK_CMD = seqalign_unit_test
CHECK_COPY = data
