# $Id: Makefile.formatguess_unit_test.app 159017 2009-04-30 18:28:43Z ucko $

APP = formatguess_unit_test
SRC = formatguess_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xncbi xutil
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included
