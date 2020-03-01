# $Id: Makefile.test_align.app 144246 2008-10-28 18:59:17Z vakatov $

APP = test_align
SRC = test_align

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = xncbi
LIBS = $(BOOST_TEST_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test
