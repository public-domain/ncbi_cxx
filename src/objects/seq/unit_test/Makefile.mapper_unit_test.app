# $Id: Makefile.mapper_unit_test.app 159017 2009-04-30 18:28:43Z ucko $
APP = mapper_unit_test
SRC = mapper_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_CMD =
