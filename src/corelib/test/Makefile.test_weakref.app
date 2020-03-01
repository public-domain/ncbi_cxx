# $Id: Makefile.test_weakref.app 159017 2009-04-30 18:28:43Z ucko $

APP = test_weakref
SRC = test_weakref

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =
